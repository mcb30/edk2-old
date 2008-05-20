/** @file

Copyright (c) 2008, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#ifndef _PEI_PCI_CFG_H_
#define _PEI_PCI_CFG_H_

#include <Ppi/PciCfg2.h>

#define ECP_PEI_PCI_CFG_PPI_GUID \
  { \
    {0xb0ee53d4, 0xa049, 0x4a79, { 0xb2, 0xff, 0x19, 0xd9, 0xfa, 0xef, 0xaa, 0x94 }} \
  }

typedef struct _ECP_PEI_PCI_CFG_PPI ECP_PEI_PCI_CFG_PPI;


typedef
EFI_STATUS
(EFIAPI *ECP_PEI_PCI_CFG_PPI_IO) (
  IN EFI_PEI_SERVICES         **PeiServices,
  IN ECP_PEI_PCI_CFG_PPI      * This,
  IN EFI_PEI_PCI_CFG_PPI_WIDTH Width,
  IN UINT64                   Address,
  IN OUT VOID                 *Buffer
  );

struct _ECP_PEI_PCI_CFG_PPI {
  ECP_PEI_PCI_CFG_PPI_IO  Read;
  ECP_PEI_PCI_CFG_PPI_IO  Write;
};

extern EFI_GUID gEcpPeiPciCfgPpiGuid;

#endif
