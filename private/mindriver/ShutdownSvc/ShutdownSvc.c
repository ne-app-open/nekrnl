// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <DriverKit/DriverKit.h>

/// @note Output file name is shutdowndrv.exe

DDK_EXTERN void KDriverMain(void) {
  if (ke_call_dispatch("_HalShutdownComputerNow", 0, NULL, 0)) {
    return;
  }

  while (YES);
}
