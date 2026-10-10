// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-open/nekrnl

#include <NeKit/NeKit.h>

#ifndef IMPORT_C
#define IMPORT_C extern "C"
#endif

using namespace ::Ne::Kernel;

/// @brief The variable that checks whether we installed CDFS (yet)
/// @note not thread safe
STATIC ATTRIBUTE(unused)
Int32  kHalCDFSInstalled = NO;

IMPORT_C Int32 _HalIsCdfsDrvInstalled(Void) {
  return kHalCDFSInstalled == YES;
}

IMPORT_C Int32 _HalCdfsDrvInstall(Void) {
  if (kHalCDFSInstalled) return NO;
  
  kHalCDFSInstalled = YES;

  return kHalCDFSInstalled == YES;
}
