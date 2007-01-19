/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    init.c

Abstract:

    Initialization functions for EFI UNDI32 driver

Revision History

--*/

#include "undi32.h"

//
// Global Variables
//
PXE_SW_UNDI             *pxe = 0;     // 3.0 entry point
PXE_SW_UNDI             *pxe_31 = 0;  // 3.1 entry
UNDI32_DEV              *UNDI32DeviceList[MAX_NIC_INTERFACES];

NII_TABLE               *UnidiDataPointer=NULL;    
//
// external Global Variables
//
extern UNDI_CALL_TABLE  api_table[];

//
// function prototypes
//
EFI_STATUS
InstallConfigTable (
  IN VOID
  );

EFI_STATUS
EFIAPI
InitializeUNDIDriver (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

VOID
UNDI_notify_virtual (
  EFI_EVENT event,
  VOID      *context
  );

VOID
EFIAPI
UndiNotifyExitBs (
  EFI_EVENT Event,
  VOID      *Context
  );

EFI_STATUS
EFIAPI
UndiDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
UndiDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
UndiDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  );

EFI_STATUS
AppendMac2DevPath (
  IN OUT  EFI_DEVICE_PATH_PROTOCOL **DevPtr,
  IN      EFI_DEVICE_PATH_PROTOCOL *BaseDevPtr,
  IN      NIC_DATA_INSTANCE        *AdapterInfo
  );
//
// end function prototypes
//
VOID
EFIAPI
UndiNotifyVirtual (
  EFI_EVENT Event,
  VOID      *Context
  )
/*++

Routine Description:

  When address mapping changes to virtual this should make the appropriate
  address conversions.

Arguments:

  (Standard Event handler)

Returns:

  None

--*/
// TODO:    Context - add argument and description to function comment
{
  UINT16  Index;
  VOID    *Pxe31Pointer;

  if (pxe_31 != NULL) {
    Pxe31Pointer = (VOID *) pxe_31;

    EfiConvertPointer (
      EFI_OPTIONAL_POINTER,
      (VOID **) &Pxe31Pointer
      );

    //
    // UNDI32DeviceList is an array of pointers
    //
    for (Index = 0; Index < pxe_31->IFcnt; Index++) {
      UNDI32DeviceList[Index]->NIIProtocol_31.ID = (UINT64) (UINTN) Pxe31Pointer;
      EfiConvertPointer (
        EFI_OPTIONAL_POINTER,
        (VOID **) &(UNDI32DeviceList[Index])
        );
    }

    EfiConvertPointer (
      EFI_OPTIONAL_POINTER,
      (VOID **) &(pxe_31->EntryPoint)
      );
    pxe_31 = Pxe31Pointer;
  }

  for (Index = 0; Index <= PXE_OPCODE_LAST_VALID; Index++) {
    EfiConvertPointer (
      EFI_OPTIONAL_POINTER,
      (VOID **) &api_table[Index].api_ptr
      );
  }
}

VOID
EFIAPI
UndiNotifyExitBs (
  EFI_EVENT Event,
  VOID      *Context
  )
/*++

Routine Description:

  When EFI is shuting down the boot services, we need to install a 
  configuration table for UNDI to work at runtime!

Arguments:

  (Standard Event handler)

Returns:

  None

--*/
// TODO:    Context - add argument and description to function comment
{
  InstallConfigTable ();
}
//
// UNDI Class Driver Global Variables
//
EFI_DRIVER_BINDING_PROTOCOL  gUndiDriverBinding = {
  UndiDriverSupported,
  UndiDriverStart,
  UndiDriverStop,
  0xa,
  NULL,
  NULL
};

EFI_STATUS
EFIAPI
InitializeUNDIDriver (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  Register Driver Binding protocol for this driver.

Arguments:

  ImageHandle - Image Handle
  
  SystemTable - Pointer to system table

Returns:

  EFI_SUCCESS - Driver loaded.
  
  other       - Driver not loaded.

--*/
{
  EFI_STATUS  Status;
  EFI_EVENT   Event;

  Status = gBS->CreateEvent (
                  EFI_EVENT_SIGNAL_EXIT_BOOT_SERVICES,
                  EFI_TPL_NOTIFY,
                  UndiNotifyExitBs,
                  NULL,
                  &Event
                  );

  return Status;
}

EFI_STATUS
EFIAPI
UndiDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
/*++

Routine Description:

  Test to see if this driver supports ControllerHandle. Any ControllerHandle
  than contains a  DevicePath, PciIo protocol, Class code of 2, Vendor ID of 0x8086,
  and DeviceId of (D100_DEVICE_ID || D102_DEVICE_ID || ICH3_DEVICE_ID_1 ||
  ICH3_DEVICE_ID_2 || ICH3_DEVICE_ID_3 || ICH3_DEVICE_ID_4 || ICH3_DEVICE_ID_5 ||
  ICH3_DEVICE_ID_6 || ICH3_DEVICE_ID_7 || ICH3_DEVICE_ID_8) can be supported.

Arguments:

  This                - Protocol instance pointer.
  
  Controller          - Handle of device to test.
  
  RemainingDevicePath - Not used.

Returns:

  EFI_SUCCESS         - This driver supports this device.
  
  other               - This driver does not support this device.

--*/
{
  EFI_STATUS          Status;
  EFI_PCI_IO_PROTOCOL *PciIo;
  PCI_TYPE00          Pci;

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  NULL,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint8,
                        0,
                        sizeof (PCI_CONFIG_HEADER),
                        &Pci
                        );

  if (!EFI_ERROR (Status)) {
    Status = EFI_UNSUPPORTED;

    if (Pci.Hdr.ClassCode[2] == 0x02 && Pci.Hdr.VendorId == PCI_VENDOR_ID_INTEL) {
      switch (Pci.Hdr.DeviceId) {
      case D100_DEVICE_ID:
      case D102_DEVICE_ID:
      case ICH3_DEVICE_ID_1:
      case ICH3_DEVICE_ID_2:
      case ICH3_DEVICE_ID_3:
      case ICH3_DEVICE_ID_4:
      case ICH3_DEVICE_ID_5:
      case ICH3_DEVICE_ID_6:
      case ICH3_DEVICE_ID_7:
      case ICH3_DEVICE_ID_8:
      case 0x1039:
      case 0x103A:
      case 0x103B:
      case 0x103C:
      case 0x103D:
      case 0x103E:
      case 0x1050:
      case 0x1051:
      case 0x1052:
      case 0x1053:
      case 0x1054:
      case 0x1055:
      case 0x1056:
      case 0x1057:
      case 0x1059:
      case 0x1064:
        Status = EFI_SUCCESS;
      }
    }
  }

  gBS->CloseProtocol (
        Controller,
        &gEfiPciIoProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

  return Status;
}

EFI_STATUS
EFIAPI
UndiDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
/*++

Routine Description:

  Start this driver on Controller by opening PciIo and DevicePath protocol.
  Initialize PXE structures, create a copy of the Controller Device Path with the
  NIC's MAC address appended to it, install the NetworkInterfaceIdentifier protocol
  on the newly created Device Path.

Arguments:

  This                - Protocol instance pointer.
  
  Controller          - Handle of device to work with.
  
  RemainingDevicePath - Not used, always produce all possible children.

Returns:

  EFI_SUCCESS         - This driver is added to Controller.
  
  other               - This driver does not support this device.

--*/
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *UndiDevicePath;
  PCI_CONFIG_HEADER         *CfgHdr;
  UNDI32_DEV                *UNDI32Device;
  UINT16                    NewCommand;
  UINT8                     *TmpPxePointer;
  EFI_PCI_IO_PROTOCOL       *PciIoFncs;
  UINTN                     Len;   

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIoFncs,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &UndiDevicePath,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (EFI_ERROR (Status)) {
    gBS->CloseProtocol (
          Controller,
          &gEfiPciIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );

    return Status;
  }

  Status = gBS->AllocatePool (
                  EfiRuntimeServicesData,
                  sizeof (UNDI32_DEV),
                  (VOID **) &UNDI32Device
                  );

  if (EFI_ERROR (Status)) {
    goto UndiError;
  }

  ZeroMem ((CHAR8 *) UNDI32Device, sizeof (UNDI32_DEV));

  //
  // allocate and initialize both (old and new) the !pxe structures here,
  // there should only be one copy of each of these structure for any number
  // of NICs this undi supports. Also, these structures need to be on a
  // paragraph boundary as per the spec. so, while allocating space for these,
  // make sure that there is space for 2 !pxe structures (old and new) and a
  // 32 bytes padding for alignment adjustment (in case)
  //
  TmpPxePointer = NULL;
  if (pxe_31 == NULL) {
    Status = gBS->AllocatePool (
                    EfiRuntimeServicesData,
                    (sizeof (PXE_SW_UNDI) + sizeof (PXE_SW_UNDI) + 32),
                    (VOID **) &TmpPxePointer
                    );

    if (EFI_ERROR (Status)) {
      goto UndiErrorDeleteDevice;
    }

    ZeroMem (
      TmpPxePointer,
      sizeof (PXE_SW_UNDI) + sizeof (PXE_SW_UNDI) + 32
      );
    //
    // check for paragraph alignment here, assuming that the pointer is
    // already 8 byte aligned.
    //
    if (((UINTN) TmpPxePointer & 0x0F) != 0) {
      pxe_31 = (PXE_SW_UNDI *) ((UINTN) (TmpPxePointer + 8));
    } else {
      pxe_31 = (PXE_SW_UNDI *) TmpPxePointer;
    }
    //
    // assuming that the sizeof pxe_31 is a 16 byte multiple
    //
    pxe = (PXE_SW_UNDI *) ((CHAR8 *) (pxe_31) + sizeof (PXE_SW_UNDI));

    PxeStructInit (pxe, 0x30);
    PxeStructInit (pxe_31, 0x31);
  }

  UNDI32Device->NIIProtocol.ID    = (UINT64) (UINTN) (pxe);
  UNDI32Device->NIIProtocol_31.ID = (UINT64) (UINTN) (pxe_31);

  Status = PciIoFncs->Attributes (
                        PciIoFncs,
                        EfiPciIoAttributeOperationEnable,
                        EFI_PCI_DEVICE_ENABLE | EFI_PCI_IO_ATTRIBUTE_MEMORY | EFI_PCI_IO_ATTRIBUTE_IO | EFI_PCI_IO_ATTRIBUTE_BUS_MASTER,
                        NULL
                        );
  //
  // Read all the registers from device's PCI Configuration space
  //
  Status = PciIoFncs->Pci.Read (
                            PciIoFncs,
                            EfiPciIoWidthUint32,
                            0,
                            MAX_PCI_CONFIG_LEN,
                            &UNDI32Device->NicInfo.Config
                            );

  CfgHdr = (PCI_CONFIG_HEADER *) &(UNDI32Device->NicInfo.Config[0]);

  //
  // make sure that this device is a PCI bus master
  //

  NewCommand = (UINT16) (CfgHdr->Command | PCI_COMMAND_MASTER | PCI_COMMAND_IO);
  if (CfgHdr->Command != NewCommand) {
    PciIoFncs->Pci.Write (
                    PciIoFncs,
                    EfiPciIoWidthUint16,
                    PCI_COMMAND,
                    1,
                    &NewCommand
                    );
    CfgHdr->Command = NewCommand;
  }

  //
  // make sure that the latency timer is at least 32
  //
  if (CfgHdr->LatencyTimer < 32) {
    CfgHdr->LatencyTimer = 32;
    PciIoFncs->Pci.Write (
                    PciIoFncs,
                    EfiPciIoWidthUint8,
                    PCI_LATENCY_TIMER,
                    1,
                    &CfgHdr->LatencyTimer
                    );
  }
  //
  // the IfNum index for the current interface will be the total number
  // of interfaces initialized so far
  //
  UNDI32Device->NIIProtocol.IfNum     = pxe->IFcnt;
  UNDI32Device->NIIProtocol_31.IfNum  = pxe_31->IFcnt;

  PxeUpdate (&UNDI32Device->NicInfo, pxe);
  PxeUpdate (&UNDI32Device->NicInfo, pxe_31);

  UNDI32Device->NicInfo.Io_Function                 = PciIoFncs;
  UNDI32DeviceList[UNDI32Device->NIIProtocol.IfNum] = UNDI32Device;
  UNDI32Device->Undi32BaseDevPath                   = UndiDevicePath;

  Status = AppendMac2DevPath (
            &UNDI32Device->Undi32DevPath,
            UNDI32Device->Undi32BaseDevPath,
            &UNDI32Device->NicInfo
            );

  if (Status != 0) {
    goto UndiErrorDeletePxe;
  }

  UNDI32Device->Signature                     = UNDI_DEV_SIGNATURE;

  UNDI32Device->NIIProtocol.Revision          = EFI_NETWORK_INTERFACE_IDENTIFIER_PROTOCOL_REVISION;
  UNDI32Device->NIIProtocol.Type              = EfiNetworkInterfaceUndi;
  UNDI32Device->NIIProtocol.MajorVer          = PXE_ROMID_MAJORVER;
  UNDI32Device->NIIProtocol.MinorVer          = PXE_ROMID_MINORVER;
  UNDI32Device->NIIProtocol.ImageSize         = 0;
  UNDI32Device->NIIProtocol.ImageAddr         = 0;
  UNDI32Device->NIIProtocol.Ipv6Supported     = FALSE;

  UNDI32Device->NIIProtocol.StringId[0]       = 'U';
  UNDI32Device->NIIProtocol.StringId[1]       = 'N';
  UNDI32Device->NIIProtocol.StringId[2]       = 'D';
  UNDI32Device->NIIProtocol.StringId[3]       = 'I';

  UNDI32Device->NIIProtocol_31.Revision       = EFI_NETWORK_INTERFACE_IDENTIFIER_PROTOCOL_REVISION_31;
  UNDI32Device->NIIProtocol_31.Type           = EfiNetworkInterfaceUndi;
  UNDI32Device->NIIProtocol_31.MajorVer       = PXE_ROMID_MAJORVER;
  UNDI32Device->NIIProtocol_31.MinorVer       = PXE_ROMID_MINORVER_31;
  UNDI32Device->NIIProtocol_31.ImageSize      = 0;
  UNDI32Device->NIIProtocol_31.ImageAddr      = 0;
  UNDI32Device->NIIProtocol_31.Ipv6Supported  = FALSE;

  UNDI32Device->NIIProtocol_31.StringId[0]    = 'U';
  UNDI32Device->NIIProtocol_31.StringId[1]    = 'N';
  UNDI32Device->NIIProtocol_31.StringId[2]    = 'D';
  UNDI32Device->NIIProtocol_31.StringId[3]    = 'I';

  UNDI32Device->DeviceHandle                  = NULL;

  //
  // install both the 3.0 and 3.1 NII protocols.
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &UNDI32Device->DeviceHandle,
                  &gEfiNetworkInterfaceIdentifierProtocolGuid_31,
                  &UNDI32Device->NIIProtocol_31,
                  &gEfiNetworkInterfaceIdentifierProtocolGuid,
                  &UNDI32Device->NIIProtocol,
                  &gEfiDevicePathProtocolGuid,
                  UNDI32Device->Undi32DevPath,
                  NULL
                  );

  if (EFI_ERROR (Status)) {
    goto UndiErrorDeleteDevicePath;
  }

  //
  // if the table exists, free it and alloc again, or alloc it directly 
  //
  if (UnidiDataPointer != NULL) {
  	Status = gBS->FreePool(UnidiDataPointer);
  }
  if (EFI_ERROR (Status)) {
    goto UndiErrorDeleteDevicePath;
  }

  Len = (pxe_31->IFcnt * sizeof (NII_ENTRY)) + sizeof (UnidiDataPointer);
  Status = gBS->AllocatePool (EfiRuntimeServicesData, Len, (VOID **) &UnidiDataPointer);

  if (EFI_ERROR (Status)) {
    goto UndiErrorAllocDataPointer;
  }
  
  //
  // Open For Child Device
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIoFncs,
                  This->DriverBindingHandle,
                  UNDI32Device->DeviceHandle,
                  EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                  );

  return EFI_SUCCESS;
UndiErrorAllocDataPointer:
  gBS->UninstallMultipleProtocolInterfaces (
                  &UNDI32Device->DeviceHandle,
                  &gEfiNetworkInterfaceIdentifierProtocolGuid_31,
                  &UNDI32Device->NIIProtocol_31,
                  &gEfiNetworkInterfaceIdentifierProtocolGuid,
                  &UNDI32Device->NIIProtocol,
                  &gEfiDevicePathProtocolGuid,
                  UNDI32Device->Undi32DevPath,
                  NULL
                  );

UndiErrorDeleteDevicePath:
  UNDI32DeviceList[UNDI32Device->NIIProtocol.IfNum] = NULL;
  gBS->FreePool (UNDI32Device->Undi32DevPath);

UndiErrorDeletePxe:
  PxeUpdate (NULL, pxe);
  PxeUpdate (NULL, pxe_31);
  if (TmpPxePointer != NULL) {
    gBS->FreePool (TmpPxePointer);

  }

UndiErrorDeleteDevice:
  gBS->FreePool (UNDI32Device);

UndiError:
  gBS->CloseProtocol (
        Controller,
        &gEfiDevicePathProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

  gBS->CloseProtocol (
        Controller,
        &gEfiPciIoProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

  return Status;
}

EFI_STATUS
EFIAPI
UndiDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  )
/*++

Routine Description:
  Stop this driver on Controller by removing NetworkInterfaceIdentifier protocol and
  closing the DevicePath and PciIo protocols on Controller.

Arguments:
  This              - Protocol instance pointer.
  Controller        - Handle of device to stop driver on.
  NumberOfChildren  - How many children need to be stopped.
  ChildHandleBuffer - Not used.

Returns:
  EFI_SUCCESS       - This driver is removed Controller.
  other             - This driver was not removed from this device.

--*/
// TODO:    EFI_DEVICE_ERROR - add return value to function comment
{
  EFI_STATUS                                Status;
  BOOLEAN                                   AllChildrenStopped;
  UINTN                                     Index;
  UNDI32_DEV                                *UNDI32Device;
  EFI_NETWORK_INTERFACE_IDENTIFIER_PROTOCOL *NIIProtocol;
  EFI_PCI_IO_PROTOCOL                       *PciIo;

  //
  // Complete all outstanding transactions to Controller.
  // Don't allow any new transaction to Controller to be started.
  //
  if (NumberOfChildren == 0) {

    //
    // Close the bus driver
    //
    Status = gBS->CloseProtocol (
                    Controller,
                    &gEfiDevicePathProtocolGuid,
                    This->DriverBindingHandle,
                    Controller
                    );

    Status = gBS->CloseProtocol (
                    Controller,
                    &gEfiPciIoProtocolGuid,
                    This->DriverBindingHandle,
                    Controller
                    );

    return Status;
  }

  AllChildrenStopped = TRUE;

  for (Index = 0; Index < NumberOfChildren; Index++) {

    Status = gBS->OpenProtocol (
                    ChildHandleBuffer[Index],
                    &gEfiNetworkInterfaceIdentifierProtocolGuid,
                    (VOID **) &NIIProtocol,
                    This->DriverBindingHandle,
                    Controller,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (!EFI_ERROR (Status)) {

      UNDI32Device = UNDI_DEV_FROM_THIS (NIIProtocol);

      Status = gBS->CloseProtocol (
                      Controller,
                      &gEfiPciIoProtocolGuid,
                      This->DriverBindingHandle,
                      ChildHandleBuffer[Index]
                      );

      Status = gBS->UninstallMultipleProtocolInterfaces (
                      ChildHandleBuffer[Index],
                      &gEfiDevicePathProtocolGuid,
                      UNDI32Device->Undi32DevPath,
                      &gEfiNetworkInterfaceIdentifierProtocolGuid_31,
                      &UNDI32Device->NIIProtocol_31,
                      &gEfiNetworkInterfaceIdentifierProtocolGuid,
                      &UNDI32Device->NIIProtocol,
                      NULL
                      );

      if (EFI_ERROR (Status)) {
        gBS->OpenProtocol (
              Controller,
              &gEfiPciIoProtocolGuid,
              (VOID **) &PciIo,
              This->DriverBindingHandle,
              ChildHandleBuffer[Index],
              EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
              );
      } else {
        gBS->FreePool (UNDI32Device->Undi32DevPath);
        gBS->FreePool (UNDI32Device);
      }
    }

    if (EFI_ERROR (Status)) {
      AllChildrenStopped = FALSE;
    }
  }

  if (!AllChildrenStopped) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;

}

VOID
TmpDelay (
  IN UINT64 UnqId,
  IN UINTN  MicroSeconds
  )
/*++

Routine Description:

  Use the EFI boot services to produce a pause. This is also the routine which
  gets replaced during RunTime by the O/S in the NIC_DATA_INSTANCE so it can
  do it's own pause.

Arguments:

  UnqId             - Runtime O/S routine might use this, this temp routine does not use it
  
  MicroSeconds      - Determines the length of pause.

Returns:

  none

--*/
{
  gBS->Stall ((UINT32) MicroSeconds);
}

VOID
TmpMemIo (
  IN UINT64 UnqId,
  IN UINT8  ReadWrite,
  IN UINT8  Len,
  IN UINT64 Port,
  IN UINT64 BuffAddr
  )
/*++

Routine Description:

  Use the PCI IO abstraction to issue memory or I/O reads and writes.  This is also the routine which
  gets replaced during RunTime by the O/S in the NIC_DATA_INSTANCE so it can do it's own I/O abstractions.

Arguments:

  UnqId             - Runtime O/S routine may use this field, this temp routine does not.
  
  ReadWrite         - Determine if it is an I/O or Memory Read/Write Operation.
  
  Len               - Determines the width of the data operation.
  
  Port              - What port to Read/Write from.
  
  BuffAddr          - Address to read to or write from.

Returns:

  none

--*/
{
  EFI_PCI_IO_PROTOCOL_WIDTH Width;
  NIC_DATA_INSTANCE         *AdapterInfo;

  Width       = 0;
  AdapterInfo = (NIC_DATA_INSTANCE *) (UINTN) UnqId;
  switch (Len) {
  case 2:
    Width = 1;
    break;

  case 4:
    Width = 2;
    break;

  case 8:
    Width = 3;
    break;
  }

  switch (ReadWrite) {
  case PXE_IO_READ:
    AdapterInfo->Io_Function->Io.Read (
                                  AdapterInfo->Io_Function,
                                  Width,
                                  1,
                                  Port,
                                  1,
                                  (VOID *) (UINTN) (BuffAddr)
                                  );
    break;

  case PXE_IO_WRITE:
    AdapterInfo->Io_Function->Io.Write (
                                  AdapterInfo->Io_Function,
                                  Width,
                                  1,
                                  Port,
                                  1,
                                  (VOID *) (UINTN) (BuffAddr)
                                  );
    break;

  case PXE_MEM_READ:
    AdapterInfo->Io_Function->Mem.Read (
                                    AdapterInfo->Io_Function,
                                    Width,
                                    0,
                                    Port,
                                    1,
                                    (VOID *) (UINTN) (BuffAddr)
                                    );
    break;

  case PXE_MEM_WRITE:
    AdapterInfo->Io_Function->Mem.Write (
                                    AdapterInfo->Io_Function,
                                    Width,
                                    0,
                                    Port,
                                    1,
                                    (VOID *) (UINTN) (BuffAddr)
                                    );
    break;
  }

  return ;
}

EFI_STATUS
AppendMac2DevPath (
  IN OUT  EFI_DEVICE_PATH_PROTOCOL **DevPtr,
  IN      EFI_DEVICE_PATH_PROTOCOL *BaseDevPtr,
  IN      NIC_DATA_INSTANCE        *AdapterInfo
  )
/*++

Routine Description:

  Using the NIC data structure information, read the EEPROM to get the MAC address and then allocate space
  for a new devicepath (**DevPtr) which will contain the original device path the NIC was found on (*BaseDevPtr)
  and an added MAC node.

Arguments:

  DevPtr            - Pointer which will point to the newly created device path with the MAC node attached.
  
  BaseDevPtr        - Pointer to the device path which the UNDI device driver is latching on to.
  
  AdapterInfo       - Pointer to the NIC data structure information which the UNDI driver is layering on..

Returns:

  EFI_SUCCESS       - A MAC address was successfully appended to the Base Device Path.
  
  other             - Not enough resources available to create new Device Path node.

--*/
{
  EFI_MAC_ADDRESS           MACAddress;
  PCI_CONFIG_HEADER         *CfgHdr;
  INT32                     Val;
  INT32                     Index;
  INT32                     Index2;
  UINT8                     AddrLen;
  MAC_ADDR_DEVICE_PATH      MacAddrNode;
  EFI_DEVICE_PATH_PROTOCOL  *EndNode;
  UINT8                     *DevicePtr;
  UINT16                    TotalPathLen;
  UINT16                    BasePathLen;
  EFI_STATUS                Status;

  //
  // set the environment ready (similar to UNDI_Start call) so that we can
  // execute the other UNDI_ calls to get the mac address
  // we are using undi 3.1 style
  //
  AdapterInfo->Delay      = TmpDelay;
  AdapterInfo->Virt2Phys  = (VOID *) 0;
  AdapterInfo->Block      = (VOID *) 0;
  AdapterInfo->Map_Mem    = (VOID *) 0;
  AdapterInfo->UnMap_Mem  = (VOID *) 0;
  AdapterInfo->Sync_Mem   = (VOID *) 0;
  AdapterInfo->Mem_Io     = TmpMemIo;
  //
  // these tmp call-backs follow 3.1 undi style
  // i.e. they have the unique_id parameter.
  //
  AdapterInfo->VersionFlag  = 0x31;
  AdapterInfo->Unique_ID    = (UINT64) (UINTN) AdapterInfo;

  //
  // undi init portion
  //
  CfgHdr              = (PCI_CONFIG_HEADER *) &(AdapterInfo->Config[0]);
  AdapterInfo->ioaddr = 0;
  AdapterInfo->RevID  = CfgHdr->RevID;

  AddrLen             = E100bGetEepromAddrLen (AdapterInfo);

  for (Index = 0, Index2 = 0; Index < 3; Index++) {
    Val                       = E100bReadEeprom (AdapterInfo, Index, AddrLen);
    MACAddress.Addr[Index2++] = (UINT8) Val;
    MACAddress.Addr[Index2++] = (UINT8) (Val >> 8);
  }

  SetMem (MACAddress.Addr + Index2, sizeof (EFI_MAC_ADDRESS) - Index2, 0);
  //for (; Index2 < sizeof (EFI_MAC_ADDRESS); Index2++) {
  //  MACAddress.Addr[Index2] = 0;
  //}
  //
  // stop undi
  //
  AdapterInfo->Delay  = (VOID *) 0;
  AdapterInfo->Mem_Io = (VOID *) 0;

  //
  // fill the mac address node first
  //
  ZeroMem ((CHAR8 *) &MacAddrNode, sizeof MacAddrNode);
  CopyMem (
    (CHAR8 *) &MacAddrNode.MacAddress,
    (CHAR8 *) &MACAddress,
    sizeof (EFI_MAC_ADDRESS)
    );

  MacAddrNode.Header.Type       = MESSAGING_DEVICE_PATH;
  MacAddrNode.Header.SubType    = MSG_MAC_ADDR_DP;
  MacAddrNode.Header.Length[0]  = sizeof (MacAddrNode);
  MacAddrNode.Header.Length[1]  = 0;

  //
  // find the size of the base dev path.
  //
  EndNode = BaseDevPtr;

  while (!IsDevicePathEnd (EndNode)) {
    EndNode = NextDevicePathNode (EndNode);
  }

  BasePathLen = (UINT16) ((UINTN) (EndNode) - (UINTN) (BaseDevPtr));

  //
  // create space for full dev path
  //
  TotalPathLen = (UINT16) (BasePathLen + sizeof (MacAddrNode) + sizeof (EFI_DEVICE_PATH_PROTOCOL));

  Status = gBS->AllocatePool (
                  EfiRuntimeServicesData,
                  TotalPathLen,
                  (VOID **) &DevicePtr
                  );

  if (Status != EFI_SUCCESS) {
    return Status;
  }
  //
  // copy the base path, mac addr and end_dev_path nodes
  //
  *DevPtr = (EFI_DEVICE_PATH_PROTOCOL *) DevicePtr;
  CopyMem (DevicePtr, (CHAR8 *) BaseDevPtr, BasePathLen);
  DevicePtr += BasePathLen;
  CopyMem (DevicePtr, (CHAR8 *) &MacAddrNode, sizeof (MacAddrNode));
  DevicePtr += sizeof (MacAddrNode);
  CopyMem (DevicePtr, (CHAR8 *) EndNode, sizeof (EFI_DEVICE_PATH_PROTOCOL));

  return EFI_SUCCESS;
}

EFI_STATUS
InstallConfigTable (
  IN VOID
  )
/*++

Routine Description:

  Install a GUID/Pointer pair into the system's configuration table.

Arguments:

  none

Returns:

  EFI_SUCCESS       - Install a GUID/Pointer pair into the system's configuration table.
  
  other             - Did not successfully install the GUID/Pointer pair into the configuration table.

--*/
// TODO:    VOID - add argument and description to function comment
{
  EFI_STATUS              Status;
  EFI_CONFIGURATION_TABLE *CfgPtr;
  NII_TABLE               *TmpData;
  UINT16                  Index;
  NII_TABLE               *UndiData;

  if (pxe_31 == NULL) {
    return EFI_SUCCESS;
  }

  if(UnidiDataPointer == NULL) { 
  	return EFI_SUCCESS;
  }
  
  UndiData = (NII_TABLE *)UnidiDataPointer;  
  
  UndiData->NumEntries  = pxe_31->IFcnt;
  UndiData->NextLink    = NULL;

  for (Index = 0; Index < pxe_31->IFcnt; Index++) {
    UndiData->NiiEntry[Index].InterfacePointer  = &UNDI32DeviceList[Index]->NIIProtocol_31;
    UndiData->NiiEntry[Index].DevicePathPointer = UNDI32DeviceList[Index]->Undi32DevPath;
  }

  //
  // see if there is an entry in the config table already
  //
  CfgPtr = gST->ConfigurationTable;

  for (Index = 0; Index < gST->NumberOfTableEntries; Index++) {
    Status = CompareGuid (
              &CfgPtr->VendorGuid,
              &gEfiNetworkInterfaceIdentifierProtocolGuid_31
              );
    if (Status != EFI_SUCCESS) {
      break;
    }

    CfgPtr++;
  }

  if (Index < gST->NumberOfTableEntries) {
    TmpData = (NII_TABLE *) CfgPtr->VendorTable;

    //
    // go to the last link
    //
    while (TmpData->NextLink != NULL) {
      TmpData = TmpData->NextLink;
    }

    TmpData->NextLink = UndiData;

    //
    // 1st one in chain
    //
    UndiData = (NII_TABLE *) CfgPtr->VendorTable;
  }

  //
  // create an entry in the configuration table for our GUID
  //
  Status = gBS->InstallConfigurationTable (
                  &gEfiNetworkInterfaceIdentifierProtocolGuid_31,
                  UndiData
                  );
  return Status;
}
