/** @file
  Copyright (c) 2006, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

  Module Name:  AtapiPassThru.h

**/

#ifndef _APT_H
#define _APT_H



#include <Uefi.h>

#include <Protocol/ScsiPassThru.h>
#include <Protocol/PciIo.h>

#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <IndustryStandard/pci22.h>

///
/// bit definition
///
#define bit(a)        (1 << (a))

#define MAX_TARGET_ID 4

//
// IDE Registers
//
typedef union {
  UINT16  Command;        /* when write */
  UINT16  Status;         /* when read */
} IDE_CMD_OR_STATUS;

typedef union {
  UINT16  Error;          /* when read */
  UINT16  Feature;        /* when write */
} IDE_ERROR_OR_FEATURE;

typedef union {
  UINT16  AltStatus;      /* when read */
  UINT16  DeviceControl;  /* when write */
} IDE_AltStatus_OR_DeviceControl;


typedef enum {
  IdePrimary    = 0,
  IdeSecondary  = 1,
  IdeMaxChannel = 2
} EFI_IDE_CHANNEL;

///


//
// Bit definitions in Programming Interface byte of the Class Code field
// in PCI IDE controller's Configuration Space
//
#define IDE_PRIMARY_OPERATING_MODE            BIT0
#define IDE_PRIMARY_PROGRAMMABLE_INDICATOR    BIT1
#define IDE_SECONDARY_OPERATING_MODE          BIT2
#define IDE_SECONDARY_PROGRAMMABLE_INDICATOR  BIT3


#define ATAPI_MAX_CHANNEL 2

///
/// IDE registers set
///
typedef struct {
  UINT16                          Data;
  IDE_ERROR_OR_FEATURE            Reg1;
  UINT16                          SectorCount;
  UINT16                          SectorNumber;
  UINT16                          CylinderLsb;
  UINT16                          CylinderMsb;
  UINT16                          Head;
  IDE_CMD_OR_STATUS               Reg;
  IDE_AltStatus_OR_DeviceControl  Alt;
  UINT16                          DriveAddress;
} IDE_BASE_REGISTERS;

#define ATAPI_SCSI_PASS_THRU_DEV_SIGNATURE  EFI_SIGNATURE_32 ('a', 's', 'p', 't')

typedef struct {
  UINTN                            Signature;
  EFI_HANDLE                       Handle;
  EFI_SCSI_PASS_THRU_PROTOCOL      ScsiPassThru;
  EFI_SCSI_PASS_THRU_MODE          ScsiPassThruMode;
  EFI_PCI_IO_PROTOCOL              *PciIo;
  UINT64                           OriginalPciAttributes;
  //
  // Local Data goes here
  //
  IDE_BASE_REGISTERS               *IoPort;
  IDE_BASE_REGISTERS               AtapiIoPortRegisters[2];
  CHAR16                           ControllerName[100];
  CHAR16                           ChannelName[100];
  UINT32                           LatestTargetId;
  UINT64                           LatestLun;
} ATAPI_SCSI_PASS_THRU_DEV;

//
// IDE registers' base addresses
//
typedef struct {
  UINT16  CommandBlockBaseAddr;
  UINT16  ControlBlockBaseAddr;
} IDE_REGISTERS_BASE_ADDR;

#define ATAPI_SCSI_PASS_THRU_DEV_FROM_THIS(a) \
  CR (a, \
      ATAPI_SCSI_PASS_THRU_DEV, \
      ScsiPassThru, \
      ATAPI_SCSI_PASS_THRU_DEV_SIGNATURE \
      )

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL   gAtapiScsiPassThruDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL   gAtapiScsiPassThruComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL  gAtapiScsiPassThruComponentName2;

//
// ATAPI Command op code
//
#define OP_INQUIRY                      0x12
#define OP_LOAD_UNLOAD_CD               0xa6
#define OP_MECHANISM_STATUS             0xbd
#define OP_MODE_SELECT_10               0x55
#define OP_MODE_SENSE_10                0x5a
#define OP_PAUSE_RESUME                 0x4b
#define OP_PLAY_AUDIO_10                0x45
#define OP_PLAY_AUDIO_MSF               0x47
#define OP_PLAY_CD                      0xbc
#define OP_PLAY_CD_MSF                  0xb4
#define OP_PREVENT_ALLOW_MEDIUM_REMOVAL 0x1e
#define OP_READ_10                      0x28
#define OP_READ_12                      0xa8
#define OP_READ_CAPACITY                0x25
#define OP_READ_CD                      0xbe
#define OP_READ_CD_MSF                  0xb9
#define OP_READ_HEADER                  0x44
#define OP_READ_SUB_CHANNEL             0x42
#define OP_READ_TOC                     0x43
#define OP_REQUEST_SENSE                0x03
#define OP_SCAN                         0xba
#define OP_SEEK_10                      0x2b
#define OP_SET_CD_SPEED                 0xbb
#define OP_STOPPLAY_SCAN                0x4e
#define OP_START_STOP_UNIT              0x1b
#define OP_TEST_UNIT_READY              0x00

#define OP_FORMAT_UNIT                  0x04
#define OP_READ_FORMAT_CAPACITIES       0x23
#define OP_VERIFY                       0x2f
#define OP_WRITE_10                     0x2a
#define OP_WRITE_12                     0xaa
#define OP_WRITE_AND_VERIFY             0x2e

//
// ATA Command
//
#define ATAPI_SOFT_RESET_CMD  0x08

typedef enum {
  DataIn  = 0,
  DataOut = 1,
  NoData  = 2,
  End     = 0xff
} DATA_DIRECTION;

typedef struct {
  UINT8           OpCode;
  DATA_DIRECTION  Direction;
} SCSI_COMMAND_SET;

#define MAX_CHANNEL         2

#define ValidCdbLength(Len) ((Len) == 6 || (Len) == 10 || (Len) == 12) ? 1 : 0

//
// IDE registers bit definitions
//
// ATA Err Reg bitmap
//
#define BBK_ERR   bit (7) ///< Bad block detected
#define UNC_ERR   bit (6) ///< Uncorrectable Data
#define MC_ERR    bit (5) ///< Media Change
#define IDNF_ERR  bit (4) ///< ID Not Found
#define MCR_ERR   bit (3) ///< Media Change Requested
#define ABRT_ERR  bit (2) ///< Aborted Command
#define TK0NF_ERR bit (1) ///< Track 0 Not Found
#define AMNF_ERR  bit (0) ///< Address Mark Not Found

//
// ATAPI Err Reg bitmap
//
#define SENSE_KEY_ERR (bit (7) | bit (6) | bit (5) | bit (4))
#define EOM_ERR bit (1) ///< End of Media Detected
#define ILI_ERR bit (0) ///< Illegal Length Indication

//
// Device/Head Reg
//
#define LBA_MODE  bit (6)
#define DEV       bit (4)
#define HS3       bit (3)
#define HS2       bit (2)
#define HS1       bit (1)
#define HS0       bit (0)
#define CHS_MODE  (0)
#define DRV0      (0)
#define DRV1      (1)
#define MST_DRV   DRV0
#define SLV_DRV   DRV1

//
// Status Reg
//
#define BSY   bit (7) ///< Controller Busy
#define DRDY  bit (6) ///< Drive Ready
#define DWF   bit (5) ///< Drive Write Fault
#define DSC   bit (4) ///< Disk Seek Complete
#define DRQ   bit (3) ///< Data Request
#define CORR  bit (2) ///< Corrected Data
#define IDX   bit (1) ///< Index
#define ERR   bit (0) ///< Error
#define CHECK bit (0) ///< Check bit for ATAPI Status Reg

//
// Device Control Reg
//
#define SRST  bit (2) ///< Software Reset
#define IEN_L bit (1) ///< Interrupt Enable

//
// ATAPI Feature Register
//
#define OVERLAP bit (1)
#define DMA     bit (0)

//
// ATAPI Interrupt Reason Reson Reg (ATA Sector Count Register)
//
#define RELEASE     bit (2)
#define IO          bit (1)
#define CoD         bit (0)

#define PACKET_CMD  0xA0

#define DEFAULT_CMD (0xa0)
//
// default content of device control register, disable INT
//
#define DEFAULT_CTL           (0x0a)
#define MAX_ATAPI_BYTE_COUNT  (0xfffe)

//
// function prototype
//

EFI_STATUS
EFIAPI
AtapiScsiPassThruDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
AtapiScsiPassThruDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
AtapiScsiPassThruDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  );

//
// EFI Component Name Functions
//
/**
  Retrieves a Unicode string that is the user readable name of the driver.

  This function retrieves the user readable name of a driver in the form of a
  Unicode string. If the driver specified by This has a user readable name in
  the language specified by Language, then a pointer to the driver name is
  returned in DriverName, and EFI_SUCCESS is returned. If the driver specified
  by This does not support the language specified by Language,
  then EFI_UNSUPPORTED is returned.

  @param  This[in]              A pointer to the EFI_COMPONENT_NAME2_PROTOCOL or
                                EFI_COMPONENT_NAME_PROTOCOL instance.

  @param  Language[in]          A pointer to a Null-terminated ASCII string
                                array indicating the language. This is the
                                language of the driver name that the caller is
                                requesting, and it must match one of the
                                languages specified in SupportedLanguages. The
                                number of languages supported by a driver is up
                                to the driver writer. Language is specified
                                in RFC 3066 or ISO 639-2 language code format.

  @param  DriverName[out]       A pointer to the Unicode string to return.
                                This Unicode string is the name of the
                                driver specified by This in the language
                                specified by Language.

  @retval EFI_SUCCESS           The Unicode string for the Driver specified by
                                This and the language specified by Language was
                                returned in DriverName.

  @retval EFI_INVALID_PARAMETER Language is NULL.

  @retval EFI_INVALID_PARAMETER DriverName is NULL.

  @retval EFI_UNSUPPORTED       The driver specified by This does not support
                                the language specified by Language.

**/
EFI_STATUS
EFIAPI
AtapiScsiPassThruComponentNameGetDriverName (
  IN  EFI_COMPONENT_NAME_PROTOCOL  *This,
  IN  CHAR8                        *Language,
  OUT CHAR16                       **DriverName
  );


/**
  Retrieves a Unicode string that is the user readable name of the controller
  that is being managed by a driver.

  This function retrieves the user readable name of the controller specified by
  ControllerHandle and ChildHandle in the form of a Unicode string. If the
  driver specified by This has a user readable name in the language specified by
  Language, then a pointer to the controller name is returned in ControllerName,
  and EFI_SUCCESS is returned.  If the driver specified by This is not currently
  managing the controller specified by ControllerHandle and ChildHandle,
  then EFI_UNSUPPORTED is returned.  If the driver specified by This does not
  support the language specified by Language, then EFI_UNSUPPORTED is returned.

  @param  This[in]              A pointer to the EFI_COMPONENT_NAME2_PROTOCOL or
                                EFI_COMPONENT_NAME_PROTOCOL instance.

  @param  ControllerHandle[in]  The handle of a controller that the driver
                                specified by This is managing.  This handle
                                specifies the controller whose name is to be
                                returned.

  @param  ChildHandle[in]       The handle of the child controller to retrieve
                                the name of.  This is an optional parameter that
                                may be NULL.  It will be NULL for device
                                drivers.  It will also be NULL for a bus drivers
                                that wish to retrieve the name of the bus
                                controller.  It will not be NULL for a bus
                                driver that wishes to retrieve the name of a
                                child controller.

  @param  Language[in]          A pointer to a Null-terminated ASCII string
                                array indicating the language.  This is the
                                language of the driver name that the caller is
                                requesting, and it must match one of the
                                languages specified in SupportedLanguages. The
                                number of languages supported by a driver is up
                                to the driver writer. Language is specified in
                                RFC 3066 or ISO 639-2 language code format.

  @param  ControllerName[out]   A pointer to the Unicode string to return.
                                This Unicode string is the name of the
                                controller specified by ControllerHandle and
                                ChildHandle in the language specified by
                                Language from the point of view of the driver
                                specified by This.

  @retval EFI_SUCCESS           The Unicode string for the user readable name in
                                the language specified by Language for the
                                driver specified by This was returned in
                                DriverName.

  @retval EFI_INVALID_PARAMETER ControllerHandle is not a valid EFI_HANDLE.

  @retval EFI_INVALID_PARAMETER ChildHandle is not NULL and it is not a valid
                                EFI_HANDLE.

  @retval EFI_INVALID_PARAMETER Language is NULL.

  @retval EFI_INVALID_PARAMETER ControllerName is NULL.

  @retval EFI_UNSUPPORTED       The driver specified by This is not currently
                                managing the controller specified by
                                ControllerHandle and ChildHandle.

  @retval EFI_UNSUPPORTED       The driver specified by This does not support
                                the language specified by Language.

**/
EFI_STATUS
EFIAPI
AtapiScsiPassThruComponentNameGetControllerName (
  IN  EFI_COMPONENT_NAME_PROTOCOL                     *This,
  IN  EFI_HANDLE                                      ControllerHandle,
  IN  EFI_HANDLE                                      ChildHandle        OPTIONAL,
  IN  CHAR8                                           *Language,
  OUT CHAR16                                          **ControllerName
  );


/**
  AtapiScsiPassThruDriverEntryPoint

  @param ImageHandle
  @param SystemTable

  @todo Add function description
  @todo ImageHandle - add argument description
  @todo SystemTable - add argument description
  @todo add return values
--*/
EFI_STATUS
EFIAPI
AtapiScsiPassThruDriverEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
;

/**
  RegisterAtapiScsiPassThru

  @param  This
  @param  Controller
  @param  PciIo
  @param  OriginalPciAttributes

  @todo Add function description
  @todo This add argument description
  @todo Controller add argument description
  @todo PciIo add argument description
  @todo add return values
**/
EFI_STATUS
RegisterAtapiScsiPassThru (
  IN  EFI_DRIVER_BINDING_PROTOCOL *This,
  IN  EFI_HANDLE                  Controller,
  IN  EFI_PCI_IO_PROTOCOL         *PciIo,
  IN  UINT64                      OriginalPciAttributes
  )
;

/**
  AtapiScsiPassThruFunction

  @param  This
  @param  Target
  @param  Lun
  @param  Packet
  @param  Event

  @todo Add function description
  @todo  This - add argument description
  @todo  Target - add argument description
  @todo  Lun - add argument description
  @todo  Packet - add argument description
  @todo  Event - add argument description
  @todo add return values
**/
EFI_STATUS
EFIAPI
AtapiScsiPassThruFunction (
  IN EFI_SCSI_PASS_THRU_PROTOCOL                        *This,
  IN UINT32                                             Target,
  IN UINT64                                             Lun,
  IN OUT EFI_SCSI_PASS_THRU_SCSI_REQUEST_PACKET         *Packet,
  IN EFI_EVENT                                          Event OPTIONAL
  )
;

/**
  AtapiScsiPassThruGetNextDevice

  TODO: Add function description

  @param  This TODO: add argument description
  @param  Target TODO: add argument description
  @param  Lun TODO: add argument description

  TODO: add return values

**/
EFI_STATUS
EFIAPI
AtapiScsiPassThruGetNextDevice (
  IN  EFI_SCSI_PASS_THRU_PROTOCOL    *This,
  IN OUT UINT32                      *Target,
  IN OUT UINT64                      *Lun
  )
;

/**
  AtapiScsiPassThruBuildDevicePath

  TODO: Add function description

  @param  This TODO: add argument description
  @param  Target TODO: add argument description
  @param  Lun TODO: add argument description
  @param  DevicePath TODO: add argument description

  TODO: add return values

**/
EFI_STATUS
EFIAPI
AtapiScsiPassThruBuildDevicePath (
  IN     EFI_SCSI_PASS_THRU_PROTOCOL    *This,
  IN     UINT32                         Target,
  IN     UINT64                         Lun,
  IN OUT EFI_DEVICE_PATH_PROTOCOL       **DevicePath
  )
;

/**
  AtapiScsiPassThruGetTargetLun

  TODO: Add function description

  @param  This TODO: add argument description
  @param  DevicePath TODO: add argument description
  @param  Target TODO: add argument description
  @param  Lun TODO: add argument description

  TODO: add return values

**/
EFI_STATUS
EFIAPI
AtapiScsiPassThruGetTargetLun (
  IN  EFI_SCSI_PASS_THRU_PROTOCOL    *This,
  IN  EFI_DEVICE_PATH_PROTOCOL       *DevicePath,
  OUT UINT32                         *Target,
  OUT UINT64                         *Lun
  )
;

/**
  AtapiScsiPassThruResetChannel

  TODO: Add function description

  @param  This TODO: add argument description

  TODO: add return values

**/
EFI_STATUS
EFIAPI
AtapiScsiPassThruResetChannel (
  IN  EFI_SCSI_PASS_THRU_PROTOCOL   *This
  )
;

/**
  AtapiScsiPassThruResetTarget

  TODO: Add function description

  @param  This TODO: add argument description
  @param  Target TODO: add argument description
  @param  Lun TODO: add argument description

  TODO: add return values

**/
EFI_STATUS
EFIAPI
AtapiScsiPassThruResetTarget (
  IN EFI_SCSI_PASS_THRU_PROTOCOL    *This,
  IN UINT32                         Target,
  IN UINT64                         Lun
  )
;

/**
  CheckSCSIRequestPacket

  TODO: Add function description

  @param  Packet TODO: add argument description

  TODO: add return values

**/
EFI_STATUS
CheckSCSIRequestPacket (
  EFI_SCSI_PASS_THRU_SCSI_REQUEST_PACKET      *Packet
  )
;

/**
  SubmitBlockingIoCommand

  TODO: Add function description

  @param  AtapiScsiPrivate TODO: add argument description
  @param  Target TODO: add argument description
  @param  Packet TODO: add argument description

  TODO: add return values

**/
EFI_STATUS
SubmitBlockingIoCommand (
  ATAPI_SCSI_PASS_THRU_DEV                  *AtapiScsiPrivate,
  UINT32                                    Target,
  EFI_SCSI_PASS_THRU_SCSI_REQUEST_PACKET    *Packet
  )
;

/**
  IsCommandValid

  TODO: Add function description

  @param Packet  - TODO: add argument description

  @return TODO: add return values

--*/
BOOLEAN
IsCommandValid (
  EFI_SCSI_PASS_THRU_SCSI_REQUEST_PACKET   *Packet
  )
;

/**
  RequestSenseCommand

  TODO: Add function description

  @param  AtapiScsiPrivate TODO: add argument description
  @param  Target TODO: add argument description
  @param  Timeout TODO: add argument description
  @param  SenseData TODO: add argument description
  @param  SenseDataLength TODO: add argument description

  TODO: add return values

**/
EFI_STATUS
RequestSenseCommand (
  ATAPI_SCSI_PASS_THRU_DEV    *AtapiScsiPrivate,
  UINT32                      Target,
  UINT64                      Timeout,
  VOID                        *SenseData,
  UINT8                       *SenseDataLength
  )
;

/**
  AtapiPacketCommand

  TODO: Add function description

  @param  AtapiScsiPrivate TODO: add argument description
  @param  Target TODO: add argument description
  @param  PacketCommand TODO: add argument description
  @param  Buffer TODO: add argument description
  @param  ByteCount TODO: add argument description
  @param  Direction TODO: add argument description
  @param  TimeOutInMicroSeconds TODO: add argument description

  TODO: add return values

**/
EFI_STATUS
AtapiPacketCommand (
  ATAPI_SCSI_PASS_THRU_DEV                  *AtapiScsiPrivate,
  UINT32                                    Target,
  UINT8                                     *PacketCommand,
  VOID                                      *Buffer,
  UINT32                                    *ByteCount,
  DATA_DIRECTION                            Direction,
  UINT64                                    TimeOutInMicroSeconds
  )
;


/**
  ReadPortB

  TODO: Add function description

  @param  PciIo TODO: add argument description
  @param  Port TODO: add argument description

  TODO: add return values

**/
UINT8
ReadPortB (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port
  )
;


/**
  ReadPortW

  TODO: Add function description

  @param  PciIo TODO: add argument description
  @param  Port TODO: add argument description

  TODO: add return values

**/
UINT16
ReadPortW (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port
  )
;


/**
  WritePortB

  TODO: Add function description

  @param  PciIo TODO: add argument description
  @param  Port TODO: add argument description
  @param  Data TODO: add argument description

  TODO: add return values

**/
VOID
WritePortB (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port,
  IN  UINT8                 Data
  )
;


/**
  WritePortW

  TODO: Add function description

  @param  PciIo TODO: add argument description
  @param  Port TODO: add argument description
  @param  Data TODO: add argument description

  TODO: add return values

**/
VOID
WritePortW (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port,
  IN  UINT16                Data
  )
;

/**
  StatusDRQClear

  TODO: Add function description

  @param  AtapiScsiPrivate TODO: add argument description
  @param  TimeOutInMicroSeconds TODO: add argument description

  TODO: add return values

**/
EFI_STATUS
StatusDRQClear (
  ATAPI_SCSI_PASS_THRU_DEV        *AtapiScsiPrivate,
  UINT64                          TimeOutInMicroSeconds
  )
;

/**
  AltStatusDRQClear

  TODO: Add function description

  @param  AtapiScsiPrivate TODO: add argument description
  @param  TimeOutInMicroSeconds TODO: add argument description

  TODO: add return values

**/
EFI_STATUS
AltStatusDRQClear (
  ATAPI_SCSI_PASS_THRU_DEV        *AtapiScsiPrivate,
  UINT64                          TimeOutInMicroSeconds
  )
;

/**
  StatusDRQReady

  TODO: Add function description

  @param  AtapiScsiPrivate TODO: add argument description
  @param  TimeOutInMicroSeconds TODO: add argument description

  TODO: add return values

**/
EFI_STATUS
StatusDRQReady (
  ATAPI_SCSI_PASS_THRU_DEV        *AtapiScsiPrivate,
  UINT64                          TimeOutInMicroSeconds
  )
;

/**
  AltStatusDRQReady

  TODO: Add function description

  @param  AtapiScsiPrivate TODO: add argument description
  @param  TimeOutInMicroSeconds TODO: add argument description

  TODO: add return values

**/
EFI_STATUS
AltStatusDRQReady (
  ATAPI_SCSI_PASS_THRU_DEV        *AtapiScsiPrivate,
  UINT64                          TimeOutInMicroSeconds
  )
;

/**
  TODO: Add function description

  @param  AtapiScsiPrivate TODO: add argument description
  @param  TimeoutInMicroSeconds TODO: add argument description

  TODO: add return values

**/
EFI_STATUS
StatusWaitForBSYClear (
  ATAPI_SCSI_PASS_THRU_DEV    *AtapiScsiPrivate,
  UINT64                      TimeoutInMicroSeconds
  )
;

/**
  TODO: Add function description

  @param  AtapiScsiPrivate TODO: add argument description
  @param  TimeoutInMicroSeconds TODO: add argument description

  TODO: add return values

**/
EFI_STATUS
AltStatusWaitForBSYClear (
  ATAPI_SCSI_PASS_THRU_DEV    *AtapiScsiPrivate,
  UINT64                      TimeoutInMicroSeconds
  )
;

/**
  TODO: Add function description

  @param  AtapiScsiPrivate TODO: add argument description
  @param  TimeoutInMicroSeconds TODO: add argument description

  TODO: add return values

**/
EFI_STATUS
StatusDRDYReady (
  ATAPI_SCSI_PASS_THRU_DEV    *AtapiScsiPrivate,
  UINT64                      TimeoutInMicroSeconds
  )
;

/**
  TODO: Add function description

  @param  AtapiScsiPrivate TODO: add argument description
  @param  TimeoutInMicroSeconds TODO: add argument description

  TODO: add return values

**/
EFI_STATUS
AltStatusDRDYReady (
  ATAPI_SCSI_PASS_THRU_DEV    *AtapiScsiPrivate,
  UINT64                      TimeoutInMicroSeconds
  )
;

/**
  TODO: Add function description

  @param  AtapiScsiPrivate TODO: add argument description
  @param  Buffer TODO: add argument description
  @param  ByteCount TODO: add argument description
  @param  Direction TODO: add argument description
  @param  TimeOutInMicroSeconds TODO: add argument description

  TODO: add return values

**/
EFI_STATUS
AtapiPassThruPioReadWriteData (
  ATAPI_SCSI_PASS_THRU_DEV  *AtapiScsiPrivate,
  UINT16                    *Buffer,
  UINT32                    *ByteCount,
  DATA_DIRECTION            Direction,
  UINT64                    TimeOutInMicroSeconds
  )
;

/**
  TODO: Add function description

  @param  AtapiScsiPrivate TODO: add argument description

  TODO: add return values

**/
EFI_STATUS
AtapiPassThruCheckErrorStatus (
  ATAPI_SCSI_PASS_THRU_DEV        *AtapiScsiPrivate
  )
;
EFI_STATUS
GetIdeRegistersBaseAddr (
  IN  EFI_PCI_IO_PROTOCOL         *PciIo,
  OUT IDE_REGISTERS_BASE_ADDR     *IdeRegsBaseAddr
  )
/*++

Routine Description:
  Get IDE IO port registers' base addresses by mode. In 'Compatibility' mode,
  use fixed addresses. In Native-PCI mode, get base addresses from BARs in
  the PCI IDE controller's Configuration Space.

Arguments:
  PciIo             - Pointer to the EFI_PCI_IO_PROTOCOL instance
  IdeRegsBaseAddr   - Pointer to IDE_REGISTERS_BASE_ADDR to
                      receive IDE IO port registers' base addresses

Returns:

  EFI_STATUS

--*/
;

VOID
InitAtapiIoPortRegisters (
  IN  ATAPI_SCSI_PASS_THRU_DEV     *AtapiScsiPrivate,
  IN  IDE_REGISTERS_BASE_ADDR      *IdeRegsBaseAddr
  )
/*++

Routine Description:

  Initialize each Channel's Base Address of CommandBlock and ControlBlock.

Arguments:

  AtapiScsiPrivate            - The pointer of ATAPI_SCSI_PASS_THRU_DEV
  IdeRegsBaseAddr             - The pointer of IDE_REGISTERS_BASE_ADDR

Returns:

  None

--*/
;

#endif
