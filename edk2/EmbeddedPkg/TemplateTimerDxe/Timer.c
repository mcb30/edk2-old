/** @file
  Template for Timer Architecture Protocol driver of the ARM flavor

  Copyright (c) 2008-2009, Apple Inc. All rights reserved.
  
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/


#include <PiDxe.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>

#include <Protocol/Timer.h>
#include <Protocol/HardwareInterrupt.h>

//
// Get Base Address of timer block from platform .DSC file
//
#define TIMER_BASE ((UINTN)FixedPcdGet32 (PcdTimerBaseAddress) + 0x00c0)


#define TIMER_CMD    ((UINTN)FixedPcdGet32 (PcdTimerBaseAddress) + 0x00000004)
#define TIMER_DATA   ((UINTN)FixedPcdGet32 (PcdTimerBaseAddress) + 0x00000008)

//
// The notification function to call on every timer interrupt.
// A bug in the compiler prevents us from initializing this here.
//
volatile EFI_TIMER_NOTIFY mTimerNotifyFunction;

//
// The current period of the timer interrupt
//
volatile UINT64           mTimerPeriod = 0;

//
// Cached copy of the Hardware Interrupt protocol instance
//
EFI_HARDWARE_INTERRUPT_PROTOCOL *gInterrupt = NULL;


/**
  C Interrupt Handler calledin the interrupt context when Source interrupt is active.

  @param Source         Source of the interrupt. Hardware routing off a specific platform defines
                        what source means.
  @param SystemContext  Pointer to system register context. Mostly used by debuggers and will
                        update the system context after the return from the interrupt if 
                        modified. Don't change these values unless you know what you are doing

**/
VOID
EFIAPI
TimerInterruptHandler (
	IN HARDWARE_INTERRUPT_SOURCE		Source,
  IN  EFI_SYSTEM_CONTEXT          SystemContext     	
	)
{
	EFI_TPL							OriginalTPL;

  // Mask all interrupts
	OriginalTPL = gBS->RaiseTPL (TPL_HIGH_LEVEL);

  MmioWrite32 (TIMER_CMD, 0);

	if (mTimerNotifyFunction) {
		mTimerNotifyFunction (mTimerPeriod);
	}

  // restore state
	gBS->RestoreTPL (OriginalTPL);
}



/**
  This function registers the handler NotifyFunction so it is called every time 
  the timer interrupt fires.  It also passes the amount of time since the last 
  handler call to the NotifyFunction.  If NotifyFunction is NULL, then the 
  handler is unregistered.  If the handler is registered, then EFI_SUCCESS is 
  returned.  If the CPU does not support registering a timer interrupt handler, 
  then EFI_UNSUPPORTED is returned.  If an attempt is made to register a handler 
  when a handler is already registered, then EFI_ALREADY_STARTED is returned.  
  If an attempt is made to unregister a handler when a handler is not registered, 
  then EFI_INVALID_PARAMETER is returned.  If an error occurs attempting to 
  register the NotifyFunction with the timer interrupt, then EFI_DEVICE_ERROR 
  is returned.

  @param  This             The EFI_TIMER_ARCH_PROTOCOL instance.
  @param  NotifyFunction   The function to call when a timer interrupt fires. This
                           function executes at TPL_HIGH_LEVEL. The DXE Core will
                           register a handler for the timer interrupt, so it can know
                           how much time has passed. This information is used to
                           signal timer based events. NULL will unregister the handler.

  @retval EFI_SUCCESS           The timer handler was registered.
  @retval EFI_UNSUPPORTED       The platform does not support timer interrupts.
  @retval EFI_ALREADY_STARTED   NotifyFunction is not NULL, and a handler is already
                                registered.
  @retval EFI_INVALID_PARAMETER NotifyFunction is NULL, and a handler was not
                                previously registered.
  @retval EFI_DEVICE_ERROR      The timer handler could not be registered.

**/
EFI_STATUS
EFIAPI
TimerDriverRegisterHandler (
  IN EFI_TIMER_ARCH_PROTOCOL  *This,
  IN EFI_TIMER_NOTIFY         NotifyFunction
  )
{
  //
  // Check for invalid parameters
  //
  if (NotifyFunction == NULL && mTimerNotifyFunction == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (NotifyFunction != NULL && mTimerNotifyFunction != NULL) {
    return EFI_ALREADY_STARTED;
  }

  mTimerNotifyFunction = NotifyFunction;

  return EFI_SUCCESS;
}



/**
  This function adjusts the period of timer interrupts to the value specified 
  by TimerPeriod.  If the timer period is updated, then the selected timer 
  period is stored in EFI_TIMER.TimerPeriod, and EFI_SUCCESS is returned.  If 
  the timer hardware is not programmable, then EFI_UNSUPPORTED is returned.  
  If an error occurs while attempting to update the timer period, then the 
  timer hardware will be put back in its state prior to this call, and 
  EFI_DEVICE_ERROR is returned.  If TimerPeriod is 0, then the timer interrupt 
  is disabled.  This is not the same as disabling the CPU's interrupts.  
  Instead, it must either turn off the timer hardware, or it must adjust the 
  interrupt controller so that a CPU interrupt is not generated when the timer 
  interrupt fires. 

  @param  This             The EFI_TIMER_ARCH_PROTOCOL instance.
  @param  TimerPeriod      The rate to program the timer interrupt in 100 nS units. If
                           the timer hardware is not programmable, then EFI_UNSUPPORTED is
                           returned. If the timer is programmable, then the timer period
                           will be rounded up to the nearest timer period that is supported
                           by the timer hardware. If TimerPeriod is set to 0, then the
                           timer interrupts will be disabled.

  @retval EFI_SUCCESS           The timer period was changed.
  @retval EFI_UNSUPPORTED       The platform cannot change the period of the timer interrupt.
  @retval EFI_DEVICE_ERROR      The timer period could not be changed due to a device error.

**/
EFI_STATUS
EFIAPI
TimerDriverSetTimerPeriod (
  IN EFI_TIMER_ARCH_PROTOCOL  *This,
  IN UINT64                   TimerPeriod
  )
{
  EFI_STATUS  Status;
  UINT64      TimerCount;
  
  if (TimerPeriod == 0) {
    //
    // Disable interrupt 0 and timer
    //
    MmioAnd32 (TIMER_DATA, 0);

		Status = gInterrupt->DisableInterruptSource (gInterrupt, FixedPcdGet32 (PcdTimerVector));    
  } else {
		//
		// Convert TimerPeriod into Timer F counts
		//
		TimerCount = DivU64x32 (TimerPeriod + 5, 10);

		//
		// Program Timer F with the new count value
		//
		MmioWrite32 (TIMER_DATA, (UINT32)TimerCount);

		//
		// Enable interrupt and initialize and enable timer.
		//
		MmioOr32 (TIMER_CMD, 0x11);

		Status = gInterrupt->EnableInterruptSource (gInterrupt,  FixedPcdGet32 (PcdTimerVector));    
  }

	//
	// Save the new timer period
	//
	mTimerPeriod = TimerPeriod;
  return Status;
}


/**
  This function retrieves the period of timer interrupts in 100 ns units, 
  returns that value in TimerPeriod, and returns EFI_SUCCESS.  If TimerPeriod 
  is NULL, then EFI_INVALID_PARAMETER is returned.  If a TimerPeriod of 0 is 
  returned, then the timer is currently disabled.

  @param  This             The EFI_TIMER_ARCH_PROTOCOL instance.
  @param  TimerPeriod      A pointer to the timer period to retrieve in 100 ns units. If
                           0 is returned, then the timer is currently disabled.

  @retval EFI_SUCCESS           The timer period was returned in TimerPeriod.
  @retval EFI_INVALID_PARAMETER TimerPeriod is NULL.

**/
EFI_STATUS
EFIAPI
TimerDriverGetTimerPeriod (
  IN EFI_TIMER_ARCH_PROTOCOL   *This,
  OUT UINT64                   *TimerPeriod
  )
{
  if (TimerPeriod == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *TimerPeriod = mTimerPeriod;
  return EFI_SUCCESS;
}



/**
  This function generates a soft timer interrupt. If the platform does not support soft 
  timer interrupts, then EFI_UNSUPPORTED is returned. Otherwise, EFI_SUCCESS is returned. 
  If a handler has been registered through the EFI_TIMER_ARCH_PROTOCOL.RegisterHandler() 
  service, then a soft timer interrupt will be generated. If the timer interrupt is 
  enabled when this service is called, then the registered handler will be invoked. The 
  registered handler should not be able to distinguish a hardware-generated timer 
  interrupt from a software-generated timer interrupt.

  @param  This             The EFI_TIMER_ARCH_PROTOCOL instance.

  @retval EFI_SUCCESS           The soft timer interrupt was generated.
  @retval EFI_UNSUPPORTED       The platform does not support the generation of soft timer interrupts.

**/
EFI_STATUS
EFIAPI
TimerDriverGenerateSoftInterrupt (
  IN EFI_TIMER_ARCH_PROTOCOL  *This
  )
{
  return EFI_UNSUPPORTED;
}


/**
  Interface stucture for the Timer Architectural Protocol.

  @par Protocol Description:
  This protocol provides the services to initialize a periodic timer 
  interrupt, and to register a handler that is called each time the timer
  interrupt fires.  It may also provide a service to adjust the rate of the
  periodic timer interrupt.  When a timer interrupt occurs, the handler is 
  passed the amount of time that has passed since the previous timer 
  interrupt.

  @param RegisterHandler
  Registers a handler that will be called each time the 
  timer interrupt fires.  TimerPeriod defines the minimum 
  time between timer interrupts, so TimerPeriod will also 
  be the minimum time between calls to the registered 
  handler.

  @param SetTimerPeriod
  Sets the period of the timer interrupt in 100 nS units.  
  This function is optional, and may return EFI_UNSUPPORTED.  
  If this function is supported, then the timer period will 
  be rounded up to the nearest supported timer period.

  @param GetTimerPeriod
  Retrieves the period of the timer interrupt in 100 nS units.

  @param GenerateSoftInterrupt
  Generates a soft timer interrupt that simulates the firing of 
  the timer interrupt. This service can be used to invoke the 
  registered handler if the timer interrupt has been masked for 
  a period of time.

**/
EFI_TIMER_ARCH_PROTOCOL   gTimer = {
  TimerDriverRegisterHandler,
  TimerDriverSetTimerPeriod,
  TimerDriverGetTimerPeriod,
  TimerDriverGenerateSoftInterrupt
};

EFI_HANDLE  gTimerHandle = NULL;


/**
  Initialize the state information for the Timer Architectural Protocol

  @param  ImageHandle   of the loaded driver
  @param  SystemTable   Pointer to the System Table

  @retval EFI_SUCCESS           Protocol registered
  @retval EFI_OUT_OF_RESOURCES  Cannot allocate protocol data structure
  @retval EFI_DEVICE_ERROR      Hardware problems

**/
EFI_STATUS
EFIAPI
TimerInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS  Status;

	//
	// Find the interrupt controller protocol.  ASSERT if not found.
	//
	Status = gBS->LocateProtocol (&gHardwareInterruptProtocolGuid, NULL, ( VOID ** )&gInterrupt);
	ASSERT_EFI_ERROR (Status);

  MmioWrite32 (TIMER_CMD, 0x01);

  //
  // Force the timer to be disabled
  //
  Status = TimerDriverSetTimerPeriod (&gTimer, 0);
	ASSERT_EFI_ERROR (Status);

  //
  // Install interrupt handler
  //
  Status = gInterrupt->RegisterInterruptSource (gInterrupt,  FixedPcdGet32 (PcdTimerVector), TimerInterruptHandler);
	ASSERT_EFI_ERROR (Status);

	//
	// Force the timer to be enabled at its default period
	//
	Status = TimerDriverSetTimerPeriod (&gTimer, FixedPcdGet32 (PcdTimerPeriod));
	ASSERT_EFI_ERROR (Status);


  //
  // Install the Timer Architectural Protocol onto a new handle
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &gTimerHandle,
                  &gEfiTimerArchProtocolGuid,  &gTimer,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

