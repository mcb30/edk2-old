/** @file
  Produces the SMM CPU I/O Protocol.

Copyright (c) 2009 - 2010, Intel Corporation
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#include <PiSmm.h>

#include <Protocol/SmmCpuIo2.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/BaseMemoryLib.h>

#define MAX_IO_PORT_ADDRESS   0xFFFF

//
// Function Prototypes
//
EFI_STATUS
EFIAPI
CpuMemoryServiceRead (
  IN  CONST EFI_SMM_CPU_IO2_PROTOCOL  *This,
  IN  EFI_SMM_IO_WIDTH                Width,
  IN  UINT64                          Address,
  IN  UINTN                           Count,
  OUT VOID                            *Buffer
  );

EFI_STATUS
EFIAPI
CpuMemoryServiceWrite (
  IN CONST EFI_SMM_CPU_IO2_PROTOCOL  *This,
  IN EFI_SMM_IO_WIDTH                Width,
  IN UINT64                          Address,
  IN UINTN                           Count,
  IN VOID                            *Buffer
  );

EFI_STATUS
EFIAPI
CpuIoServiceRead (
  IN  CONST EFI_SMM_CPU_IO2_PROTOCOL  *This,
  IN  EFI_SMM_IO_WIDTH                Width,
  IN  UINT64                          Address,
  IN  UINTN                           Count,
  OUT VOID                            *Buffer
  );

EFI_STATUS
EFIAPI
CpuIoServiceWrite (
  IN CONST EFI_SMM_CPU_IO2_PROTOCOL  *This,
  IN EFI_SMM_IO_WIDTH                Width,
  IN UINT64                          Address,
  IN UINTN                           Count,
  IN VOID                            *Buffer
  );

//
// Handle for the SMM CPU I/O Protocol
//
EFI_HANDLE  mHandle = NULL;

//
// SMM CPU I/O Protocol instance
//
EFI_SMM_CPU_IO2_PROTOCOL mSmmCpuIo2 = {
  {
    CpuMemoryServiceRead,
    CpuMemoryServiceWrite
  },
  {
    CpuIoServiceRead,
    CpuIoServiceWrite
  }
};

//
// Lookup table for increment values based on transfer widths
//
UINT8 mStride[] = {
  1, // SMM_IO_UINT8
  2, // SMM_IO_UINT16
  4, // SMM_IO_UINT32
  8  // SMM_IO_UINT64
};

/**
  Check parameters to a SMM CPU I/O Protocol service request.

  @param  Width                 Signifies the width of the I/O or Memory operation.
  @param  Address               The base address of the I/O or Memory operation.
  @param  Count                 The number of I/O or Memory operations to perform.
                                The number of bytes moved is Width size * Count, starting at Address.
  @param  Buffer                For read operations, the destination buffer to store the results.
                                For write operations, the source buffer from which to write data.
  @param  MmioOperation         TRUE for an MMIO operation, FALSE for I/O Port operation.
  
  @retval EFI_SUCCESS           The parameters for this request pass the checks.
  @retval EFI_INVALID_PARAMETER Buffer is NULL or Width is not valid.
  @retval EFI_UNSUPPORTED       The address range specified by Address, Width, and Count exceeds Limit.
                                The Buffer is not aligned for the given Width.

**/
EFI_STATUS
CpuIoCheckParameter (
  IN BOOLEAN           MmioOperation,
  IN EFI_SMM_IO_WIDTH  Width,
  IN UINT64            Address,
  IN UINTN             Count,
  IN VOID              *Buffer
  )
{
  UINT64  MaxCount;
  UINT64  Limit;

  //
  // Check to see if Buffer is NULL
  //
  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check to see if Width is in the valid range
  //
  if (Width < 0 || Width > SMM_IO_UINT64) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check to see if Width is in the valid range for I/O Port operations
  //
  if (!MmioOperation && (Width == SMM_IO_UINT64)) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Check to see if any address associated with this transfer exceeds the maximum 
  // allowed address.  The maximum address implied by the parameters passed in is
  // Address + Size * Count.  If the following condition is met, then the transfer
  // is not supported.
  //
  //    Address + Size * Count > (MmioOperation ? MAX_ADDRESS : MAX_IO_PORT_ADDRESS) + 1
  //
  // Since MAX_ADDRESS can be the maximum integer value supported by the CPU and Count 
  // can also be the maximum integer value supported by the CPU, this range
  // check must be adjusted to avoid all oveflow conditions.
  //   
  // The follwing form of the range check is equivalent but assumes that 
  // MAX_ADDRESS and MAX_IO_PORT_ADDRESS are of the form (2^n - 1).
  //
  Limit = (MmioOperation ? MAX_ADDRESS : MAX_IO_PORT_ADDRESS);
  if (Count == 0) {
    if (Address > Limit) {
      return EFI_UNSUPPORTED;
    }
  } else {  
    MaxCount = RShiftU64 (Limit, Width);
    if (MaxCount < (Count - 1)) {
      return EFI_UNSUPPORTED;
    }
    if (Address > LShiftU64 (MaxCount - Count + 1, Width)) {
      return EFI_UNSUPPORTED;
    }
  }
  
  //
  // Check to see if Address is aligned
  //
  if ((Address & (UINT64)(mStride[Width] - 1)) != 0) {
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

/**
  Reads memory-mapped registers.

  @param  This                  A pointer to the EFI_SMM_CPU_IO2_PROTOCOL instance.
  @param  Width                 Signifies the width of the I/O or Memory operation.
  @param  Address               The base address of the I/O or Memoryoperation.
  @param  Count                 The number of I/O or Memory operations to perform.
                                The number of bytes moved is Width size * Count, starting at Address.
  @param  Buffer                For read operations, the destination buffer to store the results.
                                For write operations, the source buffer from which to write data.

  @retval EFI_SUCCESS           The data was read from or written to the EFI system.
  @retval EFI_INVALID_PARAMETER Width is invalid for this EFI system.Or Buffer is NULL.
  @retval EFI_UNSUPPORTED       The Buffer is not aligned for the given Width.
                                Or,The address range specified by Address, Width, and Count is not valid for this EFI system.

**/
EFI_STATUS
EFIAPI
CpuMemoryServiceRead (
  IN  CONST EFI_SMM_CPU_IO2_PROTOCOL  *This,
  IN  EFI_SMM_IO_WIDTH                Width,
  IN  UINT64                          Address,
  IN  UINTN                           Count,
  OUT VOID                            *Buffer
  )
{
  EFI_STATUS  Status;
  UINT8       Stride;
  UINT8       *Uint8Buffer;

  Status = CpuIoCheckParameter (TRUE, Width, Address, Count, Buffer);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Select loop based on the width of the transfer
  //
  Stride = mStride[Width];
  for (Uint8Buffer = Buffer; Count > 0; Address += Stride, Uint8Buffer += Stride, Count--) {
    if (Width == SMM_IO_UINT8) {
      *Uint8Buffer = MmioRead8 ((UINTN)Address);
    } else if (Width == SMM_IO_UINT16) {
      *((UINT16 *)Uint8Buffer) = MmioRead16 ((UINTN)Address);
    } else if (Width == SMM_IO_UINT32) {
      *((UINT32 *)Uint8Buffer) = MmioRead32 ((UINTN)Address);
    } else if (Width == SMM_IO_UINT64) {
      *((UINT64 *)Uint8Buffer) = MmioRead64 ((UINTN)Address);
    }
  }
  return EFI_SUCCESS;
}

/**
  Writes memory-mapped registers.

  @param  This                  A pointer to the EFI_SMM_CPU_IO2_PROTOCOL instance.
  @param  Width                 Signifies the width of the I/O or Memory operation.
  @param  Address               The base address of the I/O or Memoryoperation.
  @param  Count                 The number of I/O or Memory operations to perform.
                                The number of bytes moved is Width size * Count, starting at Address.
  @param  Buffer                For read operations, the destination buffer to store the results.
                                For write operations, the source buffer from which to write data.

  @retval EFI_SUCCESS           The data was read from or written to the EFI system.
  @retval EFI_INVALID_PARAMETER Width is invalid for this EFI system.Or Buffer is NULL.
  @retval EFI_UNSUPPORTED       The Buffer is not aligned for the given Width.
                                Or,The address range specified by Address, Width, and Count is not valid for this EFI system.

**/
EFI_STATUS
EFIAPI
CpuMemoryServiceWrite (
  IN CONST EFI_SMM_CPU_IO2_PROTOCOL  *This,
  IN EFI_SMM_IO_WIDTH                Width,
  IN UINT64                          Address,
  IN UINTN                           Count,
  IN VOID                            *Buffer
  )
{
  EFI_STATUS  Status;
  UINT8       Stride;
  UINT8       *Uint8Buffer;

  Status = CpuIoCheckParameter (TRUE, Width, Address, Count, Buffer);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Select loop based on the width of the transfer
  //
  Stride = mStride[Width];
  for (Uint8Buffer = Buffer; Count > 0; Address += Stride, Uint8Buffer += Stride, Count--) {
    if (Width == SMM_IO_UINT8) {
      MmioWrite8 ((UINTN)Address, *Uint8Buffer);
    } else if (Width == SMM_IO_UINT16) {
      MmioWrite16 ((UINTN)Address, *((UINT16 *)Uint8Buffer));
    } else if (Width == SMM_IO_UINT32) {
      MmioWrite32 ((UINTN)Address, *((UINT32 *)Uint8Buffer));
    } else if (Width == SMM_IO_UINT64) {
      MmioWrite64 ((UINTN)Address, *((UINT64 *)Uint8Buffer));
    }
  }
  return EFI_SUCCESS;
}

/**
  Reads I/O registers.

  @param  This                  A pointer to the EFI_SMM_CPU_IO2_PROTOCOL instance.
  @param  Width                 Signifies the width of the I/O or Memory operation.
  @param  Address               The base address of the I/O or Memoryoperation.
  @param  Count                 The number of I/O or Memory operations to perform.
                                The number of bytes moved is Width size * Count, starting at Address.
  @param  Buffer                For read operations, the destination buffer to store the results.
                                For write operations, the source buffer from which to write data.

  @retval EFI_SUCCESS           The data was read from or written to the EFI system.
  @retval EFI_INVALID_PARAMETER Width is invalid for this EFI system.Or Buffer is NULL.
  @retval EFI_UNSUPPORTED       The Buffer is not aligned for the given Width.
                                Or,The address range specified by Address, Width, and Count is not valid for this EFI system.

**/
EFI_STATUS
EFIAPI
CpuIoServiceRead (
  IN  CONST EFI_SMM_CPU_IO2_PROTOCOL  *This,
  IN  EFI_SMM_IO_WIDTH                Width,
  IN  UINT64                          Address,
  IN  UINTN                           Count,
  OUT VOID                            *Buffer
  )
{
  EFI_STATUS  Status;
  UINT8       Stride;
  UINT8       *Uint8Buffer;

  Status = CpuIoCheckParameter (FALSE, Width, Address, Count, Buffer);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Select loop based on the width of the transfer
  //
  Stride = mStride[Width];
  for (Uint8Buffer = Buffer; Count > 0; Address += Stride, Uint8Buffer += Stride, Count--) {
    if (Width == SMM_IO_UINT8) {
      *Uint8Buffer = IoRead8 ((UINTN)Address);
    } else if (Width == SMM_IO_UINT16) {
      *((UINT16 *)Uint8Buffer) = IoRead16 ((UINTN)Address);
    } else if (Width == SMM_IO_UINT32) {
      *((UINT32 *)Uint8Buffer) = IoRead32 ((UINTN)Address);
    }
  }

  return EFI_SUCCESS;
}

/**
  Write I/O registers.

  @param  This                  A pointer to the EFI_SMM_CPU_IO2_PROTOCOL instance.
  @param  Width                 Signifies the width of the I/O or Memory operation.
  @param  Address               The base address of the I/O or Memoryoperation.
  @param  Count                 The number of I/O or Memory operations to perform.
                                The number of bytes moved is Width size * Count, starting at Address.
  @param  Buffer                For read operations, the destination buffer to store the results.
                                For write operations, the source buffer from which to write data.

  @retval EFI_SUCCESS           The data was read from or written to the EFI system.
  @retval EFI_INVALID_PARAMETER Width is invalid for this EFI system.Or Buffer is NULL.
  @retval EFI_UNSUPPORTED       The Buffer is not aligned for the given Width.
                                Or,The address range specified by Address, Width, and Count is not valid for this EFI system.

**/
EFI_STATUS
EFIAPI
CpuIoServiceWrite (
  IN CONST EFI_SMM_CPU_IO2_PROTOCOL  *This,
  IN EFI_SMM_IO_WIDTH                Width,
  IN UINT64                          Address,
  IN UINTN                           Count,
  IN VOID                            *Buffer
  )
{
  EFI_STATUS  Status;
  UINT8       Stride;
  UINT8       *Uint8Buffer;

  //
  // Make sure the parameters are valid
  //
  Status = CpuIoCheckParameter (FALSE, Width, Address, Count, Buffer);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Select loop based on the width of the transfer
  //
  Stride = mStride[Width];
  for (Uint8Buffer = (UINT8 *)Buffer; Count > 0; Address += Stride, Uint8Buffer += Stride, Count--) {
    if (Width == SMM_IO_UINT8) {
      IoWrite8 ((UINTN)Address, *Uint8Buffer);
    } else if (Width == SMM_IO_UINT16) {
      IoWrite16 ((UINTN)Address, *((UINT16 *)Uint8Buffer));
    } else if (Width == SMM_IO_UINT32) {
      IoWrite32 ((UINTN)Address, *((UINT32 *)Uint8Buffer));
    }
  }
  
  return EFI_SUCCESS;
}

/**
  The module Entry Point SmmCpuIoProtocol driver

  @param  ImageHandle    The firmware allocated handle for the EFI image.
  @param  SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS    The entry point is executed successfully.
  @retval Other          Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
SmmCpuIo2Initialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
 {
  EFI_STATUS  Status;

  //
  // Copy the SMM CPU I/O Protocol instance into the System Management System Table
  //
  CopyMem (&gSmst->SmmIo, &mSmmCpuIo2, sizeof (mSmmCpuIo2));

  //
  // Install the SMM CPU I/O Protocol into the SMM protocol database
  //
  Status = gSmst->SmmInstallProtocolInterface (
                    &mHandle,
                    &gEfiSmmCpuIo2ProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    &mSmmCpuIo2
                    );
  ASSERT_EFI_ERROR (Status);
  
  return Status;
}
