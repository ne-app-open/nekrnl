// SPDX-License-Identifier: Apache-2.0
// Copyright 2026, Ne.app
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#pragma once

#include <SystemKit/System.h>
#include <DriverKit/DriverKit.h>

/// @brief NeAnt's CDFS-FileSystem (CDFS-FS)

#ifndef kNeCdfsPartLen
#define kNeCdfsPartLen (128U)
#endif

/// @brief Partition Header on the compact disc structure.
struct CDFS_PRIV_PART_HDR _FINAL {
  Char fPartName[kNeCdfsPartLen];
  SInt16 fPartType;
  SInt16 fPartFlags;
  SInt32 fStartLba;
  SInt32 fEndLba;
  SInt16 fMediaSize;
  SInt16 fMediaSectorSize;
};

// END
