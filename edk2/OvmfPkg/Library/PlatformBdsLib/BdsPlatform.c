/** @file
  Platform BDS customizations.

  Copyright (c) 2004 - 2008, Intel Corporation. <BR>
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "BdsPlatform.h"


//
// BDS Platform Functions
//
VOID
EFIAPI
PlatformBdsInit (
  VOID
  )
/*++

Routine Description:

  Platform Bds init. Incude the platform firmware vendor, revision
  and so crc check.

Arguments:

Returns:

  None.

--*/
{
  DEBUG ((EFI_D_INFO, "PlatformBdsInit\n"));
}


EFI_STATUS
ConnectRootBridge (
  VOID
  )
/*++

Routine Description:

  Connect RootBridge

Arguments:

  None.

Returns:

  EFI_SUCCESS             - Connect RootBridge successfully.
  EFI_STATUS              - Connect RootBridge fail.

--*/
{
  EFI_STATUS                Status;
  EFI_HANDLE                RootHandle;

  //
  // Make all the PCI_IO protocols on PCI Seg 0 show up
  //
  BdsLibConnectDevicePath (gPlatformRootBridges[0]);

  Status = gBS->LocateDevicePath (
                  &gEfiDevicePathProtocolGuid,
                  &gPlatformRootBridges[0],
                  &RootHandle
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->ConnectController (RootHandle, NULL, NULL, FALSE);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}


EFI_STATUS
PrepareLpcBridgeDevicePath (
  IN EFI_HANDLE                DeviceHandle
  )
/*++

Routine Description:

  Add IsaKeyboard to ConIn,
  add IsaSerial to ConOut, ConIn, ErrOut.
  LPC Bridge: 06 01 00

Arguments:

  DeviceHandle            - Handle of PCIIO protocol.

Returns:

  EFI_SUCCESS             - LPC bridge is added to ConOut, ConIn, and ErrOut.
  EFI_STATUS              - No LPC bridge is added.

--*/
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *TempDevicePath;
  CHAR16                    *DevPathStr;

  DevicePath = NULL;
  Status = gBS->HandleProtocol (
                  DeviceHandle,
                  &gEfiDevicePathProtocolGuid,
                  (VOID*)&DevicePath
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  TempDevicePath = DevicePath;

  //
  // Register Keyboard
  //
  DevicePath = AppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gPnpPs2KeyboardDeviceNode);

  BdsLibUpdateConsoleVariable (VarConsoleInp, DevicePath, NULL);

  //
  // Register COM1
  //
  DevicePath = TempDevicePath;
  gPnp16550ComPortDeviceNode.UID = 0;

  DevicePath = AppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gPnp16550ComPortDeviceNode);
  DevicePath = AppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gUartDeviceNode);
  DevicePath = AppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gTerminalTypeDeviceNode);

  //
  // Print Device Path
  //
  DevPathStr = DevicePathToStr(DevicePath);
  DEBUG((
    EFI_D_INFO,
    "BdsPlatform.c+%d: COM%d DevPath: %s\n",
    __LINE__,
    gPnp16550ComPortDeviceNode.UID + 1,
    DevPathStr
    ));
  FreePool(DevPathStr);

  BdsLibUpdateConsoleVariable (VarConsoleOut, DevicePath, NULL);
  BdsLibUpdateConsoleVariable (VarConsoleInp, DevicePath, NULL);
  BdsLibUpdateConsoleVariable (VarErrorOut, DevicePath, NULL);

  //
  // Register COM2
  //
  DevicePath = TempDevicePath;
  gPnp16550ComPortDeviceNode.UID = 1;

  DevicePath = AppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gPnp16550ComPortDeviceNode);
  DevicePath = AppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gUartDeviceNode);
  DevicePath = AppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gTerminalTypeDeviceNode);

  //
  // Print Device Path
  //
  DevPathStr = DevicePathToStr(DevicePath);
  DEBUG((
    EFI_D_INFO,
    "BdsPlatform.c+%d: COM%d DevPath: %s\n",
    __LINE__,
    gPnp16550ComPortDeviceNode.UID + 1,
    DevPathStr
    ));
  FreePool(DevPathStr);

  BdsLibUpdateConsoleVariable (VarConsoleOut, DevicePath, NULL);
  BdsLibUpdateConsoleVariable (VarConsoleInp, DevicePath, NULL);
  BdsLibUpdateConsoleVariable (VarErrorOut, DevicePath, NULL);

  return EFI_SUCCESS;
}

EFI_STATUS
GetGopDevicePath (
   IN  EFI_DEVICE_PATH_PROTOCOL *PciDevicePath,
   OUT EFI_DEVICE_PATH_PROTOCOL **GopDevicePath
   )
{
  UINTN                           Index;
  EFI_STATUS                      Status;
  EFI_HANDLE                      PciDeviceHandle;
  EFI_DEVICE_PATH_PROTOCOL        *TempDevicePath;
  EFI_DEVICE_PATH_PROTOCOL        *TempPciDevicePath;
  UINTN                           GopHandleCount;
  EFI_HANDLE                      *GopHandleBuffer;

  if (PciDevicePath == NULL || GopDevicePath == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Initialize the GopDevicePath to be PciDevicePath
  //
  *GopDevicePath    = PciDevicePath;
  TempPciDevicePath = PciDevicePath;

  Status = gBS->LocateDevicePath (
                  &gEfiDevicePathProtocolGuid,
                  &TempPciDevicePath,
                  &PciDeviceHandle
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Try to connect this handle, so that GOP dirver could start on this
  // device and create child handles with GraphicsOutput Protocol installed
  // on them, then we get device paths of these child handles and select
  // them as possible console device.
  //
  gBS->ConnectController (PciDeviceHandle, NULL, NULL, FALSE);

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiGraphicsOutputProtocolGuid,
                  NULL,
                  &GopHandleCount,
                  &GopHandleBuffer
                  );
  if (!EFI_ERROR (Status)) {
    //
    // Add all the child handles as possible Console Device
    //
    for (Index = 0; Index < GopHandleCount; Index++) {
      Status = gBS->HandleProtocol (GopHandleBuffer[Index], &gEfiDevicePathProtocolGuid, (VOID*)&TempDevicePath);
      if (EFI_ERROR (Status)) {
        continue;
      }
      if (CompareMem (
            PciDevicePath,
            TempDevicePath,
            GetDevicePathSize (PciDevicePath) - END_DEVICE_PATH_LENGTH
            ) == 0) {
        //
        // In current implementation, we only enable one of the child handles
        // as console device, i.e. sotre one of the child handle's device
        // path to variable "ConOut"
        // In futhure, we could select all child handles to be console device
        //

        *GopDevicePath = TempDevicePath;

        //
        // Delete the PCI device's path that added by GetPlugInPciVgaDevicePath()
        // Add the integrity GOP device path.
        //
        BdsLibUpdateConsoleVariable (VarConsoleOutDev, NULL, PciDevicePath);
        BdsLibUpdateConsoleVariable (VarConsoleOutDev, TempDevicePath, NULL);
      }
    }
    gBS->FreePool (GopHandleBuffer);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
PreparePciVgaDevicePath (
  IN EFI_HANDLE                DeviceHandle
  )
/*++

Routine Description:

  Add PCI VGA to ConOut.
  PCI VGA: 03 00 00

Arguments:

  DeviceHandle            - Handle of PCIIO protocol.

Returns:

  EFI_SUCCESS             - PCI VGA is added to ConOut.
  EFI_STATUS              - No PCI VGA device is added.

--*/
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *GopDevicePath;

  DevicePath = NULL;
  Status = gBS->HandleProtocol (
                  DeviceHandle,
                  &gEfiDevicePathProtocolGuid,
                  (VOID*)&DevicePath
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  GetGopDevicePath (DevicePath, &GopDevicePath);
  DevicePath = GopDevicePath;

  BdsLibUpdateConsoleVariable (VarConsoleOut, DevicePath, NULL);

  return EFI_SUCCESS;
}

EFI_STATUS
PreparePciSerialDevicePath (
  IN EFI_HANDLE                DeviceHandle
  )
/*++

Routine Description:

  Add PCI Serial to ConOut, ConIn, ErrOut.
  PCI Serial: 07 00 02

Arguments:

  DeviceHandle            - Handle of PCIIO protocol.

Returns:

  EFI_SUCCESS             - PCI Serial is added to ConOut, ConIn, and ErrOut.
  EFI_STATUS              - No PCI Serial device is added.

--*/
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;

  DevicePath = NULL;
  Status = gBS->HandleProtocol (
                  DeviceHandle,
                  &gEfiDevicePathProtocolGuid,
                  (VOID*)&DevicePath
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  DevicePath = AppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gUartDeviceNode);
  DevicePath = AppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gTerminalTypeDeviceNode);

  BdsLibUpdateConsoleVariable (VarConsoleOut, DevicePath, NULL);
  BdsLibUpdateConsoleVariable (VarConsoleInp, DevicePath, NULL);
  BdsLibUpdateConsoleVariable (VarErrorOut, DevicePath, NULL);

  return EFI_SUCCESS;
}

EFI_STATUS
DetectAndPreparePlatformPciDevicePath (
  BOOLEAN DetectVgaOnly
  )
/*++

Routine Description:

  Do platform specific PCI Device check and add them to ConOut, ConIn, ErrOut

Arguments:

  DetectVgaOnly           - Only detect VGA device if it's TRUE.

Returns:

  EFI_SUCCESS             - PCI Device check and Console variable update successfully.
  EFI_STATUS              - PCI Device check or Console variable update fail.

--*/
{
  EFI_STATUS                Status;
  UINTN                     HandleCount;
  EFI_HANDLE                *HandleBuffer;
  UINTN                     Index;
  EFI_PCI_IO_PROTOCOL       *PciIo;
  PCI_TYPE00                Pci;

  //
  // Start to check all the PciIo to find all possible device
  //
  HandleCount = 0;
  HandleBuffer = NULL;
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiPciIoProtocolGuid, (VOID*)&PciIo);
    if (EFI_ERROR (Status)) {
      continue;
    }

    //
    // Check for all PCI device
    //
    Status = PciIo->Pci.Read (
                      PciIo,
                      EfiPciIoWidthUint32,
                      0,
                      sizeof (Pci) / sizeof (UINT32),
                      &Pci
                      );
    if (EFI_ERROR (Status)) {
      continue;
    }

    if (!DetectVgaOnly) {
      //
      // Here we decide whether it is LPC Bridge
      //
      if ((IS_PCI_LPC (&Pci)) ||
          ((IS_PCI_ISA_PDECODE (&Pci)) &&
           (Pci.Hdr.VendorId == 0x8086) &&
           (Pci.Hdr.DeviceId == 0x7000)
          )
         ) {
        Status = PciIo->Attributes (
          PciIo,
          EfiPciIoAttributeOperationEnable,
          EFI_PCI_DEVICE_ENABLE,
          NULL
          );
        //
        // Add IsaKeyboard to ConIn,
        // add IsaSerial to ConOut, ConIn, ErrOut
        //
        DEBUG ((EFI_D_INFO, "Find the LPC Bridge device\n"));
        PrepareLpcBridgeDevicePath (HandleBuffer[Index]);
        continue;
      }
      //
      // Here we decide which Serial device to enable in PCI bus
      //
      if (IS_PCI_16550SERIAL (&Pci)) {
        //
        // Add them to ConOut, ConIn, ErrOut.
        //
        DEBUG ((EFI_D_INFO, "Find the 16550 SERIAL device\n"));
        PreparePciSerialDevicePath (HandleBuffer[Index]);
        continue;
      }
    }

    if ((Pci.Hdr.VendorId == 0x8086) &&
        (Pci.Hdr.DeviceId == 0x7010)
       ) {
      Status = PciIo->Attributes (
        PciIo,
        EfiPciIoAttributeOperationEnable,
        EFI_PCI_DEVICE_ENABLE,
        NULL
        );
     }

    //
    // Here we decide which VGA device to enable in PCI bus
    //
    if (IS_PCI_VGA (&Pci)) {
      //
      // Add them to ConOut.
      //
      DEBUG ((EFI_D_INFO, "Find the VGA device\n"));
      PreparePciVgaDevicePath (HandleBuffer[Index]);
      continue;
    }
  }

  gBS->FreePool (HandleBuffer);

  return EFI_SUCCESS;
}


EFI_STATUS
PlatformBdsConnectConsole (
  IN BDS_CONSOLE_CONNECT_ENTRY   *PlatformConsole
  )
/*++

Routine Description:

  Connect the predefined platform default console device. Always try to find
  and enable the vga device if have.

Arguments:

  PlatformConsole         - Predfined platform default console device array.

Returns:

  EFI_SUCCESS             - Success connect at least one ConIn and ConOut
                            device, there must have one ConOut device is
                            active vga device.

  EFI_STATUS              - Return the status of
                            BdsLibConnectAllDefaultConsoles ()

--*/
{
  EFI_STATUS                         Status;
  UINTN                              Index;
  EFI_DEVICE_PATH_PROTOCOL           *VarConout;
  EFI_DEVICE_PATH_PROTOCOL           *VarConin;
  UINTN                              DevicePathSize;

  //
  // Connect RootBridge
  //
  ConnectRootBridge ();

  VarConout = BdsLibGetVariableAndSize (
                VarConsoleOut,
                &gEfiGlobalVariableGuid,
                &DevicePathSize
                );
  VarConin = BdsLibGetVariableAndSize (
               VarConsoleInp,
               &gEfiGlobalVariableGuid,
               &DevicePathSize
               );

  if (VarConout == NULL || VarConin == NULL) {
    //
    // Do platform specific PCI Device check and add them to ConOut, ConIn, ErrOut
    //
    DetectAndPreparePlatformPciDevicePath (FALSE);

    //
    // Have chance to connect the platform default console,
    // the platform default console is the minimue device group
    // the platform should support
    //
    for (Index = 0; PlatformConsole[Index].DevicePath != NULL; ++Index) {
      //
      // Update the console variable with the connect type
      //
      if ((PlatformConsole[Index].ConnectType & CONSOLE_IN) == CONSOLE_IN) {
        BdsLibUpdateConsoleVariable (VarConsoleInp, PlatformConsole[Index].DevicePath, NULL);
      }
      if ((PlatformConsole[Index].ConnectType & CONSOLE_OUT) == CONSOLE_OUT) {
        BdsLibUpdateConsoleVariable (VarConsoleOut, PlatformConsole[Index].DevicePath, NULL);
      }
      if ((PlatformConsole[Index].ConnectType & STD_ERROR) == STD_ERROR) {
        BdsLibUpdateConsoleVariable (VarErrorOut, PlatformConsole[Index].DevicePath, NULL);
      }
    }
  } else {
    //
    // Only detect VGA device and add them to ConOut
    //
    DetectAndPreparePlatformPciDevicePath (TRUE);
  }

  //
  // Connect the all the default console with current cosole variable
  //
  Status = BdsLibConnectAllDefaultConsoles ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}


VOID
PciInitialization (
  )
{
  //
  // Device 0 Function 0
  //
  PciWrite8 (PCI_LIB_ADDRESS (0,0,0,0x3c), 0x00);

  //
  // Device 1 Function 0
  //
  PciWrite8 (PCI_LIB_ADDRESS (0,1,0,0x3c), 0x00);
  PciWrite8 (PCI_LIB_ADDRESS (0,1,0,0x60), 0x8b);
  PciWrite8 (PCI_LIB_ADDRESS (0,1,0,0x61), 0x89);
  PciWrite8 (PCI_LIB_ADDRESS (0,1,0,0x62), 0x0a);
  PciWrite8 (PCI_LIB_ADDRESS (0,1,0,0x63), 0x89);
  //PciWrite8 (PCI_LIB_ADDRESS (0,1,0,0x82), 0x02);

  //
  // Device 1 Function 1
  //
  PciWrite8 (PCI_LIB_ADDRESS (0,1,1,0x3c), 0x00);

  //
  // Device 1 Function 3
  //
  PciWrite8 (PCI_LIB_ADDRESS (0,1,3,0x3c), 0x0b);
  PciWrite8 (PCI_LIB_ADDRESS (0,1,3,0x3d), 0x01);
  PciWrite8 (PCI_LIB_ADDRESS (0,1,3,0x5f), 0x90);

  //
  // Device 2 Function 0
  //
  PciWrite8 (PCI_LIB_ADDRESS (0,2,0,0x3c), 0x00);

  //
  // Device 3 Function 0
  //
  PciWrite8 (PCI_LIB_ADDRESS (0,3,0,0x3c), 0x0b);
  PciWrite8 (PCI_LIB_ADDRESS (0,3,0,0x3d), 0x01);
}


VOID
PlatformBdsConnectSequence (
  VOID
  )
/*++

Routine Description:

  Connect with predeined platform connect sequence,
  the OEM/IBV can customize with their own connect sequence.

Arguments:

  None.

Returns:

  None.

--*/
{
  UINTN Index;

  DEBUG ((EFI_D_INFO, "PlatformBdsConnectSequence\n"));

  Index = 0;

  //
  // Here we can get the customized platform connect sequence
  // Notes: we can connect with new variable which record the
  // last time boots connect device path sequence
  //
  while (gPlatformConnectSequence[Index] != NULL) {
    //
    // Build the platform boot option
    //
    BdsLibConnectDevicePath (gPlatformConnectSequence[Index]);
    Index++;
  }

  //
  // Just use the simple policy to connect all devices
  //
  BdsLibConnectAll ();

  PciInitialization ();
}

VOID
PlatformBdsGetDriverOption (
  IN OUT LIST_ENTRY              *BdsDriverLists
  )
/*++

Routine Description:

  Load the predefined driver option, OEM/IBV can customize this
  to load their own drivers

Arguments:

  BdsDriverLists  - The header of the driver option link list.

Returns:

  None.

--*/
{
  DEBUG ((EFI_D_INFO, "PlatformBdsGetDriverOption\n"));
  return;
}

VOID
PlatformBdsDiagnostics (
  IN EXTENDMEM_COVERAGE_LEVEL    MemoryTestLevel,
  IN BOOLEAN                     QuietBoot
  )
/*++

Routine Description:

  Perform the platform diagnostic, such like test memory. OEM/IBV also
  can customize this fuction to support specific platform diagnostic.

Arguments:

  MemoryTestLevel  - The memory test intensive level

  QuietBoot        - Indicate if need to enable the quiet boot

Returns:

  None.

--*/
{
  EFI_STATUS  Status;

  DEBUG ((EFI_D_INFO, "PlatformBdsDiagnostics\n"));

  //
  // Here we can decide if we need to show
  // the diagnostics screen
  // Notes: this quiet boot code should be remove
  // from the graphic lib
  //
  if (QuietBoot) {
    EnableQuietBoot (&gEfiDefaultBmpLogoGuid);
    //
    // Perform system diagnostic
    //
    Status = BdsMemoryTest (MemoryTestLevel);
    if (EFI_ERROR (Status)) {
      DisableQuietBoot ();
    }

    return ;
  }
  //
  // Perform system diagnostic
  //
  Status = BdsMemoryTest (MemoryTestLevel);
}


VOID
EFIAPI
PlatformBdsPolicyBehavior (
  IN OUT LIST_ENTRY                  *DriverOptionList,
  IN OUT LIST_ENTRY                  *BootOptionList
  )
/*++

Routine Description:

  The function will excute with as the platform policy, current policy
  is driven by boot mode. IBV/OEM can customize this code for their specific
  policy action.

Arguments:

  DriverOptionList - The header of the driver option link list

  BootOptionList   - The header of the boot option link list

Returns:

  None.

--*/
{
  EFI_STATUS                         Status;
  UINT16                             Timeout;
  EFI_EVENT                          UserInputDurationTime;
  LIST_ENTRY                     *Link;
  BDS_COMMON_OPTION                  *BootOption;
  UINTN                              Index;
  EFI_INPUT_KEY                      Key;
  EFI_TPL                            OldTpl;
  EFI_BOOT_MODE                      BootMode;

  DEBUG ((EFI_D_INFO, "PlatformBdsPolicyBehavior\n"));

  //
  // Init the time out value
  //
  Timeout = PcdGet16 (PcdPlatformBootTimeOut);

  //
  // Load the driver option as the driver option list
  //
  PlatformBdsGetDriverOption (DriverOptionList);

  //
  // Get current Boot Mode
  //
  Status = BdsLibGetBootMode (&BootMode);
  DEBUG ((EFI_D_ERROR, "Boot Mode:%x\n", BootMode));

  //
  // Go the different platform policy with different boot mode
  // Notes: this part code can be change with the table policy
  //
  ASSERT (BootMode == BOOT_WITH_FULL_CONFIGURATION);
  //
  // Connect platform console
  //
  Status = PlatformBdsConnectConsole (gPlatformConsole);
  if (EFI_ERROR (Status)) {
    //
    // Here OEM/IBV can customize with defined action
    //
    PlatformBdsNoConsoleAction ();
  }
  //
  // Create a 300ms duration event to ensure user has enough input time to enter Setup
  //
  Status = gBS->CreateEvent (
                  EVT_TIMER,
                  0,
                  NULL,
                  NULL,
                  &UserInputDurationTime
                  );
  ASSERT (Status == EFI_SUCCESS);
  Status = gBS->SetTimer (UserInputDurationTime, TimerRelative, 3000000);
  ASSERT (Status == EFI_SUCCESS);
  //
  // Memory test and Logo show
  //
  PlatformBdsDiagnostics (IGNORE, TRUE);

  //
  // Perform some platform specific connect sequence
  //
  PlatformBdsConnectSequence ();

  //
  // Give one chance to enter the setup if we
  // have the time out
  //
  if (Timeout != 0) {
    //PlatformBdsEnterFrontPage (Timeout, FALSE);
  }

  DEBUG ((EFI_D_INFO, "BdsLibConnectAll\n"));
  BdsLibConnectAll ();
  BdsLibEnumerateAllBootOption (BootOptionList);

  //
  // Please uncomment above ConnectAll and EnumerateAll code and remove following first boot
  // checking code in real production tip.
  //
  // In BOOT_WITH_FULL_CONFIGURATION boot mode, should always connect every device
  // and do enumerate all the default boot options. But in development system board, the boot mode
  // cannot be BOOT_ASSUMING_NO_CONFIGURATION_CHANGES because the machine box
  // is always open. So the following code only do the ConnectAll and EnumerateAll at first boot.
  //
  Status = BdsLibBuildOptionFromVar (BootOptionList, L"BootOrder");
  if (EFI_ERROR(Status)) {
    //
    // If cannot find "BootOrder" variable,  it may be first boot.
    // Try to connect all devices and enumerate all boot options here.
    //
    BdsLibConnectAll ();
    BdsLibEnumerateAllBootOption (BootOptionList);
  }

  //
  // To give the User a chance to enter Setup here, if user set TimeOut is 0.
  // BDS should still give user a chance to enter Setup
  //
  // Connect first boot option, and then check user input before exit
  //
  for (Link = BootOptionList->ForwardLink; Link != BootOptionList;Link = Link->ForwardLink) {
    BootOption = CR (Link, BDS_COMMON_OPTION, Link, BDS_LOAD_OPTION_SIGNATURE);
    if (!IS_LOAD_OPTION_TYPE (BootOption->Attribute, LOAD_OPTION_ACTIVE)) {
      //
      // skip the header of the link list, becuase it has no boot option
      //
      continue;
    } else {
      //
      // Make sure the boot option device path connected, but ignore the BBS device path
      //
      if (DevicePathType (BootOption->DevicePath) != BBS_DEVICE_PATH) {
        BdsLibConnectDevicePath (BootOption->DevicePath);
      }
      break;
    }
  }

  //
  // Check whether the user input after the duration time has expired
  //
  OldTpl = EfiGetCurrentTpl();
  gBS->RestoreTPL (TPL_APPLICATION);
  gBS->WaitForEvent (1, &UserInputDurationTime, &Index);
  gBS->CloseEvent (UserInputDurationTime);
  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  gBS->RaiseTPL (OldTpl);

  if (!EFI_ERROR (Status)) {
    //
    // Enter Setup if user input
    //
    Timeout = 0xffff;
    PlatformBdsEnterFrontPage (Timeout, FALSE);
  }

  return ;
}

VOID
EFIAPI
PlatformBdsBootSuccess (
  IN  BDS_COMMON_OPTION   *Option
  )
/*++

Routine Description:

  Hook point after a boot attempt succeeds. We don't expect a boot option to
  return, so the EFI 1.0 specification defines that you will default to an
  interactive mode and stop processing the BootOrder list in this case. This
  is alos a platform implementation and can be customized by IBV/OEM.

Arguments:

  Option - Pointer to Boot Option that succeeded to boot.

Returns:

  None.

--*/
{
  CHAR16  *TmpStr;

  DEBUG ((EFI_D_INFO, "PlatformBdsBootSuccess\n"));
  //
  // If Boot returned with EFI_SUCCESS and there is not in the boot device
  // select loop then we need to pop up a UI and wait for user input.
  //
  TmpStr = Option->StatusString;
  if (TmpStr != NULL) {
    BdsLibOutputStrings (gST->ConOut, TmpStr, Option->Description, L"\n\r", NULL);
    FreePool (TmpStr);
  }
}

VOID
EFIAPI
PlatformBdsBootFail (
  IN  BDS_COMMON_OPTION  *Option,
  IN  EFI_STATUS         Status,
  IN  CHAR16             *ExitData,
  IN  UINTN              ExitDataSize
  )
/*++

Routine Description:

  Hook point after a boot attempt fails.

Arguments:

  Option - Pointer to Boot Option that failed to boot.

  Status - Status returned from failed boot.

  ExitData - Exit data returned from failed boot.

  ExitDataSize - Exit data size returned from failed boot.

Returns:

  None.

--*/
{
  CHAR16  *TmpStr;

  DEBUG ((EFI_D_INFO, "PlatformBdsBootFail\n"));

  //
  // If Boot returned with failed status then we need to pop up a UI and wait
  // for user input.
  //
  TmpStr = Option->StatusString;
  if (TmpStr != NULL) {
    BdsLibOutputStrings (gST->ConOut, TmpStr, Option->Description, L"\n\r", NULL);
    FreePool (TmpStr);
  }
}

EFI_STATUS
PlatformBdsNoConsoleAction (
  VOID
  )
/*++

Routine Description:

  This function is remained for IBV/OEM to do some platform action,
  if there no console device can be connected.

Arguments:

  None.

Returns:

  EFI_SUCCESS      - Direct return success now.

--*/
{
  DEBUG ((EFI_D_INFO, "PlatformBdsNoConsoleAction\n"));
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PlatformBdsLockNonUpdatableFlash (
  VOID
  )
{
  DEBUG ((EFI_D_INFO, "PlatformBdsLockNonUpdatableFlash\n"));
  return EFI_SUCCESS;
}
