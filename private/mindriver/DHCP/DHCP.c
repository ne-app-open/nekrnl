// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <SystemKit/System.h>
#include <modules/DHCP/DHCP.h>

/// @brief NeAnt and Ant DHCP driver.
/// @note This driver is used to only connect using a DHCP host behind the workstation. To avoid
/// nightmare scenarios that could have been avoided.

/// @brief Implemented in driver to implement driver.
struct DHCP_PRIV_CONFIG_HDR;
struct DHCP_PRIV_HDR;

/// @note Output file name is DHCP.sys

DDK_EXTERN Void KDriverMain(Void) {
  return;
}
