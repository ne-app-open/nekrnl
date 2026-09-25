// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <SystemKit/System.h>
#include <modules/CDFS/CDFS.h>
#include <DriverKit/DriverKit.h>

#ifndef kNePartLen
#define kNePartLen (128U)
#endif

/// @brief ANT and NeAnt CDFS driver.
/// @note Uses the Shims framework for that matter.

/// AMLALE: This is driver related, of course CDFS specs are different.
struct CDFS_PRIV_HDR {
  Char fPartName[kNePartLen];
  SInt16 fPartType;
  SInt32 fStartLba;
  SInt32 fEndLba;
  SInt16 fMediaSize;
  SInt16 fMediaSectorSize;
};

/// @note Output file name is cdfs.exe

DDK_EXTERN void KDriverMain(void) {
  if (ke_call_dispatch("_HalIsCdfsDrvInstalled", 0, NULL, 0)) {
    return;
  }

  ke_call_dispatch("_HalCdfsDrvInstall", 0, NULL, 0);

  while (YES) {
    
  }
}
