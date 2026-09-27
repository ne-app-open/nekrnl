// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <DriverKit/DriverKit.h>

/// @brief The variable that checks whether we installed CDFS (yet)
/// @note not thread safe
static int kHalCDFSInstalled = 0;

DDK_EXTERN int _HalIsCdfsDrvInstalled(void) {
  return kHalCDFSInstalled == 1;
}

DDK_EXTERN void _HalCdfsDrvInstall(void) {
  if (kHalCDFSInstalled) return;
  kHalCDFSInstalled = 1;
}
