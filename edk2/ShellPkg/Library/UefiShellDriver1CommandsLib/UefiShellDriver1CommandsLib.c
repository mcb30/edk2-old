/** @file
  Main file for NULL named library for level 1 shell command functions.

  Copyright (c) 2010, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "UefiShellDriver1CommandsLib.h"

STATIC CONST CHAR16 mFileName[] = L"Driver1Commands";
EFI_HANDLE gShellDriver1HiiHandle = NULL;
CONST EFI_GUID gShellDriver1HiiGuid = \
  { \
  0xaf0b742, 0x63ec, 0x45bd, {0x8d, 0xb6, 0x71, 0xad, 0x7f, 0x2f, 0xe8, 0xe8} \
  };


CONST CHAR16*
EFIAPI
ShellCommandGetManFileNameDriver1 (
  VOID
  ){
  return (mFileName);
}

/**
  Constructor for the Shell Driver1 Commands library.

  @param ImageHandle    the image handle of the process
  @param SystemTable    the EFI System Table pointer

  @retval EFI_SUCCESS        the shell command handlers were installed sucessfully
  @retval EFI_UNSUPPORTED    the shell level required was not found.
**/
EFI_STATUS
EFIAPI
UefiShellDriver1CommandsLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  ) {
  //
  // check or bit of the profiles mask
  //
  if (PcdGet8(PcdShellProfileMask) && BIT0 == 0) {
    return (EFI_UNSUPPORTED);
  }

  //
  // install our shell command handlers that are always installed
  //
  ShellCommandRegisterCommandName(L"connect",    ShellCommandRunConnect               , ShellCommandGetManFileNameDriver1, 0, L"Driver1", TRUE);
  ShellCommandRegisterCommandName(L"devices",    ShellCommandRunDevices               , ShellCommandGetManFileNameDriver1, 0, L"Driver1", TRUE);
  ShellCommandRegisterCommandName(L"openinfo",   ShellCommandRunOpenInfo              , ShellCommandGetManFileNameDriver1, 0, L"Driver1", TRUE);
/*
  ShellCommandRegisterCommandName(L"devtree",    ShellCommandRunDevTree               , ShellCommandGetManFileNameDriver1, 0, L"Driver1", TRUE);
  ShellCommandRegisterCommandName(L"dh",         ShellCommandRunDH                    , ShellCommandGetManFileNameDriver1, 0, L"Driver1", TRUE);
  ShellCommandRegisterCommandName(L"disconnect", ShellCommandRunDisconnect            , ShellCommandGetManFileNameDriver1, 0, L"Driver1", TRUE);
  ShellCommandRegisterCommandName(L"drivers",    ShellCommandRunDrivers               , ShellCommandGetManFileNameDriver1, 0, L"Driver1", TRUE);
  ShellCommandRegisterCommandName(L"drvcfg",     ShellCommandRunDrvCfg                , ShellCommandGetManFileNameDriver1, 0, L"Driver1", TRUE);
  ShellCommandRegisterCommandName(L"drvdiag",    ShellCommandRunDrvDiag               , ShellCommandGetManFileNameDriver1, 0, L"Driver1", TRUE);
  ShellCommandRegisterCommandName(L"reconnect",  ShellCommandRunReconnect             , ShellCommandGetManFileNameDriver1, 0, L"Driver1", TRUE);
  ShellCommandRegisterCommandName(L"unload",     ShellCommandRunUnload                , ShellCommandGetManFileNameDriver1, 0, L"Driver1", TRUE);
*/

  //
  // install the HII stuff.
  //
  gShellDriver1HiiHandle = HiiAddPackages (&gShellDriver1HiiGuid, gImageHandle, UefiShellDriver1CommandsLibStrings, NULL);
  if (gShellDriver1HiiHandle == NULL) {
    return (EFI_DEVICE_ERROR);
  }

  return (EFI_SUCCESS);
}

/**
  Destructory for the library.  free any resources.
**/
EFI_STATUS
EFIAPI
UefiShellDriver1CommandsLibDestructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  if (gShellDriver1HiiHandle != NULL) {
    HiiRemovePackages(gShellDriver1HiiHandle);
  }
  return (EFI_SUCCESS);
}

EFI_HANDLE*
EFIAPI
GetHandleListByPotocol (
  IN CONST EFI_GUID *ProtocolGuid
  ){
  EFI_HANDLE          *HandleList;
  UINTN               Size;
  EFI_STATUS          Status;

  Size = 0;
  HandleList = NULL;

  //
  // We cannot use LocateHandleBuffer since we need that NULL item on the ends of the list!
  //
  if (ProtocolGuid == NULL) {
    Status = gBS->LocateHandle(AllHandles, NULL, NULL, &Size, HandleList);
    if (Status == EFI_BUFFER_TOO_SMALL) {
      HandleList = AllocatePool(Size + sizeof(EFI_HANDLE));
      Status = gBS->LocateHandle(AllHandles, NULL, NULL, &Size, HandleList);
      HandleList[Size/sizeof(EFI_HANDLE)] = NULL;
    }
  } else {
    Status = gBS->LocateHandle(ByProtocol, (EFI_GUID*)ProtocolGuid, NULL, &Size, HandleList);
    if (Status == EFI_BUFFER_TOO_SMALL) {
      HandleList = AllocatePool(Size + sizeof(EFI_HANDLE));
      Status = gBS->LocateHandle(ByProtocol, (EFI_GUID*)ProtocolGuid, NULL, &Size, HandleList);
      HandleList[Size/sizeof(EFI_HANDLE)] = NULL;
    }
  }
  if (EFI_ERROR(Status)) {
    if (HandleList != NULL) {
      FreePool(HandleList);
    }
    return (NULL);
  }
  return (HandleList);
}

