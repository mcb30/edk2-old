/** @file
*
*  Copyright (c) 2011, ARM Limited. All rights reserved.
*  
*  This program and the accompanying materials                          
*  are licensed and made available under the terms and conditions of the BSD License         
*  which accompanies this distribution.  The full text of the license may be found at        
*  http://opensource.org/licenses/bsd-license.php                                            
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
*
**/

#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/ArmTrustZoneLib.h>

#define TZPC_DECPROT0_STATUS_REG        0x800
#define TZPC_DECPROT0_SET_REG           0x804
#define TZPC_DECPROT0_CLEAR_REG         0x808

#define TZASC_CONFIGURATION_REG         0x000
#define TZASC_REGIONS_REG               0x100
#define TZASC_REGION0_LOW_ADDRESS_REG   0x100
#define TZASC_REGION0_HIGH_ADDRESS_REG  0x104
#define TZASC_REGION0_ATTRIBUTES        0x108

/**
    FIXME: Need documentation
**/
EFI_STATUS TZPCSetDecProtBits(UINTN TzpcBase, UINTN TzpcId, UINTN Bits) {
    if (TzpcId > TZPC_DECPROT_MAX) {
        return EFI_INVALID_PARAMETER;
    }

    MmioWrite32((UINTN)TzpcBase + TZPC_DECPROT0_SET_REG + (TzpcId * 0x0C), Bits);

    return EFI_SUCCESS;
}

/**
    FIXME: Need documentation
**/
EFI_STATUS TZPCClearDecProtBits(UINTN TzpcBase, UINTN TzpcId, UINTN Bits) {
    if (TzpcId> TZPC_DECPROT_MAX) {
        return EFI_INVALID_PARAMETER;
    }

    MmioWrite32((UINTN)TzpcBase + TZPC_DECPROT0_CLEAR_REG + (TzpcId * 0x0C), Bits);

    return EFI_SUCCESS;
}

/**
    FIXME: Need documentation
**/
UINT32 TZASCGetNumRegions(UINTN TzascBase) {
    return (MmioRead32((UINTN)TzascBase + TZASC_CONFIGURATION_REG) & 0xF);
}

/**
    FIXME: Need documentation
**/
EFI_STATUS TZASCSetRegion(UINTN TzascBase, UINTN RegionId, UINTN Enabled, UINTN LowAddress, UINTN HighAddress, UINTN Size, UINTN Security) {
    UINT32*     Region;

    if (RegionId > TZASCGetNumRegions(TzascBase)) {
        return EFI_INVALID_PARAMETER;
    }

    Region = (UINT32*)((UINTN)TzascBase + TZASC_REGIONS_REG + (RegionId * 0x10));

    MmioWrite32((UINTN)(Region), LowAddress&0xFFFF8000);
\s\sMmioWrite32((UINTN)(Region+1), HighAddress);
\s\sMmioWrite32((UINTN)(Region+2), ((Security & 0xF) <<28) | ((Size & 0x3F) << 1) | (Enabled & 0x1));

    return EFI_SUCCESS;
}
