## @file
#
# EFI/Framework Emulation Platform with UEFI HII interface supported.
#
# The Emulation Platform can be used to debug individual modules, prior to creating
#       a real platform. This also provides an example for how an DSC is created.
# Copyright (c) 2006 - 2011, Intel Corporation. All rights reserved.<BR>
# Portions copyright (c) 2010 - 2011, Apple Inc. All rights reserved.<BR>
#
#    This program and the accompanying materials
#    are licensed and made available under the terms and conditions of the BSD License
#    which accompanies this distribution. The full text of the license may be found at
#    http://opensource.org/licenses/bsd-license.php
#
#    THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#    WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
##

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = EmuUnix
  PLATFORM_GUID                  = 05FD064D-1073-E844-936C-A0E16317107D
  PLATFORM_VERSION               = 0.3
  DSC_ SPECIFICATION             = 0x00010005
  OUTPUT_DIRECTORY               = Build/EmuUnixX64
  SUPPORTED_ARCHITECTURES        = X64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = InOsEmuPkg/Unix/UnixX64.fdf

################################################################################
#
# SKU Identification section - list of all SKU IDs supported by this Platform.
#
################################################################################
[SkuIds]
  0|DEFAULT

################################################################################
#
# Library Class section - list of all Library Classes needed by this Platform.
#
################################################################################
[LibraryClasses]
  #
  # Entry point
  #
  PeiCoreEntryPoint|MdePkg/Library/PeiCoreEntryPoint/PeiCoreEntryPoint.inf
  PeimEntryPoint|MdePkg/Library/PeimEntryPoint/PeimEntryPoint.inf
  DxeCoreEntryPoint|MdePkg/Library/DxeCoreEntryPoint/DxeCoreEntryPoint.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  #
  # Basic
  #
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  CacheMaintenanceLib|MdePkg/Library/BaseCacheMaintenanceLib/BaseCacheMaintenanceLib.inf
  PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf

  #
  # UEFI & PI
  #
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  UefiDecompressLib|IntelFrameworkModulePkg/Library/BaseUefiTianoCustomDecompressLib/BaseUefiTianoCustomDecompressLib.inf

  PeiServicesLib|MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  DxeServicesTableLib|MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
  #
  # Generic Modules
  #
  UefiScsiLib|MdePkg/Library/UefiScsiLib/UefiScsiLib.inf
  NetLib|MdeModulePkg/Library/DxeNetLib/DxeNetLib.inf
  IpIoLib|MdeModulePkg/Library/DxeIpIoLib/DxeIpIoLib.inf
  UdpIoLib|MdeModulePkg/Library/DxeUdpIoLib/DxeUdpIoLib.inf
  DpcLib|MdeModulePkg/Library/DxeDpcLib/DxeDpcLib.inf
  OemHookStatusCodeLib|MdeModulePkg/Library/OemHookStatusCodeLibNull/OemHookStatusCodeLibNull.inf
  GenericBdsLib|IntelFrameworkModulePkg/Library/GenericBdsLib/GenericBdsLib.inf
  SecurityManagementLib|MdeModulePkg/Library/DxeSecurityManagementLib/DxeSecurityManagementLib.inf
  TimerLib|MdePkg/Library/BaseTimerLibNullTemplate/BaseTimerLibNullTemplate.inf
  SerialPortLib|MdePkg/Library/BaseSerialPortLibNull/BaseSerialPortLibNull.inf
  CapsuleLib|MdeModulePkg/Library/DxeCapsuleLibNull/DxeCapsuleLibNull.inf
  #
  # Platform
  #
  PlatformBdsLib|InOsEmuPkg/Library/EmuBdsLib/EmuBdsLib.inf
  KeyMapLib|InOsEmuPkg/Library/KeyMapLibNull/KeyMapLibNull.inf
  
  #
  # Misc
  #
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  DebugAgentLib|MdeModulePkg/Library/DebugAgentLibNull/DebugAgentLibNull.inf
  DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf

####  DevicePathTextLib|InOsEmuPkg/Library/DevicePathTextLib/DevicePathTextLib.inf
  PeiServicesTablePointerLib|InOsEmuPkg/Library/PeiServicesTablePointerLibMagicPage/PeiServicesTablePointerLibMagicPage.inf

[LibraryClasses.common.SEC]
  PeiServicesLib|InOsEmuPkg/Library/SecPeiServicesLib/SecPeiServicesLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PeCoffGetEntryPointLib|InOsEmuPkg/Library/PeiEmuPeCoffGetEntryPointLib/PeiEmuPeCoffGetEntryPointLib.inf
  PeCoffExtraActionLib|InOsEmuPkg/Library/PeiEmuPeCoffExtraActionLib/PeiEmuPeCoffExtraActionLib.inf
  SerialPortLib|InOsEmuPkg/Library/PeiEmuSerialPortLib/PeiEmuSerialPortLib.inf
  PpiListLib|InOsEmuPkg/Library/SecPpiListLib/SecPpiListLib.inf

[LibraryClasses.common.USER_DEFINED, LibraryClasses.common.BASE]
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
  MemoryAllocationLib|MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PpiListLib|InOsEmuPkg/Library/SecPpiListLib/SecPpiListLib.inf
  ThunkPpiList|InOsEmuPkg/Library/ThunkPpiList/ThunkPpiList.inf
  ThunkProtocolList|InOsEmuPkg/Library/ThunkProtocolList/ThunkProtocolList.inf
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  PpiListLib|InOsEmuPkg/Library/SecPpiListLib/SecPpiListLib.inf
  PeiServicesLib|InOsEmuPkg/Library/SecPeiServicesLib/SecPeiServicesLib.inf


[LibraryClasses.common.PEIM, LibraryClasses.common.PEI_CORE]
  HobLib|MdePkg/Library/PeiHobLib/PeiHobLib.inf
  MemoryAllocationLib|MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/PeiReportStatusCodeLib/PeiReportStatusCodeLib.inf
  PeCoffGetEntryPointLib|InOsEmuPkg/Library/PeiEmuPeCoffGetEntryPointLib/PeiEmuPeCoffGetEntryPointLib.inf
  PeCoffExtraActionLib|InOsEmuPkg/Library/PeiEmuPeCoffExtraActionLib/PeiEmuPeCoffExtraActionLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/PeiExtractGuidedSectionLib/PeiExtractGuidedSectionLib.inf
  SerialPortLib|InOsEmuPkg/Library/PeiEmuSerialPortLib/PeiEmuSerialPortLib.inf

[LibraryClasses.common.PEI_CORE]
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf

[LibraryClasses.common.PEIM]
  PcdLib|MdePkg/Library/PeiPcdLib/PeiPcdLib.inf

[LibraryClasses.common.DXE_CORE]
  HobLib|MdePkg/Library/DxeCoreHobLib/DxeCoreHobLib.inf
  MemoryAllocationLib|MdeModulePkg/Library/DxeCoreMemoryAllocationLib/DxeCoreMemoryAllocationLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  PeCoffExtraActionLib|InOsEmuPkg/Library/DxeEmuPeCoffExtraActionLib/DxeEmuPeCoffExtraActionLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf

[LibraryClasses.common.DXE_RUNTIME_DRIVER, LibraryClasses.common.UEFI_DRIVER, LibraryClasses.common.DXE_DRIVER, LibraryClasses.common.UEFI_APPLICATION]
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  EmuThunkLib|InOsEmuPkg/Library/DxeEmuLib/DxeEmuLib.inf
  PeCoffExtraActionLib|InOsEmuPkg/Library/DxeEmuPeCoffExtraActionLib/DxeEmuPeCoffExtraActionLib.inf

[LibraryClasses.common.UEFI_DRIVER]
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
 
[LibraryClasses.common.UEFI_APPLICATION]
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf


################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform.
#
################################################################################
[PcdsFeatureFlag]
  gEfiMdeModulePkgTokenSpaceGuid.PcdDxeIplSwitchToLongMode|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdStatusCodeUseSerial|TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdPeiCoreImageLoaderSearchTeSectionFirst|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdResetOnMemoryTypeInformationChange|FALSE

[PcdsFixedAtBuild]
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000040
  gEfiMdePkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0x0f
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x1f
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxSizeNonPopulateCapsule|0x0
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxSizePopulateCapsule|0x0

  gInOsEmuPkgTokenSpaceGuid.PcdEmuFirmwareFdSize|0x002a0000
  gInOsEmuPkgTokenSpaceGuid.PcdEmuFirmwareBlockSize|0x10000
  gInOsEmuPkgTokenSpaceGuid.PcdEmuFirmwareVolume|L"../FV/FV_RECOVERY.fd"
  
  gInOsEmuPkgTokenSpaceGuid.PcdEmuMemorySizeForSecMain|L"64!64"

!if $(BUILD_NEW_SHELL)
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdShellFile|{ 0x83, 0xA5, 0x04, 0x7C, 0x3E, 0x9E, 0x1C, 0x4F, 0xAD, 0x65, 0xE0, 0x52, 0x68, 0xD0, 0xB4, 0xD1 }
!else
!if $(USE_NEW_SHELL)
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdShellFile|{ 0x83, 0xA5, 0x04, 0x7C, 0x3E, 0x9E, 0x1C, 0x4F, 0xAD, 0x65, 0xE0, 0x52, 0x68, 0xD0, 0xB4, 0xD1 }
!endif
!endif

#define BOOT_WITH_FULL_CONFIGURATION                  0x00
#define BOOT_WITH_MINIMAL_CONFIGURATION               0x01
#define BOOT_ASSUMING_NO_CONFIGURATION_CHANGES        0x02
#define BOOT_WITH_FULL_CONFIGURATION_PLUS_DIAGNOSTICS 0x03
#define BOOT_WITH_DEFAULT_SETTINGS                    0x04
#define BOOT_ON_S4_RESUME                             0x05
#define BOOT_ON_S5_RESUME                             0x06
#define BOOT_ON_S2_RESUME                             0x10
#define BOOT_ON_S3_RESUME                             0x11
#define BOOT_ON_FLASH_UPDATE                          0x12
#define BOOT_IN_RECOVERY_MODE                         0x20
  gInOsEmuPkgTokenSpaceGuid.PcdEmuBootMode|0
  
  gInOsEmuPkgTokenSpaceGuid.PcdEmuApCount|L"1"

  gInOsEmuPkgTokenSpaceGuid.PcdEmuVirtualDisk|L"disk.dmg:FW"
  gInOsEmuPkgTokenSpaceGuid.PcdEmuGop|L"GOP Window"
  gInOsEmuPkgTokenSpaceGuid.PcdEmuFileSystem|L".!../../../../EdkShellBinPkg/Bin"
  gInOsEmuPkgTokenSpaceGuid.PcdEmuSerialPort|L"/dev/ttyS0"
  gInOsEmuPkgTokenSpaceGuid.PcdEmuNetworkInterface|L"en0"

  gInOsEmuPkgTokenSpaceGuid.PcdEmuCpuModel|L"Intel(R) Processor Model"
  gInOsEmuPkgTokenSpaceGuid.PcdEmuCpuSpeed|L"3000"

  #  0-PCANSI, 1-VT100, 2-VT00+, 3-UTF8
  gEfiMdePkgTokenSpaceGuid.PcdDefaultTerminalType|1

################################################################################
#
# Pcd Dynamic Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################

[PcdsDynamicDefault.common.DEFAULT]
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwSpareBase64|0
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwWorkingBase64|0
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableBase64|0

[PcdsDynamicHii.common.DEFAULT]
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|L"Setup"|gEmuSystemConfigGuid|0x0|80
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|L"Setup"|gEmuSystemConfigGuid|0x4|25
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdPlatformBootTimeOut|L"Timeout"|gEfiGlobalVariableGuid|0x0|10


###################################################################################################
#
# Components Section - list of the modules and components that will be processed by compilation
#                      tools and the EDK II tools to generate PE32/PE32+/Coff image files.
#
# Note: The EDK II DSC file is not used to specify how compiled binary images get placed
#       into firmware volume images. This section is just a list of modules to compile from
#       source into UEFI-compliant binaries.
#       It is the FDF file that contains information on combining binary files into firmware
#       volume images, whose concept is beyond UEFI and is described in PI specification.
#       Binary modules do not need to be listed in this section, as they should be
#       specified in the FDF file. For example: Shell binary (Shell_Full.efi), FAT binary (Fat.efi),
#       Logo (Logo.bmp), and etc.
#       There may also be modules listed in this section that are not required in the FDF file,
#       When a module listed here is excluded from FDF file, then UEFI-compliant binary will be
#       generated for it, but the binary will not be put into any firmware volume.
#
###################################################################################################
[Components]
!if $(SEC_ONLY)
  ##
  #  SEC Phase modules
  ##
  InOsEmuPkg/Unix/Sec/SecMain.inf
!else
  #
  # Generic SEC
  #
  InOsEmuPkg/Sec/Sec.inf

  ##
  #  PEI Phase modules
  ##
  MdeModulePkg/Core/Pei/PeiMain.inf
  MdeModulePkg/Universal/PCD/Pei/Pcd.inf  {
   <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }
  MdeModulePkg/Universal/ReportStatusCodeRouter/Pei/ReportStatusCodeRouterPei.inf
  MdeModulePkg/Universal/StatusCodeHandler/Pei/StatusCodeHandlerPei.inf 
  
  IntelFrameworkModulePkg/Universal/StatusCode/Pei/StatusCodePei.inf
  InOsEmuPkg/BootModePei/BootModePei.inf
  MdeModulePkg/Universal/Variable/Pei/VariablePei.inf
  InOsEmuPkg/AutoScanPei/AutoScanPei.inf
  InOsEmuPkg/FirmwareVolumePei/FirmwareVolumePei.inf
  InOsEmuPkg/FlashMapPei/FlashMapPei.inf
  InOsEmuPkg/ThunkPpiToProtocolPei/ThunkPpiToProtocolPei.inf
  MdeModulePkg/Core/DxeIplPeim/DxeIpl.inf {
    <LibraryClasses>
      # turn off CR3 write so that DXE IPL will not crash emulator
      BaseLib|UnixPkg/Library/UnixBaseLib/UnixBaseLib.inf
  }

  ##
  #  DXE Phase modules
  ##
  MdeModulePkg/Core/Dxe/DxeMain.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Library/DxeCrc32GuidedSectionExtractLib/DxeCrc32GuidedSectionExtractLib.inf
      NULL|IntelFrameworkModulePkg/Library/LzmaCustomDecompressLib/LzmaCustomDecompressLib.inf
  }
  MdeModulePkg/Universal/PCD/Dxe/Pcd.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }

  MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/ReportStatusCodeRouterRuntimeDxe.inf
  MdeModulePkg/Universal/StatusCodeHandler/RuntimeDxe/StatusCodeHandlerRuntimeDxe.inf {
   <LibraryClasses>
      SerialPortLib|InOsEmuPkg/Library/DxeEmuStdErrSerialPortLib/DxeEmuStdErrSerialPortLib.inf    
  }

  InOsEmuPkg/MetronomeDxe/Metronome.inf
  InOsEmuPkg/RealTimeClockRuntimeDxe/RealTimeClock.inf
  InOsEmuPkg/ResetRuntimeDxe/Reset.inf
  MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
  InOsEmuPkg/FvbServicesRuntimeDxe/FvbServicesRuntimeDxe.inf
  MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf
  MdeModulePkg/Universal/EbcDxe/EbcDxe.inf
  MdeModulePkg/Universal/MemoryTest/NullMemoryTestDxe/NullMemoryTestDxe.inf
  InOsEmuPkg/EmuThunkDxe/EmuThunk.inf
  InOsEmuPkg/CpuRuntimeDxe/Cpu.inf
  MdeModulePkg/Universal/FaultTolerantWriteDxe/FaultTolerantWriteDxe.inf
  InOsEmuPkg/MiscSubClassPlatformDxe/MiscSubClassDriver.inf
  InOsEmuPkg/TimerDxe/Timer.inf


  MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe.inf
  MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf
  MdeModulePkg/Universal/MonotonicCounterRuntimeDxe/MonotonicCounterRuntimeDxe.inf
  MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
  MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
  MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf
  MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf
  EmbeddedPkg/SerialDxe/SerialDxe.inf {
   <LibraryClasses>
      DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
      SerialPortLib|InOsEmuPkg/Library/DxeEmuSerialPortLib/DxeEmuSerialPortLib.inf    
  }
  
  MdeModulePkg/Universal/Console/TerminalDxe/TerminalDxe.inf
  IntelFrameworkModulePkg/Universal/BdsDxe/BdsDxe.inf
  MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
  MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
  MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
  MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf
  MdeModulePkg/Bus/Pci/PciBusDxe/PciBusDxe.inf
  MdeModulePkg/Bus/Scsi/ScsiBusDxe/ScsiBusDxe.inf
  MdeModulePkg/Bus/Scsi/ScsiDiskDxe/ScsiDiskDxe.inf
  IntelFrameworkModulePkg/Bus/Pci/IdeBusDxe/IdeBusDxe.inf

  InOsEmuPkg/EmuBusDriverDxe/EmuBusDriverDxe.inf
  InOsEmuPkg/EmuGopDxe/EmuGopDxe.inf
  InOsEmuPkg/EmuSimpleFileSystemDxe/EmuSimpleFileSystemDxe.inf
  InOsEmuPkg/EmuBlockIoDxe/EmuBlockIoDxe.inf
  InOsEmuPkg/EmuSnpDxe/EmuSnpDxe.inf

  MdeModulePkg/Application/HelloWorld/HelloWorld.inf

  #
  # Network stack drivers
  #
  MdeModulePkg/Universal/Network/DpcDxe/DpcDxe.inf
  MdeModulePkg/Universal/Network/ArpDxe/ArpDxe.inf
  MdeModulePkg/Universal/Network/Dhcp4Dxe/Dhcp4Dxe.inf
  MdeModulePkg/Universal/Network/Ip4ConfigDxe/Ip4ConfigDxe.inf
  MdeModulePkg/Universal/Network/Ip4Dxe/Ip4Dxe.inf
  MdeModulePkg/Universal/Network/MnpDxe/MnpDxe.inf
  MdeModulePkg/Universal/Network/VlanConfigDxe/VlanConfigDxe.inf
  MdeModulePkg/Universal/Network/Mtftp4Dxe/Mtftp4Dxe.inf
  MdeModulePkg/Universal/Network/Tcp4Dxe/Tcp4Dxe.inf
  MdeModulePkg/Universal/Network/Udp4Dxe/Udp4Dxe.inf

  MdeModulePkg/Universal/SmbiosDxe/SmbiosDxe.inf
  MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf
  MdeModulePkg/Universal/SetupBrowserDxe/SetupBrowserDxe.inf
  MdeModulePkg/Universal/PrintDxe/PrintDxe.inf
  MdeModulePkg/Universal/DriverSampleDxe/DriverSampleDxe.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }

!if $(BUILD_FAT)
  FatPkg/EnhancedFatDxe/Fat.inf
!endif

!if $(BUILD_NEW_SHELL)
  ShellPkg/Application/Shell/Shell.inf {
    <LibraryClasses>
      ShellCommandLib|ShellPkg/Library/UefiShellCommandLib/UefiShellCommandLib.inf
      NULL|ShellPkg/Library/UefiShellLevel2CommandsLib/UefiShellLevel2CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellLevel1CommandsLib/UefiShellLevel1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellLevel3CommandsLib/UefiShellLevel3CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellDriver1CommandsLib/UefiShellDriver1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellDebug1CommandsLib/UefiShellDebug1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellInstall1CommandsLib/UefiShellInstall1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellNetwork1CommandsLib/UefiShellNetwork1CommandsLib.inf
      HandleParsingLib|ShellPkg/Library/UefiHandleParsingLib/UefiHandleParsingLib.inf
      FileHandleLib|ShellPkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
      ShellLib|ShellPkg/Library/UefiShellLib/UefiShellLib.inf
      SortLib|ShellPkg/Library/UefiSortLib/UefiSortLib.inf
      PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0xFF
      gEfiShellPkgTokenSpaceGuid.PcdShellLibAutoInitialize|FALSE
      gEfiMdePkgTokenSpaceGuid.PcdUefiLibMaxPrintBufferSize|8000
  }
!endif

!endif  
