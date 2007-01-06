/*++

Copyright (c) 2004 - 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

 Reset.c

Abstract:

  Reset Architectural Protocol as defined in Tiano under NT Emulation

--*/

EFI_STATUS
EFIAPI
InitializeUnixReset (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );

STATIC
VOID
EFIAPI
UnixResetSystem (
  IN EFI_RESET_TYPE   ResetType,
  IN EFI_STATUS       ResetStatus,
  IN UINTN            DataSize,
  IN CHAR16           *ResetData OPTIONAL
  );

EFI_STATUS
EFIAPI
InitializeUnixReset (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/*++

Routine Description:


Arguments:

  ImageHandle of the loaded driver
  Pointer to the System Table

Returns:

  Status
--*/
// TODO:    SystemTable - add argument and description to function comment
{
  EFI_STATUS  Status;
  EFI_HANDLE  Handle;

  SystemTable->RuntimeServices->ResetSystem = UnixResetSystem;

  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEfiResetArchProtocolGuid,
                  NULL,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

STATIC
VOID
EFIAPI
UnixResetSystem (
  IN EFI_RESET_TYPE   ResetType,
  IN EFI_STATUS       ResetStatus,
  IN UINTN            DataSize,
  IN CHAR16           *ResetData OPTIONAL
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ResetType   - TODO: add argument description
  ResetStatus - TODO: add argument description
  DataSize    - TODO: add argument description
  ResetData   - TODO: add argument description

Returns:

  EFI_SUCCESS - TODO: Add description for return value

--*/
{
  //
  // BUGBUG Need to kill all console windows later
  //
  //
  // Discard ResetType, always return 0 as exit code
  //
  gUnix->Exit (0);

  //
  // Should never go here
  //
  while (1)
    ;
}
