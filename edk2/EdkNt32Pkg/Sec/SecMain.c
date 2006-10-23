/*++

Copyright (c) 2006, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  SecMain.c

Abstract:
  WinNt emulator of SEC phase. It's really a Win32 application, but this is
  Ok since all the other modules for NT32 are NOT Win32 applications.

  This program processes Windows environment variables and figures out
  what the memory layout will be, how may FD's will be loaded and also
  what the boot mode is.

  The SEC registers a set of services with the SEC core. gPrivateDispatchTable
  is a list of PPI's produced by the SEC that are availble for usage in PEI.

  This code produces 128 K of temporary memory for the PEI stack by opening a
  Windows file and mapping it directly to memory addresses.

  The system.cmd script is used to set windows environment variables that drive
  the configuration opitons of the SEC.

--*/

#include "SecMain.h"
#pragma warning(disable : 4996)

//
// Globals
//
EFI_PEI_PE_COFF_LOADER_PROTOCOL_INSTANCE  mPeiEfiPeiPeCoffLoaderInstance = {
  {
    SecNt32PeCoffGetImageInfo,
    SecNt32PeCoffLoadImage,
    SecNt32PeCoffRelocateImage,
    SecNt32PeCoffUnloadimage
  },
  NULL
};



EFI_PEI_PE_COFF_LOADER_PROTOCOL           *gPeiEfiPeiPeCoffLoader = &mPeiEfiPeiPeCoffLoaderInstance.PeCoff;

NT_PEI_LOAD_FILE_PPI                      mSecNtLoadFilePpi     = { SecWinNtPeiLoadFile };

PEI_NT_AUTOSCAN_PPI                       mSecNtAutoScanPpi     = { SecWinNtPeiAutoScan };

PEI_NT_THUNK_PPI                          mSecWinNtThunkPpi     = { SecWinNtWinNtThunkAddress };

EFI_PEI_PROGRESS_CODE_PPI                 mSecStatusCodePpi     = { SecPeiReportStatusCode };

NT_FWH_PPI                                mSecFwhInformationPpi = { SecWinNtFdAddress };


EFI_PEI_PPI_DESCRIPTOR  gPrivateDispatchTable[] = {
  {
    EFI_PEI_PPI_DESCRIPTOR_PPI,
    &gEfiPeiPeCoffLoaderGuid,
    NULL
  },
  {
    EFI_PEI_PPI_DESCRIPTOR_PPI,
    &gNtPeiLoadFilePpiGuid,
    &mSecNtLoadFilePpi
  },
  {
    EFI_PEI_PPI_DESCRIPTOR_PPI,
    &gPeiNtAutoScanPpiGuid,
    &mSecNtAutoScanPpi
  },
  {
    EFI_PEI_PPI_DESCRIPTOR_PPI,
    &gPeiNtThunkPpiGuid,
    &mSecWinNtThunkPpi
  },
  {
    EFI_PEI_PPI_DESCRIPTOR_PPI,
    &gEfiPeiStatusCodePpiGuid,
    &mSecStatusCodePpi
  },
  {
    EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
    &gNtFwhPpiGuid,
    &mSecFwhInformationPpi
  }
};


//
// Default information about where the FD is located.
//  This array gets filled in with information from EFI_FIRMWARE_VOLUMES
//  EFI_FIRMWARE_VOLUMES is a Windows environment variable set by system.cmd.
//  The number of array elements is allocated base on parsing
//  EFI_FIRMWARE_VOLUMES and the memory is never freed.
//
UINTN                                     gFdInfoCount = 0;
NT_FD_INFO                                *gFdInfo;

//
// Array that supports seperate memory rantes.
//  The memory ranges are set in system.cmd via the EFI_MEMORY_SIZE variable.
//  The number of array elements is allocated base on parsing
//  EFI_MEMORY_SIZE and the memory is never freed.
//
UINTN                                     gSystemMemoryCount = 0;
NT_SYSTEM_MEMORY                          *gSystemMemory;


UINTN                   mPdbNameModHandleArraySize = 0;
PDB_NAME_TO_MOD_HANDLE  *mPdbNameModHandleArray = NULL;




INTN
EFIAPI
main (
  IN  INTN  Argc,
  IN  CHAR8 **Argv,
  IN  CHAR8 **Envp
  )
/*++

Routine Description:
  Main entry point to SEC for WinNt. This is a Windows program

Arguments:
  Argc - Number of command line arguments
  Argv - Array of command line argument strings
  Envp - Array of environmemt variable strings

Returns:
  0 - Normal exit
  1 - Abnormal exit

--*/
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  InitialStackMemory;
  UINT64                InitialStackMemorySize;
  UINTN                 Index;
  UINTN                 Index1;
  UINTN                 Index2;
  UINTN                 PeiIndex;
  CHAR16                *FileName;
  CHAR16                *FileNamePtr;
  BOOLEAN               Done;
  VOID                  *PeiCoreFile;
  CHAR16                *MemorySizeStr;
  CHAR16                *FirmwareVolumesStr;

  MemorySizeStr      = (CHAR16 *)PcdGetPtr (PcdWinNtMemorySizeForSecMain);
  FirmwareVolumesStr = (CHAR16 *)PcdGetPtr (PcdWinNtFirmwareVolume);

  printf ("\nEDK SEC Main NT Emulation Environment from www.TianoCore.org\n");

  //
  // Make some Windows calls to Set the process to the highest priority in the
  //  idle class. We need this to have good performance.
  //
  SetPriorityClass (GetCurrentProcess (), IDLE_PRIORITY_CLASS);
  SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_HIGHEST);

  //
  // Allocate space for gSystemMemory Array
  //
  gSystemMemoryCount  = CountSeperatorsInString (MemorySizeStr, '!') + 1;
  gSystemMemory       = calloc (gSystemMemoryCount, sizeof (NT_SYSTEM_MEMORY));
  if (gSystemMemory == NULL) {
    printf ("ERROR : Can not allocate memory for %s.  Exiting.\n", MemorySizeStr);
    exit (1);
  }
  //
  // Allocate space for gSystemMemory Array
  //
  gFdInfoCount  = CountSeperatorsInString (FirmwareVolumesStr, '!') + 1;
  gFdInfo       = calloc (gFdInfoCount, sizeof (NT_FD_INFO));
  if (gFdInfo == NULL) {
    printf ("ERROR : Can not allocate memory for %s.  Exiting.\n", FirmwareVolumesStr);
    exit (1);
  }
  //
  // Setup Boot Mode. If BootModeStr == "" then BootMode = 0 (BOOT_WITH_FULL_CONFIGURATION)
  //
  printf ("  BootMode 0x%02x\n", FixedPcdGet32 (PcdWinNtBootMode));

  //
  // Open up a 128K file to emulate temp memory for PEI.
  //  on a real platform this would be SRAM, or using the cache as RAM.
  //  Set InitialStackMemory to zero so WinNtOpenFile will allocate a new mapping
  //
  InitialStackMemory      = 0;
  InitialStackMemorySize  = 0x20000;
  Status = WinNtOpenFile (
            L"SecStack",
            (UINT32) InitialStackMemorySize,
            OPEN_ALWAYS,
            &InitialStackMemory,
            &InitialStackMemorySize
            );
  if (EFI_ERROR (Status)) {
    printf ("ERROR : Can not open SecStack Exiting\n");
    exit (1);
  }

  printf ("  SEC passing in %d bytes of temp RAM to PEI\n", InitialStackMemorySize);

  //
  // Open All the firmware volumes and remember the info in the gFdInfo global
  //
  FileNamePtr = (CHAR16 *)malloc (StrLen ((CHAR16 *)FirmwareVolumesStr) * sizeof(CHAR16));
  if (FileNamePtr == NULL) {
    printf ("ERROR : Can not allocate memory for firmware volume string\n");
    exit (1);
  }

  StrCpy (FileNamePtr, (CHAR16*)FirmwareVolumesStr);

  for (Done = FALSE, Index = 0, PeiIndex = 0, PeiCoreFile = NULL; !Done; Index++) {
    FileName = FileNamePtr;
    for (Index1 = 0; (FileNamePtr[Index1] != '!') && (FileNamePtr[Index1] != 0); Index1++)
      ;
    if (FileNamePtr[Index1] == 0) {
      Done = TRUE;
    } else {
      FileNamePtr[Index1]  = '\0';
      FileNamePtr = FileNamePtr + Index1 + 1;
    }

    //
    // Open the FD and remmeber where it got mapped into our processes address space
    //
    Status = WinNtOpenFile (
              FileName,
              0,
              OPEN_EXISTING,
              &gFdInfo[Index].Address,
              &gFdInfo[Index].Size
              );
    if (EFI_ERROR (Status)) {
      printf ("ERROR : Can not open Firmware Device File %S (%r).  Exiting.\n", FileName, Status);
      exit (1);
    }

    printf ("  FD loaded from");
    //
    // printf can't print filenames directly as the \ gets interperted as an
    //  escape character.
    //
    for (Index2 = 0; FileName[Index2] != '\0'; Index2++) {
      printf ("%c", FileName[Index2]);
    }

    if (PeiCoreFile == NULL) {
      //
      // Assume the beginning of the FD is an FV and look for the PEI Core.
      // Load the first one we find.
      //
      Status = SecFfsFindPeiCore ((EFI_FIRMWARE_VOLUME_HEADER *) (UINTN) gFdInfo[Index].Address, &PeiCoreFile);
      if (!EFI_ERROR (Status)) {
        PeiIndex = Index;
        printf (" contains SEC Core");
      }
    }

    printf ("\n");
  }
  //
  // Calculate memory regions and store the information in the gSystemMemory
  //  global for later use. The autosizing code will use this data to
  //  map this memory into the SEC process memory space.
  //
  for (Index = 0, Done = FALSE; !Done; Index++) {
    //
    // Save the size of the memory and make a Unicode filename SystemMemory00, ...
    //
    gSystemMemory[Index].Size = _wtoi (MemorySizeStr) * 0x100000;
    _snwprintf (gSystemMemory[Index].FileName, NT_SYSTEM_MEMORY_FILENAME_SIZE, L"SystemMemory%02d", Index);

    //
    // Find the next region
    //
    for (Index1 = 0; MemorySizeStr[Index1] != '!' && MemorySizeStr[Index1] != 0; Index1++)
      ;
    if (MemorySizeStr[Index1] == 0) {
      Done = TRUE;
    }

    MemorySizeStr = MemorySizeStr + Index1 + 1;
  }

  printf ("\n");

  //
  // Hand off to PEI Core
  //
  SecLoadFromCore ((UINTN) InitialStackMemory, (UINTN) InitialStackMemorySize, (UINTN) gFdInfo[0].Address, PeiCoreFile);

  //
  // If we get here, then the PEI Core returned. This is an error as PEI should
  //  always hand off to DXE.
  //
  printf ("ERROR : PEI Core returned\n");
  exit (1);
}

EFI_STATUS
WinNtOpenFile (
  IN  CHAR16                    *FileName,
  IN  UINT32                    MapSize,
  IN  DWORD                     CreationDisposition,
  IN OUT  EFI_PHYSICAL_ADDRESS  *BaseAddress,
  OUT UINT64                    *Length
  )
/*++

Routine Description:
  Opens and memory maps a file using WinNt services. If BaseAddress is non zero
  the process will try and allocate the memory starting at BaseAddress.

Arguments:
  FileName            - The name of the file to open and map
  MapSize             - The amount of the file to map in bytes
  CreationDisposition - The flags to pass to CreateFile().  Use to create new files for
                        memory emulation, and exiting files for firmware volume emulation
  BaseAddress         - The base address of the mapped file in the user address space.
                         If passed in as NULL the a new memory region is used.
                         If passed in as non NULL the request memory region is used for
                          the mapping of the file into the process space.
  Length              - The size of the mapped region in bytes

Returns:
  EFI_SUCCESS      - The file was opened and mapped.
  EFI_NOT_FOUND    - FileName was not found in the current directory
  EFI_DEVICE_ERROR - An error occured attempting to map the opened file

--*/
{
  HANDLE  NtFileHandle;
  HANDLE  NtMapHandle;
  VOID    *VirtualAddress;
  UINTN   FileSize;

  //
  // Use Win API to open/create a file
  //
  NtFileHandle = CreateFile (
                  FileName,
                  GENERIC_READ | GENERIC_WRITE,
                  FILE_SHARE_READ,
                  NULL,
                  CreationDisposition,
                  FILE_ATTRIBUTE_NORMAL,
                  NULL
                  );
  if (NtFileHandle == INVALID_HANDLE_VALUE) {
    return EFI_NOT_FOUND;
  }
  //
  // Map the open file into a memory range
  //
  NtMapHandle = CreateFileMapping (
                  NtFileHandle,
                  NULL,
                  PAGE_READWRITE,
                  0,
                  MapSize,
                  NULL
                  );
  if (NtMapHandle == NULL) {
    return EFI_DEVICE_ERROR;
  }
  //
  // Get the virtual address (address in the emulator) of the mapped file
  //
  VirtualAddress = MapViewOfFileEx (
                    NtMapHandle,
                    FILE_MAP_ALL_ACCESS,
                    0,
                    0,
                    MapSize,
                    (LPVOID) (UINTN) *BaseAddress
                    );
  if (VirtualAddress == NULL) {
    return EFI_DEVICE_ERROR;
  }

  if (MapSize == 0) {
    //
    // Seek to the end of the file to figure out the true file size.
    //
    FileSize = SetFilePointer (
                NtFileHandle,
                0,
                NULL,
                FILE_END
                );
    if (FileSize == -1) {
      return EFI_DEVICE_ERROR;
    }

    *Length = (UINT64) FileSize;
  } else {
    *Length = (UINT64) MapSize;
  }

  *BaseAddress = (EFI_PHYSICAL_ADDRESS) (UINTN) VirtualAddress;

  return EFI_SUCCESS;
}

#define BYTES_PER_RECORD  512

/**
  Extracts ASSERT() information from a status code structure.

  Converts the status code specified by CodeType, Value, and Data to the ASSERT()
  arguments specified by Filename, Description, and LineNumber.  If CodeType is 
  an EFI_ERROR_CODE, and CodeType has a severity of EFI_ERROR_UNRECOVERED, and 
  Value has an operation mask of EFI_SW_EC_ILLEGAL_SOFTWARE_STATE, extract 
  Filename, Description, and LineNumber from the optional data area of the 
  status code buffer specified by Data.  The optional data area of Data contains 
  a Null-terminated ASCII string for the FileName, followed by a Null-terminated 
  ASCII string for the Description, followed by a 32-bit LineNumber.  If the 
  ASSERT() information could be extracted from Data, then return TRUE.  
  Otherwise, FALSE is returned.  

  If Data is NULL, then ASSERT().
  If Filename is NULL, then ASSERT().
  If Description is NULL, then ASSERT().
  If LineNumber is NULL, then ASSERT().

  @param  CodeType     The type of status code being converted.
  @param  Value        The status code value being converted.
  @param  Data         Pointer to status code data buffer. 
  @param  Filename     Pointer to the source file name that generated the ASSERT().
  @param  Description  Pointer to the description of the ASSERT().
  @param  LineNumber   Pointer to source line number that generated the ASSERT().

  @retval  TRUE   The status code specified by CodeType, Value, and Data was 
                  converted ASSERT() arguments specified by Filename, Description, 
                  and LineNumber.
  @retval  FALSE  The status code specified by CodeType, Value, and Data could 
                  not be converted to ASSERT() arguments.

**/
STATIC
BOOLEAN
ReportStatusCodeExtractAssertInfo (
  IN EFI_STATUS_CODE_TYPE        CodeType,
  IN EFI_STATUS_CODE_VALUE       Value,  
  IN CONST EFI_STATUS_CODE_DATA  *Data, 
  OUT CHAR8                      **Filename,
  OUT CHAR8                      **Description,
  OUT UINT32                     *LineNumber
  )
{
  EFI_DEBUG_ASSERT_DATA  *AssertData;

  ASSERT (Data        != NULL);
  ASSERT (Filename    != NULL);
  ASSERT (Description != NULL);
  ASSERT (LineNumber  != NULL);

  if (((CodeType & EFI_STATUS_CODE_TYPE_MASK)      == EFI_ERROR_CODE) && 
      ((CodeType & EFI_STATUS_CODE_SEVERITY_MASK)  == EFI_ERROR_UNRECOVERED) &&
      ((Value    & EFI_STATUS_CODE_OPERATION_MASK) == EFI_SW_EC_ILLEGAL_SOFTWARE_STATE)) {
    AssertData   = (EFI_DEBUG_ASSERT_DATA *)(Data + 1);
    *Filename    = (CHAR8 *)(AssertData + 1);
    *Description = *Filename + AsciiStrLen (*Filename) + 1;
    *LineNumber  = AssertData->LineNumber;
    return TRUE;
  }
  return FALSE;
}

EFI_STATUS
EFIAPI
SecPeiReportStatusCode (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_STATUS_CODE_TYPE       CodeType,
  IN EFI_STATUS_CODE_VALUE      Value,
  IN UINT32                     Instance,
  IN EFI_GUID                   * CallerId,
  IN EFI_STATUS_CODE_DATA       * Data OPTIONAL
  )
/*++

Routine Description:

  This routine produces the ReportStatusCode PEI service. It's passed
  up to the PEI Core via a PPI. T

  This code currently uses the NT clib printf. This does not work the same way
  as the EFI Print (), as %t, %g, %s as Unicode are not supported.

Arguments:
  (see EFI_PEI_REPORT_STATUS_CODE)

Returns:
  EFI_SUCCESS - Always return success

--*/
// TODO:    PeiServices - add argument and description to function comment
// TODO:    CodeType - add argument and description to function comment
// TODO:    Value - add argument and description to function comment
// TODO:    Instance - add argument and description to function comment
// TODO:    CallerId - add argument and description to function comment
// TODO:    Data - add argument and description to function comment
{
  CHAR8           *Format;
  EFI_DEBUG_INFO  *DebugInfo;
  VA_LIST         Marker;
  CHAR8           PrintBuffer[BYTES_PER_RECORD * 2];
  CHAR8           *Filename;
  CHAR8           *Description;
  UINT32          LineNumber;

  if ((CodeType & EFI_STATUS_CODE_TYPE_MASK) == EFI_DEBUG_CODE) {
    //
    // This supports DEBUG () marcos
    // Data format
    //  EFI_STATUS_CODE_DATA
    //  EFI_DEBUG_INFO
    //
    // The first 12 * UINT64 bytes of the string are really an
    // arguement stack to support varargs on the Format string.
    //
    if (Data != NULL) {
      DebugInfo = (EFI_DEBUG_INFO *) (Data + 1);
      Marker    = (VA_LIST) (DebugInfo + 1);
      Format    = (CHAR8 *) (((UINT64 *) Marker) + 12);

      AsciiVSPrint (PrintBuffer, BYTES_PER_RECORD, Format, Marker);
      printf (PrintBuffer);
    } else {
      printf ("DEBUG <null>\n");
    }
  }

  if (((CodeType & EFI_STATUS_CODE_TYPE_MASK) == EFI_ERROR_CODE) &&
      ((CodeType & EFI_STATUS_CODE_SEVERITY_MASK) == EFI_ERROR_UNRECOVERED)
      ) {
    if (Data != NULL && ReportStatusCodeExtractAssertInfo (CodeType, Value, Data, &Filename, &Description, &LineNumber)) {
      //
      // Support ASSERT () macro
      //
      printf ("ASSERT %s(%d): %s\n", Filename, LineNumber, Description);
    } else {
      printf ("ASSERT <null>\n");
    }
    CpuBreakpoint ();
  }

  return EFI_SUCCESS;
}


VOID
SecLoadFromCore (
  IN  UINTN   LargestRegion,
  IN  UINTN   LargestRegionSize,
  IN  UINTN   BootFirmwareVolumeBase,
  IN  VOID    *PeiCorePe32File
  )
/*++

Routine Description:
  This is the service to load the PEI Core from the Firmware Volume

Arguments:
  LargestRegion           - Memory to use for PEI.
  LargestRegionSize       - Size of Memory to use for PEI
  BootFirmwareVolumeBase  - Start of the Boot FV
  PeiCorePe32File         - PEI Core PE32

Returns:
  Success means control is transfered and thus we should never return

--*/
{
  EFI_STATUS                  Status;
  EFI_PHYSICAL_ADDRESS        TopOfMemory;
  VOID                        *TopOfStack;
  UINT64                      PeiCoreSize;
  EFI_PHYSICAL_ADDRESS        PeiCoreEntryPoint;
  EFI_PHYSICAL_ADDRESS        PeiImageAddress;
  EFI_PEI_STARTUP_DESCRIPTOR  *PeiStartup;

  //
  // Compute Top Of Memory for Stack and PEI Core Allocations
  //
  TopOfMemory = LargestRegion + ((LargestRegionSize) & (~15));

  //
  // Allocate 128KB for the Stack
  //
  TopOfStack  = (VOID *) (UINTN) (TopOfMemory - sizeof (EFI_PEI_STARTUP_DESCRIPTOR));
  TopOfMemory = TopOfMemory - STACK_SIZE;

  //
  // Patch value in dispatch table values
  //
  gPrivateDispatchTable[0].Ppi = gPeiEfiPeiPeCoffLoader;

  //
  // Bind this information into the SEC hand-off state
  //
  PeiStartup = (EFI_PEI_STARTUP_DESCRIPTOR *) (UINTN) TopOfStack;
  PeiStartup->DispatchTable      = (EFI_PEI_PPI_DESCRIPTOR *) &gPrivateDispatchTable;
  PeiStartup->SizeOfCacheAsRam   = STACK_SIZE;
  PeiStartup->BootFirmwareVolume = BootFirmwareVolumeBase;

  //
  // Load the PEI Core from a Firmware Volume
  //
  Status = SecWinNtPeiLoadFile (
            PeiCorePe32File,
            &PeiImageAddress,
            &PeiCoreSize,
            &PeiCoreEntryPoint
            );
  if (EFI_ERROR (Status)) {
    return ;
  }
  //
  // Transfer control to the PEI Core
  //
  SwitchStack (
    (SWITCH_STACK_ENTRY_POINT) (UINTN) PeiCoreEntryPoint,
    PeiStartup,
    NULL,
    TopOfStack
    );
  //
  // If we get here, then the PEI Core returned.  This is an error
  //
  return ;
}

EFI_STATUS
EFIAPI
SecWinNtPeiAutoScan (
  IN  UINTN                 Index,
  OUT EFI_PHYSICAL_ADDRESS  *MemoryBase,
  OUT UINT64                *MemorySize
  )
/*++

Routine Description:
  This service is called from Index == 0 until it returns EFI_UNSUPPORTED.
  It allows discontiguous memory regions to be supported by the emulator.
  It uses gSystemMemory[] and gSystemMemoryCount that were created by
  parsing the Windows environment variable EFI_MEMORY_SIZE.
  The size comes from the varaible and the address comes from the call to
  WinNtOpenFile.

Arguments:
  Index      - Which memory region to use
  MemoryBase - Return Base address of memory region
  MemorySize - Return size in bytes of the memory region

Returns:
  EFI_SUCCESS - If memory region was mapped
  EFI_UNSUPPORTED - If Index is not supported

--*/
{
  EFI_STATUS  Status;

  if (Index >= gSystemMemoryCount) {
    return EFI_UNSUPPORTED;
  }

  *MemoryBase = 0;
  Status = WinNtOpenFile (
            gSystemMemory[Index].FileName,
            (UINT32) gSystemMemory[Index].Size,
            OPEN_ALWAYS,
            MemoryBase,
            MemorySize
            );

  gSystemMemory[Index].Memory = *MemoryBase;

  return Status;
}

VOID *
EFIAPI
SecWinNtWinNtThunkAddress (
  VOID
  )
/*++

Routine Description:
  Since the SEC is the only Windows program in stack it must export
  an interface to do Win API calls. That's what the WinNtThunk address
  is for. gWinNt is initailized in WinNtThunk.c.

Arguments:
  InterfaceSize - sizeof (EFI_WIN_NT_THUNK_PROTOCOL);
  InterfaceBase - Address of the gWinNt global

Returns:
  EFI_SUCCESS - Data returned

--*/
{
  return gWinNt;
}


EFI_STATUS
EFIAPI
SecWinNtPeiLoadFile (
  IN  VOID                    *Pe32Data,
  IN  EFI_PHYSICAL_ADDRESS    *ImageAddress,
  IN  UINT64                  *ImageSize,
  IN  EFI_PHYSICAL_ADDRESS    *EntryPoint
  )
/*++

Routine Description:
  Loads and relocates a PE/COFF image into memory.

Arguments:
  Pe32Data         - The base address of the PE/COFF file that is to be loaded and relocated
  ImageAddress     - The base address of the relocated PE/COFF image
  ImageSize        - The size of the relocated PE/COFF image
  EntryPoint       - The entry point of the relocated PE/COFF image

Returns:
  EFI_SUCCESS   - The file was loaded and relocated
  EFI_OUT_OF_RESOURCES - There was not enough memory to load and relocate the PE/COFF file

--*/
{
  EFI_STATUS                            Status;
  PE_COFF_LOADER_IMAGE_CONTEXT          ImageContext;

  ZeroMem (&ImageContext, sizeof (ImageContext));
  ImageContext.Handle     = Pe32Data;

  ImageContext.ImageRead  = (PE_COFF_LOADER_READ_FILE) SecImageRead;

  Status                  = gPeiEfiPeiPeCoffLoader->GetImageInfo (gPeiEfiPeiPeCoffLoader, &ImageContext);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Allocate space in NT (not emulator) memory. Extra space is for alignment
  //
  ImageContext.ImageAddress = (EFI_PHYSICAL_ADDRESS) (UINTN) malloc ((UINTN) (ImageContext.ImageSize + (ImageContext.SectionAlignment * 2)));
  if (ImageContext.ImageAddress == 0) {
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // Align buffer on section boundry
  //
  ImageContext.ImageAddress += ImageContext.SectionAlignment;
  ImageContext.ImageAddress &= ~(ImageContext.SectionAlignment - 1);

  Status = gPeiEfiPeiPeCoffLoader->LoadImage (gPeiEfiPeiPeCoffLoader, &ImageContext);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gPeiEfiPeiPeCoffLoader->RelocateImage (gPeiEfiPeiPeCoffLoader, &ImageContext);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // BugBug: Flush Instruction Cache Here when CPU Lib is ready
  //

  *ImageAddress = ImageContext.ImageAddress;
  *ImageSize    = ImageContext.ImageSize;
  *EntryPoint   = ImageContext.EntryPoint;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SecWinNtFdAddress (
  IN     UINTN                 Index,
  IN OUT EFI_PHYSICAL_ADDRESS  *FdBase,
  IN OUT UINT64                *FdSize
  )
/*++

Routine Description:
  Return the FD Size and base address. Since the FD is loaded from a
  file into Windows memory only the SEC will know it's address.

Arguments:
  Index  - Which FD, starts at zero.
  FdSize - Size of the FD in bytes
  FdBase - Start address of the FD. Assume it points to an FV Header

Returns:
  EFI_SUCCESS     - Return the Base address and size of the FV
  EFI_UNSUPPORTED - Index does nto map to an FD in the system

--*/
{
  if (Index >= gFdInfoCount) {
    return EFI_UNSUPPORTED;
  }

  *FdBase = gFdInfo[Index].Address;
  *FdSize = gFdInfo[Index].Size;

  if (*FdBase == 0 && *FdSize == 0) {
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SecImageRead (
  IN     VOID    *FileHandle,
  IN     UINTN   FileOffset,
  IN OUT UINTN   *ReadSize,
  OUT    VOID    *Buffer
  )
/*++

Routine Description:
  Support routine for the PE/COFF Loader that reads a buffer from a PE/COFF file

Arguments:
  FileHandle - The handle to the PE/COFF file
  FileOffset - The offset, in bytes, into the file to read
  ReadSize   - The number of bytes to read from the file starting at FileOffset
  Buffer     - A pointer to the buffer to read the data into.

Returns:
  EFI_SUCCESS - ReadSize bytes of data were read into Buffer from the PE/COFF file starting at FileOffset

--*/
{
  CHAR8 *Destination8;
  CHAR8 *Source8;
  UINTN Length;

  Destination8  = Buffer;
  Source8       = (CHAR8 *) ((UINTN) FileHandle + FileOffset);
  Length        = *ReadSize;
  while (Length--) {
    *(Destination8++) = *(Source8++);
  }

  return EFI_SUCCESS;
}

CHAR16 *
AsciiToUnicode (
  IN  CHAR8   *Ascii,
  IN  UINTN   *StrLen OPTIONAL
  )
/*++

Routine Description:
  Convert the passed in Ascii string to Unicode.
  Optionally return the length of the strings.

Arguments:
  Ascii   - Ascii string to convert
  StrLen  - Length of string

Returns:
  Pointer to malloc'ed Unicode version of Ascii

--*/
{
  UINTN   Index;
  CHAR16  *Unicode;

  //
  // Allocate a buffer for unicode string
  //
  for (Index = 0; Ascii[Index] != '\0'; Index++)
    ;
  Unicode = malloc ((Index + 1) * sizeof (CHAR16));
  if (Unicode == NULL) {
    return NULL;
  }

  for (Index = 0; Ascii[Index] != '\0'; Index++) {
    Unicode[Index] = (CHAR16) Ascii[Index];
  }

  Unicode[Index] = '\0';

  if (StrLen != NULL) {
    *StrLen = Index;
  }

  return Unicode;
}

UINTN
CountSeperatorsInString (
  IN  const CHAR16   *String,
  IN  CHAR16         Seperator
  )
/*++

Routine Description:
  Count the number of seperators in String

Arguments:
  String    - String to process
  Seperator - Item to count

Returns:
  Number of Seperator in String

--*/
{
  UINTN Count;

  for (Count = 0; *String != '\0'; String++) {
    if (*String == Seperator) {
      Count++;
    }
  }

  return Count;
}


EFI_STATUS
AddModHandle (
  IN  PE_COFF_LOADER_IMAGE_CONTEXT         *ImageContext,
  IN  VOID                                 *ModHandle
  )
/*++

Routine Description:
  Store the ModHandle in an array indexed by the Pdb File name.
  The ModHandle is needed to unload the image. 

Arguments:
  ImageContext - Input data returned from PE Laoder Library. Used to find the 
                 .PDB file name of the PE Image.
  ModHandle    - Returned from LoadLibraryEx() and stored for call to 
                 FreeLibrary().

Returns:
  EFI_SUCCESS - ModHandle was stored. 

--*/
{
  UINTN                   Index;
  PDB_NAME_TO_MOD_HANDLE  *Array;
  UINTN                   PreviousSize;


  Array = mPdbNameModHandleArray;
  for (Index = 0; Index < mPdbNameModHandleArraySize; Index++, Array++) {
    if (Array->PdbPointer == NULL) {
      //
      // Make a copy of the stirng and store the ModHandle
      //
      Array->PdbPointer = malloc (strlen (ImageContext->PdbPointer) + 1);
      ASSERT (Array->PdbPointer != NULL);

      strcpy (Array->PdbPointer, ImageContext->PdbPointer);
      Array->ModHandle = ModHandle;
      return EFI_SUCCESS;
    }
  }
  
  //
  // No free space in mPdbNameModHandleArray so grow it by 
  // MAX_PDB_NAME_TO_MOD_HANDLE_ARRAY_SIZE entires. realloc will
  // copy the old values to the new locaiton. But it does
  // not zero the new memory area.
  //
  PreviousSize = mPdbNameModHandleArraySize * sizeof (PDB_NAME_TO_MOD_HANDLE);
  mPdbNameModHandleArraySize += MAX_PDB_NAME_TO_MOD_HANDLE_ARRAY_SIZE;

  mPdbNameModHandleArray = realloc (mPdbNameModHandleArray, mPdbNameModHandleArraySize * sizeof (PDB_NAME_TO_MOD_HANDLE));
  if (mPdbNameModHandleArray == NULL) {
    ASSERT (FALSE);
    return EFI_OUT_OF_RESOURCES;
  }
  
  memset (mPdbNameModHandleArray + PreviousSize, 0, MAX_PDB_NAME_TO_MOD_HANDLE_ARRAY_SIZE * sizeof (PDB_NAME_TO_MOD_HANDLE));
 
  return AddModHandle (ImageContext, ModHandle);
}


VOID *
RemoveModeHandle (
  IN  PE_COFF_LOADER_IMAGE_CONTEXT         *ImageContext
  )
/*++

Routine Description:
  Return the ModHandle and delete the entry in the array.

Arguments:
  ImageContext - Input data returned from PE Laoder Library. Used to find the 
                 .PDB file name of the PE Image.

Returns:
  ModHandle - ModHandle assoicated with ImageContext is returned
  NULL      - No ModHandle associated with ImageContext

--*/
{
  UINTN                   Index;
  PDB_NAME_TO_MOD_HANDLE  *Array;

  if (ImageContext->PdbPointer == NULL) {
    //
    // If no PDB pointer there is no ModHandle so return NULL
    //
    return NULL;
  }

  Array = mPdbNameModHandleArray;
  for (Index = 0; Index < mPdbNameModHandleArraySize; Index++, Array++) {
    if ((Array->PdbPointer != NULL) && (strcmp(Array->PdbPointer, ImageContext->PdbPointer) == 0)) {
      //
      // If you find a match return it and delete the entry
      //
      free (Array->PdbPointer);
      Array->PdbPointer = NULL;
      return Array->ModHandle;
    }
  }

  return NULL;
}



EFI_STATUS
EFIAPI
SecNt32PeCoffGetImageInfo (
  IN EFI_PEI_PE_COFF_LOADER_PROTOCOL          *This,
  IN OUT PE_COFF_LOADER_IMAGE_CONTEXT         *ImageContext
  )
{
  EFI_STATUS  Status;

  Status = PeCoffLoaderGetImageInfo (ImageContext);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  switch (ImageContext->ImageType) {

  case EFI_IMAGE_SUBSYSTEM_EFI_APPLICATION:
    ImageContext->ImageCodeMemoryType = EfiLoaderCode;
    ImageContext->ImageDataMemoryType = EfiLoaderData;
    break;

  case EFI_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER:
    ImageContext->ImageCodeMemoryType = EfiBootServicesCode;
    ImageContext->ImageDataMemoryType = EfiBootServicesData;
    break;

  case EFI_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER:
  case EFI_IMAGE_SUBSYSTEM_SAL_RUNTIME_DRIVER:
    ImageContext->ImageCodeMemoryType = EfiRuntimeServicesCode;
    ImageContext->ImageDataMemoryType = EfiRuntimeServicesData;
    break;

  default:
    ImageContext->ImageError = IMAGE_ERROR_INVALID_SUBSYSTEM;
    return RETURN_UNSUPPORTED;
  }

  return Status;
}

EFI_STATUS
EFIAPI
SecNt32PeCoffLoadImage (
  IN EFI_PEI_PE_COFF_LOADER_PROTOCOL          *This,
  IN OUT PE_COFF_LOADER_IMAGE_CONTEXT         *ImageContext
  )
{
  EFI_STATUS  Status;

  Status = PeCoffLoaderLoadImage (ImageContext);
  return Status;
}

EFI_STATUS
EFIAPI
SecNt32PeCoffRelocateImage (
  IN EFI_PEI_PE_COFF_LOADER_PROTOCOL          *This,
  IN OUT PE_COFF_LOADER_IMAGE_CONTEXT         *ImageContext
  )
{
  EFI_STATUS        Status;
  VOID              *DllEntryPoint;
  CHAR16            *DllFileName;
  HMODULE           Library;
  UINTN             Index;


  Status = PeCoffLoaderRelocateImage (ImageContext);
  if (EFI_ERROR (Status)) {
    //
    // We could not relocated the image in memory properly
    //
    return Status;
  }

  //
  // If we load our own PE COFF images the Windows debugger can not source
  //  level debug our code. If a valid PDB pointer exists usw it to load
  //  the *.dll file as a library using Windows* APIs. This allows 
  //  source level debug. The image is still loaded and reloaced
  //  in the Framework memory space like on a real system (by the code above),
  //  but the entry point points into the DLL loaded by the code bellow. 
  //

  DllEntryPoint = NULL;

  //
  // Load the DLL if it's not an EBC image.
  //
  if ((ImageContext->PdbPointer != NULL) &&
      (ImageContext->Machine != EFI_IMAGE_MACHINE_EBC)) {
    //
    // Convert filename from ASCII to Unicode
    //
    DllFileName = AsciiToUnicode (ImageContext->PdbPointer, &Index);

    //
    // Check that we have a valid filename
    //
    if (Index < 5 || DllFileName[Index - 4] != '.') {
      free (DllFileName);

      //
      // Never return an error if PeCoffLoaderRelocateImage() succeeded.
      // The image will run, but we just can't source level debug. If we
      // return an error the image will not run.
      //
      return EFI_SUCCESS;
    }
    //
    // Replace .PDB with .DLL on the filename
    //
    DllFileName[Index - 3]  = 'D';
    DllFileName[Index - 2]  = 'L';
    DllFileName[Index - 1]  = 'L';

    //
    // Load the .DLL file into the user process's address space for source 
    // level debug
    //
    Library = LoadLibraryEx (DllFileName, NULL, DONT_RESOLVE_DLL_REFERENCES);
    if (Library != NULL) {
      //
      // InitializeDriver is the entry point we put in all our EFI DLL's. The
      // DONT_RESOLVE_DLL_REFERENCES argument to LoadLIbraryEx() supresses the 
      // normal DLL entry point of DllMain, and prevents other modules that are
      // referenced in side the DllFileName from being loaded. There is no error 
      // checking as the we can point to the PE32 image loaded by Tiano. This 
      // step is only needed for source level debuging
      //
      DllEntryPoint = (VOID *) (UINTN) GetProcAddress (Library, "InitializeDriver");

    }

    if ((Library != NULL) && (DllEntryPoint != NULL)) {
      AddModHandle (ImageContext, Library);
      ImageContext->EntryPoint  = (EFI_PHYSICAL_ADDRESS) (UINTN) DllEntryPoint;
      wprintf (L"LoadLibraryEx (%s,\n               NULL, DONT_RESOLVE_DLL_REFERENCES)\n", DllFileName);
    } else {
      wprintf (L"WARNING: No source level debug %s. \n", DllFileName);
    }

    free (DllFileName);
  }

  //
  // Never return an error if PeCoffLoaderRelocateImage() succeeded.
  // The image will run, but we just can't source level debug. If we
  // return an error the image will not run.
  //
  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
SecNt32PeCoffUnloadimage (
  IN EFI_PEI_PE_COFF_LOADER_PROTOCOL      *This,
  IN PE_COFF_LOADER_IMAGE_CONTEXT         *ImageContext
  )
{
  VOID *ModHandle;

  ModHandle = RemoveModeHandle (ImageContext);
  if (ModHandle != NULL) {
    FreeLibrary (ModHandle);
  }
  return EFI_SUCCESS;
}

VOID
_ModuleEntryPoint (
  VOID
  )
{
}

#pragma warning(default : 4996)
