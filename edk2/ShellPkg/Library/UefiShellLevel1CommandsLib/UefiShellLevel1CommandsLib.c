/** @file
  Main file for NULL named library for level 1 shell command functions.

  Copyright (c) 2009 - 2010, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "UefiShellLevel1CommandsLib.h"

STATIC CONST CHAR16 mFileName[] = L"ShellCommands";
EFI_HANDLE gShellLevel1HiiHandle = NULL;
CONST EFI_GUID gShellLevel1HiiGuid = \
  { \
    0xdec5daa4, 0x6781, 0x4820, { 0x9c, 0x63, 0xa7, 0xb0, 0xe4, 0xf1, 0xdb, 0x31 }
  };


CONST CHAR16*
EFIAPI
ShellCommandGetManFileNameLevel1 (
  VOID
  )
{
  return (mFileName);
}

/**
  Constructor for the Shell Level 1 Commands library.

  Install the handlers for level 1 UEFI Shell 2.0 commands.

  @param ImageHandle    the image handle of the process
  @param SystemTable    the EFI System Table pointer

  @retval EFI_SUCCESS        the shell command handlers were installed sucessfully
  @retval EFI_UNSUPPORTED    the shell level required was not found.
**/
EFI_STATUS
EFIAPI
ShellLevel1CommandsLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  //
  // if shell level is less than 2 do nothing
  //
  if (PcdGet8(PcdShellSupportLevel) < 1) {
    return (EFI_UNSUPPORTED);
  }

  gShellLevel1HiiHandle = HiiAddPackages (&gShellLevel1HiiGuid, gImageHandle, UefiShellLevel1CommandsLibStrings, NULL);
  if (gShellLevel1HiiHandle == NULL) {
    return (EFI_DEVICE_ERROR);
  }

  //
  // install our shell command handlers that are always installed
  //
  ShellCommandRegisterCommandName(L"for",    ShellCommandRunFor     , ShellCommandGetManFileNameLevel1, 1, L"", FALSE, gShellLevel1HiiHandle, (EFI_STRING_ID)(PcdGet8(PcdShellSupportLevel) < 3 ? 0 : STRING_TOKEN(STR_GET_HELP_FOR)   ));
  ShellCommandRegisterCommandName(L"goto",   ShellCommandRunGoto    , ShellCommandGetManFileNameLevel1, 1, L"", FALSE, gShellLevel1HiiHandle, (EFI_STRING_ID)(PcdGet8(PcdShellSupportLevel) < 3 ? 0 : STRING_TOKEN(STR_GET_HELP_GOTO)  ));
  ShellCommandRegisterCommandName(L"if",     ShellCommandRunIf      , ShellCommandGetManFileNameLevel1, 1, L"", FALSE, gShellLevel1HiiHandle, (EFI_STRING_ID)(PcdGet8(PcdShellSupportLevel) < 3 ? 0 : STRING_TOKEN(STR_GET_HELP_IF)    ));
  ShellCommandRegisterCommandName(L"shift",  ShellCommandRunShift   , ShellCommandGetManFileNameLevel1, 1, L"", FALSE, gShellLevel1HiiHandle, (EFI_STRING_ID)(PcdGet8(PcdShellSupportLevel) < 3 ? 0 : STRING_TOKEN(STR_GET_HELP_SHIFT) ));
  ShellCommandRegisterCommandName(L"exit",   ShellCommandRunExit    , ShellCommandGetManFileNameLevel1, 1, L"", TRUE , gShellLevel1HiiHandle, (EFI_STRING_ID)(PcdGet8(PcdShellSupportLevel) < 3 ? 0 : STRING_TOKEN(STR_GET_HELP_EXIT)  ));
  ShellCommandRegisterCommandName(L"else",   ShellCommandRunElse    , ShellCommandGetManFileNameLevel1, 1, L"", FALSE, gShellLevel1HiiHandle, (EFI_STRING_ID)(PcdGet8(PcdShellSupportLevel) < 3 ? 0 : STRING_TOKEN(STR_GET_HELP_ELSE)  ));
  ShellCommandRegisterCommandName(L"endif",  ShellCommandRunEndIf   , ShellCommandGetManFileNameLevel1, 1, L"", FALSE, gShellLevel1HiiHandle, (EFI_STRING_ID)(PcdGet8(PcdShellSupportLevel) < 3 ? 0 : STRING_TOKEN(STR_GET_HELP_ENDIF) ));
  ShellCommandRegisterCommandName(L"endfor", ShellCommandRunEndFor  , ShellCommandGetManFileNameLevel1, 1, L"", FALSE, gShellLevel1HiiHandle, (EFI_STRING_ID)(PcdGet8(PcdShellSupportLevel) < 3 ? 0 : STRING_TOKEN(STR_GET_HELP_ENDFOR)));

  return (EFI_SUCCESS);
}

/**
  Destructor for the library.  free any resources.
**/
EFI_STATUS
EFIAPI
ShellLevel1CommandsLibDestructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  if (gShellLevel1HiiHandle != NULL) {
    HiiRemovePackages(gShellLevel1HiiHandle);
  }
  return (EFI_SUCCESS);
}

BOOLEAN
EFIAPI
TestNodeForMove (
  IN CONST LIST_MANIP_FUNC      Function,
  IN CONST CHAR16               *DecrementerTag,
  IN CONST CHAR16               *IncrementerTag,
  IN CONST CHAR16               *Label OPTIONAL,
  IN SCRIPT_FILE                *ScriptFile,
  IN CONST BOOLEAN              MovePast,
  IN CONST BOOLEAN              FindOnly,
  IN CONST SCRIPT_COMMAND_LIST  *CommandNode,
  IN UINTN                      *TargetCount
  )
{
  BOOLEAN             Found;
  CHAR16              *CommandName;
  CHAR16              *CommandNameWalker;
  CHAR16              *TempLocation;

  Found = FALSE;

  //
  // get just the first part of the command line...
  //
  CommandName   = NULL;
  CommandName   = StrnCatGrow(&CommandName, NULL, CommandNode->Cl, 0);
  CommandNameWalker = CommandName;
  while(CommandNameWalker[0] == L' ') {
    CommandNameWalker++;
  }
  TempLocation  = StrStr(CommandNameWalker, L" ");

  if (TempLocation != NULL) {
    *TempLocation = CHAR_NULL;
  }

  //
  // did we find a nested item ?
  //
  if (gUnicodeCollation->StriColl(
      gUnicodeCollation,
      (CHAR16*)CommandNameWalker,
      (CHAR16*)IncrementerTag) == 0) {
    (*TargetCount)++;
  } else if (gUnicodeCollation->StriColl(
      gUnicodeCollation,
      (CHAR16*)CommandNameWalker,
      (CHAR16*)DecrementerTag) == 0) {
    if (*TargetCount > 0) {
      (*TargetCount)--;
    }
  }

  //
  // did we find the matching one...
  //
  if (Label == NULL) {
    if (*TargetCount == 0) {
      Found = TRUE;
      if (!FindOnly) {
        if (MovePast) {
          ScriptFile->CurrentCommand = (SCRIPT_COMMAND_LIST *)(*Function)(&ScriptFile->CommandList, &CommandNode->Link);
        } else {
          ScriptFile->CurrentCommand = (SCRIPT_COMMAND_LIST *)CommandNode;
        }
      }
    }
  } else {
    if (gUnicodeCollation->StriColl(
      gUnicodeCollation,
      (CHAR16*)CommandNameWalker,
      (CHAR16*)Label) == 0
      && (*TargetCount) == 0) {
      Found = TRUE;
      if (!FindOnly) {
        //
        // we found the target label without loops
        //
        if (MovePast) {
          ScriptFile->CurrentCommand = (SCRIPT_COMMAND_LIST *)(*Function)(&ScriptFile->CommandList, &CommandNode->Link);
        } else {
          ScriptFile->CurrentCommand = (SCRIPT_COMMAND_LIST *)CommandNode;
        }
      }
    }
  }

  //
  // Free the memory for this loop...
  //
  FreePool(CommandName);
  return (Found);
}

BOOLEAN
EFIAPI
MoveToTag (
  IN CONST LIST_MANIP_FUNC      Function,
  IN CONST CHAR16               *DecrementerTag,
  IN CONST CHAR16               *IncrementerTag,
  IN CONST CHAR16               *Label OPTIONAL,
  IN SCRIPT_FILE                *ScriptFile,
  IN CONST BOOLEAN              MovePast,
  IN CONST BOOLEAN              FindOnly,
  IN CONST BOOLEAN              WrapAroundScript
  )
{
  SCRIPT_COMMAND_LIST *CommandNode;
  BOOLEAN             Found;
  UINTN               TargetCount;

  if (Label == NULL) {
    TargetCount       = 1;
  } else {
    TargetCount       = 0;
  }

  if (ScriptFile == NULL) {
    return FALSE;
  }

  for (CommandNode = (SCRIPT_COMMAND_LIST *)(*Function)(&ScriptFile->CommandList, &ScriptFile->CurrentCommand->Link), Found = FALSE
    ;  !IsNull(&ScriptFile->CommandList, &CommandNode->Link)&& !Found
    ;  CommandNode = (SCRIPT_COMMAND_LIST *)(*Function)(&ScriptFile->CommandList, &CommandNode->Link)
   ){
    Found = TestNodeForMove(
      Function,
      DecrementerTag,
      IncrementerTag,
      Label,
      ScriptFile,
      MovePast,
      FindOnly,
      CommandNode,
      &TargetCount);
  }

  if (WrapAroundScript && !Found) {
    for (CommandNode = (SCRIPT_COMMAND_LIST *)GetFirstNode(&ScriptFile->CommandList), Found = FALSE
      ;  CommandNode != ScriptFile->CurrentCommand && !Found
      ;  CommandNode = (SCRIPT_COMMAND_LIST *)(*Function)(&ScriptFile->CommandList, &CommandNode->Link)
     ){
      Found = TestNodeForMove(
        Function,
        DecrementerTag,
        IncrementerTag,
        Label,
        ScriptFile,
        MovePast,
        FindOnly,
        CommandNode,
        &TargetCount);
    }
  }
  return (Found);
}
