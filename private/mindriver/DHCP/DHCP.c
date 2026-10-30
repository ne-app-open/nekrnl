// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <SystemKit/System.h>
#include <modules/DHCP/DHCP.h>

/// @brief NeAnt and Ant DHCP driver.
/// @note This driver is used to only connect using a DHCP host behind the workstation. To avoid
/// nightmare scenarios that could have been avoided.

/// @note Output file name is dhcp.exe

static struct DHCP_PRIV_PART_HDR* kDhcpHdr = NULL;

DDK_EXTERN Void KDriverMain(Void) {
  if (kDhcpHdr) return;
  
  kDhcpHdr = (struct DHCP_PRIV_PART_HDR*)kalloc(sizeof(struct DHCP_PRIV_PART_HDR));
  
  MUST_PASS(kDhcpHdr);

  if (kDhcpHdr)
    MmFillMemory(kDhcpHdr, sizeof(struct DHCP_PRIV_PART_HDR), 0);
}

DDK_EXTERN Void KDriverExit(Void) {
  if (!kDhcpHdr) return;

  kfree(kDhcpHdr);
  kDhcpHdr = NULL;

  while (YES)
    ke_call_dispatch("_HalFiniDriverRuntime", 0, NULL, 0);
}

// END
