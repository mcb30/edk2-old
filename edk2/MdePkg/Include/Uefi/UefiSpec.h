/** @file
  Include file that supportes UEFI.

  This include file must only contain things defined in the UEFI 2.0 specification.
  If a code construct is defined in the UEFI 2.0 specification it must be included
  by this include file.

  Copyright (c) 2006 - 2007, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __UEFI_SPEC_H__
#define __UEFI_SPEC_H__

#include <Uefi/UefiMultiPhase.h>

#include <Protocol/DevicePath.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>

//
// Enumeration of memory allocation.
// 
typedef enum {
  AllocateAnyPages,
  AllocateMaxAddress,
  AllocateAddress,
  MaxAllocateType
} EFI_ALLOCATE_TYPE;

//
// Bit definitions for EFI_TIME.Daylight
//
#define EFI_TIME_ADJUST_DAYLIGHT  0x01
#define EFI_TIME_IN_DAYLIGHT      0x02

//
// Value definition for EFI_TIME.TimeZone
//
#define EFI_UNSPECIFIED_TIMEZONE  0x07FF

//
// possible caching types for the memory range
//
#define EFI_MEMORY_UC   0x0000000000000001ULL
#define EFI_MEMORY_WC   0x0000000000000002ULL
#define EFI_MEMORY_WT   0x0000000000000004ULL
#define EFI_MEMORY_WB   0x0000000000000008ULL
#define EFI_MEMORY_UCE  0x0000000000000010ULL

//
// physical memory protection on range
//
#define EFI_MEMORY_WP   0x0000000000001000ULL
#define EFI_MEMORY_RP   0x0000000000002000ULL
#define EFI_MEMORY_XP   0x0000000000004000ULL

//
// range requires a runtime mapping
//
#define EFI_MEMORY_RUNTIME  0x8000000000000000ULL

#define EFI_MEMORY_DESCRIPTOR_VERSION 1
typedef struct {
  UINT32                Type;
  EFI_PHYSICAL_ADDRESS  PhysicalStart;
  EFI_VIRTUAL_ADDRESS   VirtualStart;
  UINT64                NumberOfPages;
  UINT64                Attribute;
} EFI_MEMORY_DESCRIPTOR;

//
// Build macros to find next EFI_MEMORY_DESCRIPTOR.
// 
#define NextMemoryDescriptor(_Ptr, _Size)   ((EFI_MEMORY_DESCRIPTOR *) (((UINT8 *) (_Ptr)) + (_Size)))
#define NEXT_MEMORY_DESCRIPTOR(_Ptr, _Size) NextMemoryDescriptor (_Ptr, _Size)

//
// Declare forward referenced data structures
//
typedef struct _EFI_SYSTEM_TABLE   EFI_SYSTEM_TABLE;

/**
  Allocates memory pages from the system.

  @param  Type        The type of allocation to perform.
  @param  MemoryType  The type of memory to allocate.
  @param  Pages       The number of contiguous 4 KB pages to allocate.
  @param  Memory      Pointer to a physical address. On input, the way in which the address is
                      used depends on the value of Type.

  @retval EFI_SUCCESS           The requested pages were allocated.
  @retval EFI_INVALID_PARAMETER 1) Type is not AllocateAnyPages or
                                AllocateMaxAddress or AllocateAddress.
                                2) MemoryType is in the range
                                EfiMaxMemoryType..0x7FFFFFFF.
  @retval EFI_OUT_OF_RESOURCES  The pages could not be allocated.
  @retval EFI_NOT_FOUND         The requested pages could not be found.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_ALLOCATE_PAGES) (
  IN     EFI_ALLOCATE_TYPE            Type,
  IN     EFI_MEMORY_TYPE              MemoryType,
  IN     UINTN                        Pages,
  IN OUT EFI_PHYSICAL_ADDRESS         *Memory
  );

/**
  Frees memory pages.

  @param  Memory      The base physical address of the pages to be freed.
  @param  Pages       The number of contiguous 4 KB pages to free.

  @retval EFI_SUCCESS           The requested pages were freed.
  @retval EFI_INVALID_PARAMETER Memory is not a page-aligned address or Pages is invalid.
  @retval EFI_NOT_FOUND         The requested memory pages were not allocated with
                                AllocatePages().

**/
typedef
EFI_STATUS
(EFIAPI *EFI_FREE_PAGES) (
  IN  EFI_PHYSICAL_ADDRESS         Memory,
  IN  UINTN                        Pages
  );

/**
  Returns the current memory map.

  @param  MemoryMapSize         A pointer to the size, in bytes, of the MemoryMap buffer.
  @param  MemoryMap             A pointer to the buffer in which firmware places the current memory
                                map.
  @param  MapKey                A pointer to the location in which firmware returns the key for the
                                current memory map.
  @param  DescriptorSize        A pointer to the location in which firmware returns the size, in bytes, of
                                an individual EFI_MEMORY_DESCRIPTOR.
  @param  DescriptorVersion     A pointer to the location in which firmware returns the version number
                                associated with the EFI_MEMORY_DESCRIPTOR.

  @retval EFI_SUCCESS           The memory map was returned in the MemoryMap buffer.
  @retval EFI_BUFFER_TOO_SMALL  The MemoryMap buffer was too small. The current buffer size
                                needed to hold the memory map is returned in MemoryMapSize.
  @retval EFI_INVALID_PARAMETER 1) MemoryMapSize is NULL.
                                2) The MemoryMap buffer is not too small and MemoryMap is
                                   NULL.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_GET_MEMORY_MAP) (
  IN OUT UINTN                       *MemoryMapSize,
  IN OUT EFI_MEMORY_DESCRIPTOR       *MemoryMap,
  OUT    UINTN                       *MapKey,
  OUT    UINTN                       *DescriptorSize,
  OUT    UINT32                      *DescriptorVersion
  );

/**
  Allocates pool memory.

  @param  PoolType              The type of pool to allocate.
  @param  Size                  The number of bytes to allocate from the pool.
  @param  Buffer                A pointer to a pointer to the allocated buffer if the call succeeds;
                                undefined otherwise.

  @retval EFI_SUCCESS           The requested number of bytes was allocated.
  @retval EFI_OUT_OF_RESOURCES  The pool requested could not be allocated.
  @retval EFI_INVALID_PARAMETER PoolType was invalid.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_ALLOCATE_POOL) (
  IN  EFI_MEMORY_TYPE              PoolType,
  IN  UINTN                        Size,
  OUT VOID                         **Buffer
  );

/**
  Returns pool memory to the system.

  @param  Buffer                Pointer to the buffer to free.

  @retval EFI_SUCCESS           The memory was returned to the system.
  @retval EFI_INVALID_PARAMETER Buffer was invalid.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_FREE_POOL) (
  IN  VOID                         *Buffer
  );

/**
  Changes the runtime addressing mode of EFI firmware from physical to virtual.

  @param  MemoryMapSize         The size in bytes of VirtualMap.
  @param  DescriptorSize        The size in bytes of an entry in the VirtualMap.
  @param  DescriptorVersion     The version of the structure entries in VirtualMap.
  @param  VirtualMap            An array of memory descriptors which contain new virtual
                                address mapping information for all runtime ranges.

  @retval EFI_SUCCESS           The virtual address map has been applied.
  @retval EFI_UNSUPPORTED       EFI firmware is not at runtime, or the EFI firmware is already in
                                virtual address mapped mode.
  @retval EFI_INVALID_PARAMETER DescriptorSize or DescriptorVersion is invalid.
  @retval EFI_NO_MAPPING        A virtual address was not supplied for a range in the memory
                                map that requires a mapping.
  @retval EFI_NOT_FOUND         A virtual address was supplied for an address that is not found
                                in the memory map.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_SET_VIRTUAL_ADDRESS_MAP) (
  IN  UINTN                        MemoryMapSize,
  IN  UINTN                        DescriptorSize,
  IN  UINT32                       DescriptorVersion,
  IN  EFI_MEMORY_DESCRIPTOR        *VirtualMap
  );

/**
  Connects one or more drivers to a controller.

  @param  ControllerHandle      The handle of the controller to which driver(s) are to be connected.
  @param  DriverImageHandle     A pointer to an ordered list handles that support the
                                EFI_DRIVER_BINDING_PROTOCOL.
  @param  RemainingDevicePath   A pointer to the device path that specifies a child of the
                                controller specified by ControllerHandle.
  @param  Recursive             If TRUE, then ConnectController() is called recursively
                                until the entire tree of controllers below the controller specified
                                by ControllerHandle have been created. If FALSE, then
                                the tree of controllers is only expanded one level.

  @retval EFI_SUCCESS           1) One or more drivers were connected to ControllerHandle.
                                2) No drivers were connected to ControllerHandle, but
                                RemainingDevicePath is not NULL, and it is an End Device
                                Path Node.
  @retval EFI_INVALID_PARAMETER ControllerHandle is not a valid EFI_HANDLE.
  @retval EFI_NOT_FOUND         1) There are no EFI_DRIVER_BINDING_PROTOCOL instances
                                present in the system.
                                2) No drivers were connected to ControllerHandle.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_CONNECT_CONTROLLER) (
  IN  EFI_HANDLE                    ControllerHandle,
  IN  EFI_HANDLE                    *DriverImageHandle,   OPTIONAL
  IN  EFI_DEVICE_PATH_PROTOCOL      *RemainingDevicePath, OPTIONAL
  IN  BOOLEAN                       Recursive
  );

/**
  Disconnects one or more drivers from a controller.

  @param  ControllerHandle      The handle of the controller from which driver(s) are to be disconnected.
  @param  DriverImageHandle     The driver to disconnect from ControllerHandle.
  @param  ChildHandle           The handle of the child to destroy.

  @retval EFI_SUCCESS           1) One or more drivers were disconnected from the controller.
                                2) On entry, no drivers are managing ControllerHandle.
                                3) DriverImageHandle is not NULL, and on entry
                                   DriverImageHandle is not managing ControllerHandle.

  @retval EFI_INVALID_PARAMETER One ore more parameters are invalid.
  @retval EFI_OUT_OF_RESOURCES  There are not enough resources available to disconnect any drivers from
                                ControllerHandle.
  @retval EFI_DEVICE_ERROR      The controller could not be disconnected because of a device error.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_DISCONNECT_CONTROLLER) (
  IN  EFI_HANDLE                     ControllerHandle,
  IN  EFI_HANDLE                     DriverImageHandle, OPTIONAL
  IN  EFI_HANDLE                     ChildHandle        OPTIONAL
  );



//
// ConvertPointer DebugDisposition type.
//
#define EFI_OPTIONAL_PTR     0x00000001
#define EFI_OPTIONAL_POINTER EFI_OPTIONAL_PTR

/**
  Determines the new virtual address that is to be used on subsequent memory accesses.

  @param  DebugDisposition      Supplies type information for the pointer being converted.
  @param  Address               A pointer to a pointer that is to be fixed to be the value needed
                                for the new virtual address mappings being applied.

  @retval EFI_SUCCESS           The pointer pointed to by Address was modified.
  @retval EFI_INVALID_PARAMETER 1) Address is NULL.
                                2) *Address is NULL and DebugDisposition does
                                not have the EFI_OPTIONAL_PTR bit set.
  @retval EFI_NOT_FOUND         The pointer pointed to by Address was not found to be part
                                of the current memory map. This is normally fatal.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_CONVERT_POINTER) (
  IN     UINTN                      DebugDisposition,
  IN OUT VOID                       **Address
  );


//
// These types can be ��ORed�� together as needed �C for example,
// EVT_TIMER might be ��Ored�� with EVT_NOTIFY_WAIT or
// EVT_NOTIFY_SIGNAL.
//
#define EVT_TIMER                         0x80000000
#define EVT_RUNTIME                       0x40000000

#define EVT_NOTIFY_WAIT                   0x00000100
#define EVT_NOTIFY_SIGNAL                 0x00000200
#define EVT_SIGNAL_EXIT_BOOT_SERVICES     0x00000201
#define EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE 0x60000202


//
// The event��s NotifyContext pointer points to a runtime memory
// address. 
// The event is deprecated in UEFI2.0 and later specifications.
// 
#define EVT_RUNTIME_CONTEXT               0x20000000


/**
  Invoke a notification event

  @param  Event                 Event whose notification function is being invoked.
  @param  Context               Pointer to the notification function's context,
                                which is implementation-dependent.

**/
typedef
VOID
(EFIAPI *EFI_EVENT_NOTIFY) (
  IN  EFI_EVENT                Event,
  IN  VOID                     *Context
  );

/**
  Creates an event.

  @param  Type                  The type of event to create and its mode and attributes.
  @param  NotifyTpl             Pointer to the notification function's context.
  @param  NotifyFunction        Pointer to the event's notification function, if any.
  @param  NotifyContext         Pointer to the notification function's context; corresponds to parameter
                                Context in the notification function.
  @param  Event                 Pointer to the newly created event if the call succeeds; undefined
                                otherwise.

  @retval EFI_SUCCESS           The event structure was created.
  @retval EFI_INVALID_PARAMETER One or more parameters are invalid.
  @retval EFI_OUT_OF_RESOURCES  The event could not be allocated.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_CREATE_EVENT) (
  IN  UINT32                       Type,
  IN  EFI_TPL                      NotifyTpl,
  IN  EFI_EVENT_NOTIFY             NotifyFunction,
  IN  VOID                         *NotifyContext,
  OUT EFI_EVENT                    *Event
  );

/**
  Creates an event in a group.

  @param  Type                  The type of event to create and its mode and attributes.
  @param  NotifyTpl             Pointer to the notification function's context.
  @param  NotifyFunction        Pointer to the event's notification function, if any.
  @param  NotifyContext         Pointer to the notification function's context; corresponds to parameter
                                Context in the notification function.
  @param  EventGroup            Pointer to the unique identifier of the group to which this event belongs.
  @param  Event                 Pointer to the newly created event if the call succeeds; undefined
                                otherwise.

  @retval EFI_SUCCESS           The event structure was created.
  @retval EFI_INVALID_PARAMETER One or more parameters are invalid.
  @retval EFI_OUT_OF_RESOURCES  The event could not be allocated.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_CREATE_EVENT_EX) (
  IN       UINT32                 Type,
  IN       EFI_TPL                NotifyTpl      OPTIONAL,
  IN       EFI_EVENT_NOTIFY       NotifyFunction OPTIONAL,
  IN CONST VOID                   *NotifyContext OPTIONAL,
  IN CONST EFI_GUID               *EventGroup    OPTIONAL,
  OUT      EFI_EVENT              *Event
  );

typedef enum {
  TimerCancel,
  TimerPeriodic,
  TimerRelative
} EFI_TIMER_DELAY;

/**
  Sets the type of timer and the trigger time for a timer event.

  @param  Event                 The timer event that is to be signaled at the specified time.
  @param  Type                  The type of time that is specified in TriggerTime.
  @param  TriggerTime           The number of 100ns units until the timer expires.

  @retval EFI_SUCCESS           The event has been set to be signaled at the requested time.
  @retval EFI_INVALID_PARAMETER Event or Type is not valid.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_SET_TIMER) (
  IN  EFI_EVENT                Event,
  IN  EFI_TIMER_DELAY          Type,
  IN  UINT64                   TriggerTime
  );

/**
  Signals an event.

  @param  Event                 The event to signal.

  @retval EFI_SUCCESS           The event has been signaled.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_SIGNAL_EVENT) (
  IN  EFI_EVENT                Event
  );

/**
  Stops execution until an event is signaled.

  @param  NumberOfEvents        The number of events in the Event array.
  @param  Event                 An array of EFI_EVENT.
  @param  Index                 Pointer to the index of the event which satisfied the wait condition.

  @retval EFI_SUCCESS           The event indicated by Index was signaled.
  @retval EFI_INVALID_PARAMETER 1) NumberOfEvents is 0.
                                2) The event indicated by Index is of type
                                   EVT_NOTIFY_SIGNAL.
  @retval EFI_UNSUPPORTED       The current TPL is not TPL_APPLICATION.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_WAIT_FOR_EVENT) (
  IN  UINTN                    NumberOfEvents,
  IN  EFI_EVENT                *Event,
  OUT UINTN                    *Index
  );

/**
  Closes an event.

  @param  Event                 The event to close.

  @retval EFI_SUCCESS           The event has been closed.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_CLOSE_EVENT) (
  IN EFI_EVENT                Event
  );

/**
  Checks whether an event is in the signaled state.

  @param  Event                 The event to check.

  @retval EFI_SUCCESS           The event is in the signaled state.
  @retval EFI_NOT_READY         The event is not in the signaled state.
  @retval EFI_INVALID_PARAMETER Event is of type EVT_NOTIFY_SIGNAL.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_CHECK_EVENT) (
  IN EFI_EVENT                Event
  );


//
// Task priority level (name defined in spec).
//
#define TPL_APPLICATION       4
#define TPL_CALLBACK          8
#define TPL_NOTIFY            16
#define TPL_HIGH_LEVEL        31


/**
  Raises a task's priority level and returns its previous level.

  @param  NewTpl                The new task priority level.

  @retval                       Previous task priority level

**/
typedef
EFI_TPL
(EFIAPI *EFI_RAISE_TPL) (
  IN EFI_TPL      NewTpl
  );

/**
  Restores a task's priority level to its previous value.

  @param  OldTpl                The previous task priority level to restore

**/
typedef
VOID
(EFIAPI *EFI_RESTORE_TPL) (
  IN EFI_TPL      OldTpl
  );

/**
  Returns the value of a variable.

  @param  VariableName          A Null-terminated Unicode string that is the name of the
                                vendor's variable.
  @param  VendorGuid            A unique identifier for the vendor.
  @param  Attributes            If not NULL, a pointer to the memory location to return the
                                attributes bitmask for the variable.
  @param  DataSize              On input, the size in bytes of the return Data buffer.
                                On output the size of data returned in Data.
  @param  Data                  The buffer to return the contents of the variable.

  @retval EFI_SUCCESS           The function completed successfully.
  @retval EFI_NOT_FOUND         The variable was not found.
  @retval EFI_BUFFER_TOO_SMALL  The DataSize is too small for the result.
  @retval EFI_INVALID_PARAMETER One or more parameters are invalid.
  @retval EFI_DEVICE_ERROR      The variable could not be retrieved due to a hardware error.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_GET_VARIABLE) (
  IN     CHAR16                      *VariableName,
  IN     EFI_GUID                    *VendorGuid,
  OUT    UINT32                      *Attributes,    OPTIONAL
  IN OUT UINTN                       *DataSize,
  OUT    VOID                        *Data
  );

/**
  Enumerates the current variable names.

  @param  VariableNameSize      The size of the VariableName buffer.
  @param  VariableName          On input, supplies the last VariableName that was returned
                                by GetNextVariableName(). On output, returns the Nullterminated
                                Unicode string of the current variable.
  @param  VendorGuid            On input, supplies the last VendorGuid that was returned by
                                GetNextVariableName(). On output, returns the
                                VendorGuid of the current variable.

  @retval EFI_SUCCESS           The function completed successfully.
  @retval EFI_NOT_FOUND         The next variable was not found.
  @retval EFI_BUFFER_TOO_SMALL  The VariableNameSize is too small for the result.
  @retval EFI_INVALID_PARAMETER One or more parameters are invalid.
  @retval EFI_DEVICE_ERROR      The variable could not be retrieved due to a hardware error.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_GET_NEXT_VARIABLE_NAME) (
  IN OUT UINTN                    *VariableNameSize,
  IN OUT CHAR16                   *VariableName,
  IN OUT EFI_GUID                 *VendorGuid
  );

/**
  Sets the value of a variable.

  @param  VariableName          A Null-terminated Unicode string that is the name of the
                                vendor's variable.
  @param  VendorGuid            A unique identifier for the vendor.
  @param  Attributes            Attributes bitmask to set for the variable.
  @param  DataSize              The size in bytes of the Data buffer.
  @param  Data                  The contents for the variable.

  @retval EFI_SUCCESS           The firmware has successfully stored the variable and its data as
                                defined by the Attributes.
  @retval EFI_WRITE_PROTECTED   The variable in question is read-only.
  @retval EFI_OUT_OF_RESOURCES  Not enough storage is available to hold the variable and its data.
  @retval EFI_INVALID_PARAMETER One or more parameters are invalid.
  @retval EFI_DEVICE_ERROR      The variable could not be retrieved due to a hardware error.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_SET_VARIABLE) (
  IN  CHAR16                       *VariableName,
  IN  EFI_GUID                     *VendorGuid,
  IN  UINT32                       Attributes,
  IN  UINTN                        DataSize,
  IN  VOID                         *Data
  );


// 
// This provides the capabilities of the
// real time clock device as exposed through the EFI interfaces.
// 
typedef struct {
  UINT32    Resolution;
  UINT32    Accuracy;
  BOOLEAN   SetsToZero;
} EFI_TIME_CAPABILITIES;

/**
  Returns the current time and date information, and the time-keeping capabilities
  of the hardware platform.

  @param  Time                  A pointer to storage to receive a snapshot of the current time.
  @param  Capabilities          An optional pointer to a buffer to receive the real time clock
                                device's capabilities.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_INVALID_PARAMETER Time is NULL.
  @retval EFI_DEVICE_ERROR      The time could not be retrieved due to hardware error.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_GET_TIME) (
  OUT  EFI_TIME                    *Time,
  OUT  EFI_TIME_CAPABILITIES       *Capabilities OPTIONAL
  );

/**
  Sets the current local time and date information.

  @param  Time                  A pointer to the current time.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_INVALID_PARAMETER A time field is out of range.
  @retval EFI_DEVICE_ERROR      The time could not be set due due to hardware error.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_SET_TIME) (
  IN  EFI_TIME                     *Time
  );

/**
  Returns the current wakeup alarm clock setting.

  @param  Enabled               Indicates if the alarm is currently enabled or disabled.
  @param  Pending               Indicates if the alarm signal is pending and requires acknowledgement.
  @param  Time                  The current alarm setting.

  @retval EFI_SUCCESS           The alarm settings were returned.
  @retval EFI_INVALID_PARAMETER Any parameter is NULL.
  @retval EFI_DEVICE_ERROR      The wakeup time could not be retrieved due to a hardware error.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_GET_WAKEUP_TIME) (
  OUT BOOLEAN                     *Enabled,
  OUT BOOLEAN                     *Pending,
  OUT EFI_TIME                    *Time
  );

/**
  Sets the system wakeup alarm clock time.

  @param  Enabled               Enable or disable the wakeup alarm.
  @param  Time                  If Enable is TRUE, the time to set the wakeup alarm for.

  @retval EFI_SUCCESS           If Enable is TRUE, then the wakeup alarm was enabled. If
                                Enable is FALSE, then the wakeup alarm was disabled.
  @retval EFI_INVALID_PARAMETER A time field is out of range.
  @retval EFI_DEVICE_ERROR      The wakeup time could not be set due to a hardware error.
  @retval EFI_UNSUPPORTED       A wakeup timer is not supported on this platform.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_SET_WAKEUP_TIME) (
  IN  BOOLEAN                      Enable,
  IN  EFI_TIME                     *Time   OPTIONAL
  );

/**
  This is the declaration of an EFI image entry point. This can be the entry point to an application
  written to this specification, an EFI boot service driver, or an EFI runtime driver.

  @param  ImageHandle           Handle that identifies the loaded image.
  @param  SystemTable           System Table for this image.

  @retval EFI_SUCCESS           The operation completed successfully.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_IMAGE_ENTRY_POINT) (
  IN  EFI_HANDLE                   ImageHandle,
  IN  EFI_SYSTEM_TABLE             *SystemTable
  );

/**
  Loads an EFI image into memory.

  @param  BootPolicy            If TRUE, indicates that the request originates from the boot
                                manager, and that the boot manager is attempting to load
                                FilePath as a boot selection. Ignored if SourceBuffer is
                                not NULL.
  @param  ParentImageHandle     The caller's image handle.
  @param  FilePath              The DeviceHandle specific file path from which the image is
                                loaded.
  @param  SourceBuffer          If not NULL, a pointer to the memory location containing a copy
                                of the image to be loaded.
  @param  SourceSize            The size in bytes of SourceBuffer.
  @param  ImageHandle           Pointer to the returned image handle that is created when the
                                image is successfully loaded.

  @retval EFI_SUCCESS           Image was loaded into memory correctly.
  @retval EFI_NOT_FOUND         Both SourceBuffer and FilePath are NULL.
  @retval EFI_INVALID_PARAMETER One or more parametes are invalid.
  @retval EFI_UNSUPPORTED       The image type is not supported.
  @retval EFI_OUT_OF_RESOURCES  Image was not loaded due to insufficient resources.
  @retval EFI_LOAD_ERROR        Image was not loaded because the image format was corrupt or not
                                understood.
  @retval EFI_DEVICE_ERROR      Image was not loaded because the device returned a read error.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_IMAGE_LOAD) (
  IN  BOOLEAN                      BootPolicy,
  IN  EFI_HANDLE                   ParentImageHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL     *FilePath,
  IN  VOID                         *SourceBuffer OPTIONAL,
  IN  UINTN                        SourceSize,
  OUT EFI_HANDLE                   *ImageHandle
  );

/**
  Transfers control to a loaded image's entry point.

  @param  ImageHandle           Handle of image to be started.
  @param  ExitDataSize          Pointer to the size, in bytes, of ExitData.
  @param  ExitData              Pointer to a pointer to a data buffer that includes a Null-terminated
                                Unicode string, optionally followed by additional binary data.

  @retval EFI_INVALID_PARAMETER ImageHandle is either an invalid image handle or the image
                                has already been initialized with StartImage
  @retval Exit code from image  Exit code from image

**/
typedef
EFI_STATUS
(EFIAPI *EFI_IMAGE_START) (
  IN  EFI_HANDLE                  ImageHandle,
  OUT UINTN                       *ExitDataSize,
  OUT CHAR16                      **ExitData    OPTIONAL
  );

/**
  Terminates a loaded EFI image and returns control to boot services.

  @param  ImageHandle           Handle that identifies the image.
  @param  ExitStatus            The image's exit code.
  @param  ExitDataSize          The size, in bytes, of ExitData.
  @param  ExitData              Pointer to a data buffer that includes a Null-terminated Unicode string,
                                optionally followed by additional binary data.

  @retval EFI_SUCCESS           The image specified by ImageHandle was unloaded.
  @retval EFI_INVALID_PARAMETER The image specified by ImageHandle has been loaded and
                                started with LoadImage() and StartImage(), but the
                                image is not the currently executing image.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_EXIT) (
  IN  EFI_HANDLE                   ImageHandle,
  IN  EFI_STATUS                   ExitStatus,
  IN  UINTN                        ExitDataSize,
  IN  CHAR16                       *ExitData     OPTIONAL
  );

/**
  Unloads an image.

  @param  ImageHandle           Handle that identifies the image to be unloaded.

  @retval EFI_SUCCESS           The image has been unloaded.
  @retval EFI_INVALID_PARAMETER ImageHandle is not a valid image handle.
  @retval EFI_UNSUPPORTED       The image has been started, and does not support unload.
  @retval                       Exit code from the image's unload handler

**/
typedef
EFI_STATUS
(EFIAPI *EFI_IMAGE_UNLOAD) (
  IN  EFI_HANDLE                   ImageHandle
  );

/**
  Terminates all boot services.

  @param  ImageHandle           Handle that identifies the exiting image.
  @param  MapKey                Key to the latest memory map.

  @retval EFI_SUCCESS           Boot services have been terminated.
  @retval EFI_INVALID_PARAMETER MapKey is incorrect.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_EXIT_BOOT_SERVICES) (
  IN  EFI_HANDLE                   ImageHandle,
  IN  UINTN                        MapKey
  );

/**
  Induces a fine-grained stall.

  @param  Microseconds          The number of microseconds to stall execution.

  @retval EFI_SUCCESS           Execution was stalled at least the requested number of
                                Microseconds.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_STALL) (
  IN  UINTN                    Microseconds
  );

/**
  Sets the system's watchdog timer.

  @param  Timeout               The number of seconds to set the watchdog timer to.
  @param  WatchdogCode          The numeric code to log on a watchdog timer timeout event.
  @param  DataSize              The size, in bytes, of WatchdogData.
  @param  WatchdogData          A data buffer that includes a Null-terminated Unicode string, optionally
                                followed by additional binary data.

  @retval EFI_SUCCESS           The timeout has been set.
  @retval EFI_INVALID_PARAMETER The supplied WatchdogCode is invalid.
  @retval EFI_UNSUPPORTED       The system does not have a watchdog timer.
  @retval EFI_DEVICE_ERROR      The watch dog timer could not be programmed due to a hardware
                                error.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_SET_WATCHDOG_TIMER) (
  IN UINTN                    Timeout,
  IN UINT64                   WatchdogCode,
  IN UINTN                    DataSize,
  IN CHAR16                   *WatchdogData OPTIONAL
  );

//
// Enumeration of reset types.
// 
typedef enum {
  EfiResetCold,
  EfiResetWarm,
  EfiResetShutdown,
} EFI_RESET_TYPE;

/**
  Resets the entire platform.

  @param  ResetType             The type of reset to perform.
  @param  ResetStatus           The status code for the reset.
  @param  DataSize              The size, in bytes, of WatchdogData.
  @param  ResetData             For a ResetType of EfiResetCold, EfiResetWarm, or
                                EfiResetShutdown the data buffer starts with a Null-terminated
                                Unicode string, optionally followed by additional binary data.

**/
typedef
VOID
(EFIAPI *EFI_RESET_SYSTEM) (
  IN EFI_RESET_TYPE           ResetType,
  IN EFI_STATUS               ResetStatus,
  IN UINTN                    DataSize,
  IN CHAR16                   *ResetData OPTIONAL
  );

/**
  Returns a monotonically increasing count for the platform.

  @param  Count                 Pointer to returned value.

  @retval EFI_SUCCESS           The next monotonic count was returned.
  @retval EFI_INVALID_PARAMETER Count is NULL.
  @retval EFI_DEVICE_ERROR      The device is not functioning properly.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_GET_NEXT_MONOTONIC_COUNT) (
  OUT UINT64                  *Count
  );

/**
  Returns the next high 32 bits of the platform's monotonic counter.

  @param  HighCount             Pointer to returned value.

  @retval EFI_SUCCESS           The next high monotonic count was returned.
  @retval EFI_INVALID_PARAMETER HighCount is NULL.
  @retval EFI_DEVICE_ERROR      The device is not functioning properly.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_GET_NEXT_HIGH_MONO_COUNT) (
  OUT UINT32                  *HighCount
  );

/**
  Computes and returns a 32-bit CRC for a data buffer.

  @param  Data                  A pointer to the buffer on which the 32-bit CRC is to be computed.
  @param  DataSize              The number of bytes in the buffer Data.
  @param  Crc32                 The 32-bit CRC that was computed for the data buffer specified by Data
                                and DataSize.

  @retval EFI_SUCCESS           The 32-bit CRC was computed for the data buffer and returned in
                                Crc32.
  @retval EFI_INVALID_PARAMETER One or more parameters are invalid.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_CALCULATE_CRC32) (
  IN  VOID                              *Data,
  IN  UINTN                             DataSize,
  OUT UINT32                            *Crc32
  );

/**
  Copies the contents of one buffer to another buffer.

  @param  Destination           Pointer to the destination buffer of the memory copy.
  @param  Source                Pointer to the source buffer of the memory copy.
  @param  Length                Number of bytes to copy from Source to Destination.

**/
typedef
VOID
(EFIAPI *EFI_COPY_MEM) (
  IN VOID     *Destination,
  IN VOID     *Source,
  IN UINTN    Length
  );

/**
  The SetMem() function fills a buffer with a specified value.

  @param  Buffer                Pointer to the buffer to fill.
  @param  Size                  Number of bytes in Buffer to fill.
  @param  Value                 Value to fill Buffer with.

**/
typedef
VOID
(EFIAPI *EFI_SET_MEM) (
  IN VOID     *Buffer,
  IN UINTN    Size,
  IN UINT8    Value
  );


//
// Protocol handler functions
//
typedef enum {
  EFI_NATIVE_INTERFACE
} EFI_INTERFACE_TYPE;

/**
  Installs a protocol interface on a device handle. If the handle does not exist, it is created and added
  to the list of handles in the system. InstallMultipleProtocolInterfaces() performs
  more error checking than InstallProtocolInterface(), so it is recommended that
  InstallMultipleProtocolInterfaces() be used in place of
  InstallProtocolInterface()

  @param  Handle                A pointer to the EFI_HANDLE on which the interface is to be installed.
  @param  Protocol              The numeric ID of the protocol interface.
  @param  InterfaceType         Indicates whether Interface is supplied in native form.
  @param  Interface             A pointer to the protocol interface.

  @retval EFI_SUCCESS           The protocol interface was installed.
  @retval EFI_OUT_OF_RESOURCES  Space for a new handle could not be allocated.
  @retval EFI_INVALID_PARAMETER One or more parameters are invalid.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_INSTALL_PROTOCOL_INTERFACE) (
  IN OUT EFI_HANDLE               *Handle,
  IN     EFI_GUID                 *Protocol,
  IN     EFI_INTERFACE_TYPE       InterfaceType,
  IN     VOID                     *Interface
  );

/**
  Installs one or more protocol interfaces into the boot services environment.

  @param  Handle                The handle to install the new protocol interfaces on, or NULL if a new
                                handle is to be allocated.
  @param  ...                   A variable argument list containing pairs of protocol GUIDs and protocol
                                interfaces.

  @retval EFI_SUCCESS           All the protocol interface was installed.
  @retval EFI_OUT_OF_RESOURCES  There was not enough memory in pool to install all the protocols.
  @retval EFI_ALREADY_STARTED   A Device Path Protocol instance was passed in that is already present in
                                the handle database.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_INSTALL_MULTIPLE_PROTOCOL_INTERFACES) (
  IN OUT EFI_HANDLE           *Handle,
  ...
  );

/**
  Reinstalls a protocol interface on a device handle.

  @param  Handle                Handle on which the interface is to be reinstalled.
  @param  Protocol              The numeric ID of the interface.
  @param  OldInterface          A pointer to the old interface. NULL can be used if a structure is not
                                associated with Protocol.
  @param  NewInterface          A pointer to the new interface.

  @retval EFI_SUCCESS           The protocol interface was reinstalled.
  @retval EFI_NOT_FOUND         The OldInterface on the handle was not found.
  @retval EFI_ACCESS_DENIED     The protocol interface could not be reinstalled,
                                because OldInterface is still being used by a
                                driver that will not release it.
  @retval EFI_INVALID_PARAMETER One or more parameters are invalid.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_REINSTALL_PROTOCOL_INTERFACE) (
  IN EFI_HANDLE               Handle,
  IN EFI_GUID                 *Protocol,
  IN VOID                     *OldInterface,
  IN VOID                     *NewInterface
  );

/**
  Removes a protocol interface from a device handle. It is recommended that
  UninstallMultipleProtocolInterfaces() be used in place of
  UninstallProtocolInterface().

  @param  Handle                The handle on which the interface was installed.
  @param  Protocol              The numeric ID of the interface.
  @param  Interface             A pointer to the interface.

  @retval EFI_SUCCESS           The interface was removed.
  @retval EFI_NOT_FOUND         The interface was not found.
  @retval EFI_ACCESS_DENIED     The interface was not removed because the interface
                                is still being used by a driver.
  @retval EFI_INVALID_PARAMETER One or more parameters are invalid.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_UNINSTALL_PROTOCOL_INTERFACE) (
  IN EFI_HANDLE               Handle,
  IN EFI_GUID                 *Protocol,
  IN VOID                     *Interface
  );

/**
  Removes one or more protocol interfaces into the boot services environment.

  @param  Handle                The handle to remove the protocol interfaces from.
  @param  ...                   A variable argument list containing pairs of protocol GUIDs and
                                protocol interfaces.

  @retval EFI_SUCCESS           All the protocol interfaces were removed.
  @retval EFI_INVALID_PARAMETER One of the protocol interfaces was not previously installed on Handle.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_UNINSTALL_MULTIPLE_PROTOCOL_INTERFACES) (
  IN EFI_HANDLE           Handle,
  ...
  );

/**
  Queries a handle to determine if it supports a specified protocol.

  @param  Handle                The handle being queried.
  @param  Protocol              The published unique identifier of the protocol.
  @param  Interface             Supplies the address where a pointer to the corresponding Protocol
                                Interface is returned.
  @retval EFI_SUCCESS           The interface information for the specified protocol was returned.
  @retval EFI_UNSUPPORTED       The device does not support the specified protocol.
  @retval EFI_INVALID_PARAMETER One of the protocol interfaces was not previously installed on Handle.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_HANDLE_PROTOCOL) (
  IN  EFI_HANDLE               Handle,
  IN  EFI_GUID                 *Protocol,
  OUT VOID                     **Interface
  );

#define EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL  0x00000001
#define EFI_OPEN_PROTOCOL_GET_PROTOCOL        0x00000002
#define EFI_OPEN_PROTOCOL_TEST_PROTOCOL       0x00000004
#define EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER 0x00000008
#define EFI_OPEN_PROTOCOL_BY_DRIVER           0x00000010
#define EFI_OPEN_PROTOCOL_EXCLUSIVE           0x00000020

/**
  Queries a handle to determine if it supports a specified protocol. If the protocol is supported by the
  handle, it opens the protocol on behalf of the calling agent.

  @param  Handle                The handle for the protocol interface that is being opened.
  @param  Protocol              The published unique identifier of the protocol.
  @param  Interface             Supplies the address where a pointer to the corresponding Protocol
                                Interface is returned.
  @param  AgentHandle           The handle of the agent that is opening the protocol interface
                                specified by Protocol and Interface.
  @param  ControllerHandle      If the agent that is opening a protocol is a driver that follows the
                                UEFI Driver Model, then this parameter is the controller handle
                                that requires the protocol interface. If the agent does not follow
                                the UEFI Driver Model, then this parameter is optional and may
                                be NULL.
  @param  Attributes            The open mode of the protocol interface specified by Handle
                                and Protocol.

  @retval EFI_SUCCESS           An item was added to the open list for the protocol interface, and the
                                protocol interface was returned in Interface.
  @retval EFI_UNSUPPORTED       Handle does not support Protocol.
  @retval EFI_INVALID_PARAMETER One or more parameters are invalid.
  @retval EFI_ACCESS_DENIED     Required attributes can't be supported in current environment.
  @retval EFI_ALREADY_STARTED   Item on the open list already has requierd attributes whose agent
                                handle is the same as AgentHandle.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_OPEN_PROTOCOL) (
  IN  EFI_HANDLE                Handle,
  IN  EFI_GUID                  *Protocol,
  OUT VOID                      **Interface,
  IN  EFI_HANDLE                AgentHandle,
  IN  EFI_HANDLE                ControllerHandle, OPTIONAL
  IN  UINT32                    Attributes
  );


/**
  Closes a protocol on a handle that was opened using OpenProtocol().

  @param  Handle                The handle for the protocol interface that was previously opened
                                with OpenProtocol(), and is now being closed.
  @param  Protocol              The published unique identifier of the protocol.
  @param  Interface             Supplies the address where a pointer to the corresponding Protocol
                                Interface is returned.
  @param  AgentHandle           The handle of the agent that is closing the protocol interface.
  @param  ControllerHandle      If the agent that opened a protocol is a driver that follows the
                                UEFI Driver Model, then this parameter is the controller handle
                                that required the protocol interface.

  @retval EFI_SUCCESS           The protocol instance was closed.
  @retval EFI_INVALID_PARAMETER One or more parameters are invalid.
  @retval EFI_NOT_FOUND         1) Handle does not support the protocol specified by Protocol.
                                2) The protocol interface specified by Handle and Protocol is not
                                   currently open by AgentHandle and ControllerHandle.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_CLOSE_PROTOCOL) (
  IN EFI_HANDLE               Handle,
  IN EFI_GUID                 *Protocol,
  IN EFI_HANDLE               AgentHandle,
  IN EFI_HANDLE               ControllerHandle
  );


typedef struct {
  EFI_HANDLE  AgentHandle;
  EFI_HANDLE  ControllerHandle;
  UINT32      Attributes;
  UINT32      OpenCount;
} EFI_OPEN_PROTOCOL_INFORMATION_ENTRY;

/**
  Retrieves the list of agents that currently have a protocol interface opened.

  @param  Handle                The handle for the protocol interface that is being queried.
  @param  Protocol              The published unique identifier of the protocol.
  @param  EntryBuffer           A pointer to a buffer of open protocol information in the form of
                                EFI_OPEN_PROTOCOL_INFORMATION_ENTRY structures.
  @param  EntryCount            A pointer to the number of entries in EntryBuffer.

  @retval EFI_SUCCESS           The open protocol information was returned in EntryBuffer, and the
                                number of entries was returned EntryCount.
  @retval EFI_OUT_OF_RESOURCES  There are not enough resources available to allocate EntryBuffer.
  @retval EFI_NOT_FOUND         Handle does not support the protocol specified by Protocol.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_OPEN_PROTOCOL_INFORMATION) (
  IN  EFI_HANDLE                          Handle,
  IN  EFI_GUID                            *Protocol,
  IN  EFI_OPEN_PROTOCOL_INFORMATION_ENTRY **EntryBuffer,
  OUT UINTN                               *EntryCount
  );

/**
  Retrieves the list of protocol interface GUIDs that are installed on a handle in a buffer allocated
  from pool.

  @param  Handle                The handle from which to retrieve the list of protocol interface
                                GUIDs.
  @param  ProtocolBuffer        A pointer to the list of protocol interface GUID pointers that are
                                installed on Handle.
  @param  ProtocolBufferCount   A pointer to the number of GUID pointers present in
                                ProtocolBuffer.

  @retval EFI_SUCCESS           The list of protocol interface GUIDs installed on Handle was returned in
                                ProtocolBuffer. The number of protocol interface GUIDs was
                                returned in ProtocolBufferCount.
  @retval EFI_OUT_OF_RESOURCES  There is not enough pool memory to store the results.
  @retval EFI_INVALID_PARAMETER One or more parameters are invalid.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_PROTOCOLS_PER_HANDLE) (
  IN  EFI_HANDLE      Handle,
  OUT EFI_GUID        ***ProtocolBuffer,
  OUT UINTN           *ProtocolBufferCount
  );

/**
  Creates an event that is to be signaled whenever an interface is installed for a specified protocol.

  @param  Protocol              The numeric ID of the protocol for which the event is to be registered.
  @param  Event                 Event that is to be signaled whenever a protocol interface is registered
                                for Protocol.
  @param  Registration          A pointer to a memory location to receive the registration value.

  @retval EFI_SUCCESS           The notification event has been registered.
  @retval EFI_OUT_OF_RESOURCES  Space for the notification event could not be allocated.
  @retval EFI_INVALID_PARAMETER One or more parameters are invalid.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_REGISTER_PROTOCOL_NOTIFY) (
  IN  EFI_GUID                 *Protocol,
  IN  EFI_EVENT                Event,
  OUT VOID                     **Registration
  );


typedef enum {
  AllHandles,
  ByRegisterNotify,
  ByProtocol
} EFI_LOCATE_SEARCH_TYPE;

/**
  Returns an array of handles that support a specified protocol.

  @param  SearchType            Specifies which handle(s) are to be returned.
  @param  Protocol              Specifies the protocol to search by.
  @param  SearchKey             Specifies the search key.
  @param  BufferSize            On input, the size in bytes of Buffer. On output, the size in bytes of
                                the array returned in Buffer (if the buffer was large enough) or the
                                size, in bytes, of the buffer needed to obtain the array (if the buffer was
                                not large enough).
  @param  Buffer                The buffer in which the array is returned.

  @retval EFI_SUCCESS           The array of handles was returned.
  @retval EFI_NOT_FOUND         No handles match the search.
  @retval EFI_BUFFER_TOO_SMALL  The BufferSize is too small for the result.
  @retval EFI_INVALID_PARAMETER One or more parameters are invalid.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_LOCATE_HANDLE) (
  IN     EFI_LOCATE_SEARCH_TYPE   SearchType,
  IN     EFI_GUID                 *Protocol,    OPTIONAL
  IN     VOID                     *SearchKey,   OPTIONAL
  IN OUT UINTN                    *BufferSize,
  OUT    EFI_HANDLE               *Buffer
  );

/**
  Locates the handle to a device on the device path that supports the specified protocol.

  @param  Protocol              Specifies the protocol to search for.
  @param  DevicePath            On input, a pointer to a pointer to the device path. On output, the device
                                path pointer is modified to point to the remaining part of the device
                                path.
  @param  Device                A pointer to the returned device handle.

  @retval EFI_SUCCESS           The resulting handle was returned.
  @retval EFI_NOT_FOUND         No handles match the search.
  @retval EFI_INVALID_PARAMETER One or more parameters are invalid.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_LOCATE_DEVICE_PATH) (
  IN     EFI_GUID                         *Protocol,
  IN OUT EFI_DEVICE_PATH_PROTOCOL         **DevicePath,
  OUT    EFI_HANDLE                       *Device
  );

/**
  Adds, updates, or removes a configuration table entry from the EFI System Table.

  @param  Guid                  A pointer to the GUID for the entry to add, update, or remove.
  @param  Table                 A pointer to the configuration table for the entry to add, update, or
                                remove. May be NULL.

  @retval EFI_SUCCESS           The (Guid, Table) pair was added, updated, or removed.
  @retval EFI_NOT_FOUND         An attempt was made to delete a nonexistent entry.
  @retval EFI_INVALID_PARAMETER Guid is not valid.
  @retval EFI_OUT_OF_RESOURCES  There is not enough memory available to complete the operation.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_INSTALL_CONFIGURATION_TABLE) (
  IN EFI_GUID                 *Guid,
  IN VOID                     *Table
  );


/**
  Returns an array of handles that support the requested protocol in a buffer allocated from pool.

  @param  SearchType            Specifies which handle(s) are to be returned.
  @param  Protocol              Specifies the protocol to search by.
  @param  SearchKey             Supplies the search key depending on the SearchType.
  @param  NoHandles             The number of handles returned in Buffer.
  @param  Buffer                A pointer to the buffer to return the requested array of handles that
                                support Protocol.

  @retval EFI_SUCCESS           The array of handles was returned in Buffer, and the number of
                                handles in Buffer was returned in NoHandles.
  @retval EFI_NOT_FOUND         No handles match the search.
  @retval EFI_OUT_OF_RESOURCES  There is not enough pool memory to store the matching results.
  @retval EFI_INVALID_PARAMETER One or more parameters are invalid.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_LOCATE_HANDLE_BUFFER) (
  IN     EFI_LOCATE_SEARCH_TYPE       SearchType,
  IN     EFI_GUID                     *Protocol,      OPTIONAL
  IN     VOID                         *SearchKey,     OPTIONAL
  IN OUT UINTN                        *NoHandles,
  OUT    EFI_HANDLE                   **Buffer
  );

/**
  Returns the first protocol instance that matches the given protocol.

  @param  Protocol              Provides the protocol to search for.
  @param  Registration          Optional registration key returned from
                                RegisterProtocolNotify().
  @param  Interface             On return, a pointer to the first interface that matches Protocol and
                                Registration.

  @retval EFI_SUCCESS           A protocol instance matching Protocol was found and returned in
                                Interface.
  @retval EFI_NOT_FOUND         No protocol instances were found that match Protocol and
                                Registration.
  @retval EFI_INVALID_PARAMETER Interface is NULL.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_LOCATE_PROTOCOL) (
  IN  EFI_GUID  *Protocol,
  IN  VOID      *Registration, OPTIONAL
  OUT VOID      **Interface
  );

typedef struct {
  UINT64                            Length;
  union {
    EFI_PHYSICAL_ADDRESS  DataBlock;
    EFI_PHYSICAL_ADDRESS  ContinuationPointer;
  } Union;
} UEFI_CAPSULE_BLOCK_DESCRIPTOR;

typedef struct {
  EFI_GUID          CapsuleGuid;
  UINT32            HeaderSize;
  UINT32            Flags;
  UINT32            CapsuleImageSize;
} UEFI_CAPSULE_HEADER;

#define CAPSULE_FLAGS_PERSIST_ACROSS_RESET          0x00010000
#define CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE         0x00020000

/**
  Passes capsules to the firmware with both virtual and physical mapping. Depending on the intended
  consumption, the firmware may process the capsule immediately. If the payload should persist
  across a system reset, the reset value returned from EFI_QueryCapsuleCapabilities must
  be passed into ResetSystem() and will cause the capsule to be processed by the firmware as
  part of the reset process.

  @param  CapsuleHeaderArray    Virtual pointer to an array of virtual pointers to the capsules
                                being passed into update capsule.
  @param  CapsuleCount          Number of pointers to EFI_CAPSULE_HEADER in
                                CaspuleHeaderArray.
  @param  ScatterGatherList     Physical pointer to a set of
                                EFI_CAPSULE_BLOCK_DESCRIPTOR that describes the
                                location in physical memory of a set of capsules.

  @retval EFI_SUCCESS           Valid capsule was passed. If
                                CAPSULE_FLAGS_PERSIT_ACROSS_RESET is not set, the
                                capsule has been successfully processed by the firmware.
  @retval EFI_DEVICE_ERROR      The capsule update was started, but failed due to a device error.
  @retval EFI_INVALID_PARAMETER CapsuleSize is NULL.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_UPDATE_CAPSULE) (
  IN UEFI_CAPSULE_HEADER    **CapsuleHeaderArray,
  IN UINTN                  CapsuleCount,
  IN EFI_PHYSICAL_ADDRESS   ScatterGatherList   OPTIONAL
  );

/**
  Returns if the capsule can be supported via UpdateCapsule().

  @param  CapsuleHeaderArray    Virtual pointer to an array of virtual pointers to the capsules
                                being passed into update capsule.
  @param  CapsuleCount          Number of pointers to EFI_CAPSULE_HEADER in
                                CaspuleHeaderArray.
  @param  MaxiumCapsuleSize     On output the maximum size that UpdateCapsule() can
                                support as an argument to UpdateCapsule() via
                                CapsuleHeaderArray and ScatterGatherList.
  @param  ResetType             Returns the type of reset required for the capsule update.

  @retval EFI_SUCCESS           Valid answer returned.
  @retval EFI_UNSUPPORTED       The capsule type is not supported on this platform, and
                                MaximumCapsuleSize and ResetType are undefined.
  @retval EFI_INVALID_PARAMETER MaximumCapsuleSize is NULL.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_QUERY_CAPSULE_CAPABILITIES) (
  IN  UEFI_CAPSULE_HEADER    **CapsuleHeaderArray,
  IN  UINTN                  CapsuleCount,
  OUT UINT64                 *MaximumCapsuleSize,
  OUT EFI_RESET_TYPE         *ResetType
  );

/**
  Returns information about the EFI variables.

  @param  Attributes                   Attributes bitmask to specify the type of variables on
                                       which to return information.
  @param  MaximumVariableStorageSize   On output the maximum size of the storage space
                                       available for the EFI variables associated with the
                                       attributes specified.
  @param  RemainingVariableStorageSize Returns the remaining size of the storage space
                                       available for the EFI variables associated with the
                                       attributes specified.
  @param  MaximumVariableSize          Returns the maximum size of the individual EFI
                                       variables associated with the attributes specified.

  @retval EFI_SUCCESS                  Valid answer returned.
  @retval EFI_INVALID_PARAMETER        An invalid combination of attribute bits was supplied
  @retval EFI_UNSUPPORTED              The attribute is not supported on this platform, and the
                                       MaximumVariableStorageSize,
                                       RemainingVariableStorageSize, MaximumVariableSize
                                       are undefined.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_QUERY_VARIABLE_INFO) (
  IN  UINT32            Attributes,
  OUT UINT64            *MaximumVariableStorageSize,
  OUT UINT64            *RemainingVariableStorageSize,
  OUT UINT64            *MaximumVariableSize
  );


//
// EFI Runtime Services Table
//
#define EFI_SYSTEM_TABLE_SIGNATURE      0x5453595320494249
#define EFI_SYSTEM_TABLE_REVISION       ((2<<16) | (10))
#define EFI_2_10_SYSTEM_TABLE_REVISION  ((2<<16) | (10))
#define EFI_2_00_SYSTEM_TABLE_REVISION  ((2<<16) | (00))
#define EFI_1_10_SYSTEM_TABLE_REVISION  ((1<<16) | (10))
#define EFI_1_02_SYSTEM_TABLE_REVISION  ((1<<16) | (02))

#define EFI_RUNTIME_SERVICES_SIGNATURE  0x56524553544e5552
#define EFI_RUNTIME_SERVICES_REVISION   EFI_2_10_SYSTEM_TABLE_REVISION

typedef struct {
  EFI_TABLE_HEADER Hdr;

  //
  // Time Services
  //
  EFI_GET_TIME                    GetTime;
  EFI_SET_TIME                    SetTime;
  EFI_GET_WAKEUP_TIME             GetWakeupTime;
  EFI_SET_WAKEUP_TIME             SetWakeupTime;

  //
  // Virtual Memory Services
  //
  EFI_SET_VIRTUAL_ADDRESS_MAP     SetVirtualAddressMap;
  EFI_CONVERT_POINTER             ConvertPointer;

  //
  // Variable Services
  //
  EFI_GET_VARIABLE                GetVariable;
  EFI_GET_NEXT_VARIABLE_NAME      GetNextVariableName;
  EFI_SET_VARIABLE                SetVariable;

  //
  // Miscellaneous Services
  //
  EFI_GET_NEXT_HIGH_MONO_COUNT    GetNextHighMonotonicCount;
  EFI_RESET_SYSTEM                ResetSystem;

  //
  // UEFI 2.0 Capsule Services
  //
  EFI_UPDATE_CAPSULE              UpdateCapsule;
  EFI_QUERY_CAPSULE_CAPABILITIES  QueryCapsuleCapabilities;

  //
  // Miscellaneous UEFI 2.0 Service
  //
  EFI_QUERY_VARIABLE_INFO         QueryVariableInfo;
} EFI_RUNTIME_SERVICES;


#define EFI_BOOT_SERVICES_SIGNATURE   0x56524553544f4f42
#define EFI_BOOT_SERVICES_REVISION    EFI_2_10_SYSTEM_TABLE_REVISION

typedef struct {
  EFI_TABLE_HEADER  Hdr;
  
  //
  // Task Priority Services
  //
  EFI_RAISE_TPL                   RaiseTPL;
  EFI_RESTORE_TPL                 RestoreTPL;

  //
  // Memory Services
  //
  EFI_ALLOCATE_PAGES              AllocatePages;
  EFI_FREE_PAGES                  FreePages;
  EFI_GET_MEMORY_MAP              GetMemoryMap;
  EFI_ALLOCATE_POOL               AllocatePool;
  EFI_FREE_POOL                   FreePool;

  //
  // Event & Timer Services
  //
  EFI_CREATE_EVENT                  CreateEvent;
  EFI_SET_TIMER                     SetTimer;
  EFI_WAIT_FOR_EVENT                WaitForEvent;
  EFI_SIGNAL_EVENT                  SignalEvent;
  EFI_CLOSE_EVENT                   CloseEvent;
  EFI_CHECK_EVENT                   CheckEvent;

  //
  // Protocol Handler Services
  //
  EFI_INSTALL_PROTOCOL_INTERFACE    InstallProtocolInterface;
  EFI_REINSTALL_PROTOCOL_INTERFACE  ReinstallProtocolInterface;
  EFI_UNINSTALL_PROTOCOL_INTERFACE  UninstallProtocolInterface;
  EFI_HANDLE_PROTOCOL               HandleProtocol;
  VOID                              *Reserved;
  EFI_REGISTER_PROTOCOL_NOTIFY      RegisterProtocolNotify;
  EFI_LOCATE_HANDLE                 LocateHandle;
  EFI_LOCATE_DEVICE_PATH            LocateDevicePath;
  EFI_INSTALL_CONFIGURATION_TABLE   InstallConfigurationTable;

  //
  // Image Services
  //
  EFI_IMAGE_LOAD                    LoadImage;
  EFI_IMAGE_START                   StartImage;
  EFI_EXIT                          Exit;
  EFI_IMAGE_UNLOAD                  UnloadImage;
  EFI_EXIT_BOOT_SERVICES            ExitBootServices;

  //
  // Miscellaneous Services
  //
  EFI_GET_NEXT_MONOTONIC_COUNT      GetNextMonotonicCount;
  EFI_STALL                         Stall;
  EFI_SET_WATCHDOG_TIMER            SetWatchdogTimer;

  //
  // DriverSupport Services
  //
  EFI_CONNECT_CONTROLLER            ConnectController;
  EFI_DISCONNECT_CONTROLLER         DisconnectController;

  //
  // Open and Close Protocol Services
  //
  EFI_OPEN_PROTOCOL                 OpenProtocol;
  EFI_CLOSE_PROTOCOL                CloseProtocol;
  EFI_OPEN_PROTOCOL_INFORMATION     OpenProtocolInformation;

  //
  // Library Services
  //
  EFI_PROTOCOLS_PER_HANDLE          ProtocolsPerHandle;
  EFI_LOCATE_HANDLE_BUFFER          LocateHandleBuffer;
  EFI_LOCATE_PROTOCOL               LocateProtocol;
  EFI_INSTALL_MULTIPLE_PROTOCOL_INTERFACES    InstallMultipleProtocolInterfaces;
  EFI_UNINSTALL_MULTIPLE_PROTOCOL_INTERFACES  UninstallMultipleProtocolInterfaces;

  //
  // 32-bit CRC Services
  //
  EFI_CALCULATE_CRC32               CalculateCrc32;

  //
  // Miscellaneous Services
  //
  EFI_COPY_MEM                      CopyMem;
  EFI_SET_MEM                       SetMem;

  EFI_CREATE_EVENT_EX               CreateEventEx;
} EFI_BOOT_SERVICES;

//
// Contains a set of GUID/pointer pairs comprised of the ConfigurationTable field in the 
// EFI System Table.
// 
typedef struct{
  EFI_GUID                          VendorGuid;
  VOID                              *VendorTable;
} EFI_CONFIGURATION_TABLE;

struct _EFI_SYSTEM_TABLE {
  EFI_TABLE_HEADER  Hdr;
  CHAR16                            *FirmwareVendor;
  UINT32                            FirmwareRevision;
  EFI_HANDLE                        ConsoleInHandle;
  EFI_SIMPLE_TEXT_INPUT_PROTOCOL    *ConIn;
  EFI_HANDLE                        ConsoleOutHandle;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL   *ConOut;
  EFI_HANDLE                        StandardErrorHandle;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL   *StdErr;
  EFI_RUNTIME_SERVICES              *RuntimeServices;
  EFI_BOOT_SERVICES                 *BootServices;
  UINTN                             NumberOfTableEntries;
  EFI_CONFIGURATION_TABLE           *ConfigurationTable;
};

//
// EFI File location to boot from on removable media devices
//
#define EFI_REMOVABLE_MEDIA_FILE_NAME_IA32    L"\\EFI\\BOOT\\BOOTIA32.EFI"
#define EFI_REMOVABLE_MEDIA_FILE_NAME_IA64    L"\\EFI\\BOOT\\BOOTIA64.EFI"
#define EFI_REMOVABLE_MEDIA_FILE_NAME_X64     L"\\EFI\\BOOT\\BOOTX64.EFI"
#define EFI_REMOVABLE_MEDIA_FILE_NAME_EBC     L"\\EFI\\BOOT\\BOOTEBC.EFI"

#if   defined (MDE_CPU_IA32)
  #define EFI_REMOVABLE_MEDIA_FILE_NAME   EFI_REMOVABLE_MEDIA_FILE_NAME_IA32
#elif defined (MDE_CPU_IPF)
  #define EFI_REMOVABLE_MEDIA_FILE_NAME   EFI_REMOVABLE_MEDIA_FILE_NAME_IA64
#elif defined (MDE_CPU_X64)
  #define EFI_REMOVABLE_MEDIA_FILE_NAME   EFI_REMOVABLE_MEDIA_FILE_NAME_X64
#elif defined (MDE_CPU_EBC)
  #define EFI_REMOVABLE_MEDIA_FILE_NAME   EFI_REMOVABLE_MEDIA_FILE_NAME_EBC
#else
  #error Unknown Processor Type
#endif

#include <Uefi/UefiPxe.h>
#include <Uefi/UefiGpt.h>
#include <Uefi/UefiInternalFormRepresentation.h>

#endif
