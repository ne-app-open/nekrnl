// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <KernelKit/DriveMgr.h>
#include <KernelKit/FileMgr.h>

/*************************************************************
 *
 * File: \file IFS.cpp
 * Purpose: \brief Filesystem to mountpoint interface.
 * Date: \date 05/26/2025
 *
 * Copyright (C) 2024-2025, Amlal El Mahrouss, licensed under the Apache 2.0 license.
 *
 *************************************************************/

/// Useful macros regarding the IFS.

#define fsi_ifs_write(DRV, TRAITS, MP) (MP->DRV()).fOutput(TRAITS)
#define fsi_ifs_read(DRV, TRAITS, MP) (MP->DRV()).fInput(TRAITS)

namespace Ne::Kernel {
/// @brief Read from fs disk.
/// @param Mnt mounted interface.
/// @param DrvTrait drive info
/// @param DrvIndex drive index.
/// @return KPC status code from the IFS.
Int32 fs_ifs_read(IMountpoint* Mnt, DriveTrait& DrvTrait, Int32 DrvIndex) {
  if (!Mnt) return kErrorDisk;
  if (DrvIndex >= IMountpoint::kDriveIndexInvalid) return kErrorDisk;

  DrvTrait.fPacket.fPacketGood = false;

  switch (DrvIndex) {
    case IMountpoint::kDriveIndexA: {
      fsi_ifs_read(A, DrvTrait.fPacket, Mnt);
      break;
    }
    case IMountpoint::kDriveIndexB: {
      fsi_ifs_read(B, DrvTrait.fPacket, Mnt);
      break;
    }
    case IMountpoint::kDriveIndexC: {
      fsi_ifs_read(C, DrvTrait.fPacket, Mnt);
      break;
    }
    case IMountpoint::kDriveIndexD: {
      fsi_ifs_read(D, DrvTrait.fPacket, Mnt);
      break;
    }
  }

  return DrvTrait.fPacket.fPacketGood ? kErrorSuccess : kErrorDisk;
}

/// @brief Write to fs disk.
/// @param Mnt mounted interface.
/// @param DrvTrait drive info
/// @param DrvIndex drive index.
/// @return
Int32 fs_ifs_write(IMountpoint* Mnt, DriveTrait& DrvTrait, Int32 DrvIndex) {
  if (!Mnt) return kErrorDisk;
  if (DrvIndex >= IMountpoint::kDriveIndexInvalid) return kErrorDisk;

  DrvTrait.fPacket.fPacketGood = false;

  switch (DrvIndex) {
    case IMountpoint::kDriveIndexA: {
      fsi_ifs_write(A, DrvTrait.fPacket, Mnt);
      break;
    }
    case IMountpoint::kDriveIndexB: {
      fsi_ifs_write(B, DrvTrait.fPacket, Mnt);
      break;
    }
    case IMountpoint::kDriveIndexC: {
      fsi_ifs_write(C, DrvTrait.fPacket, Mnt);
      break;
    }
    case IMountpoint::kDriveIndexD: {
      fsi_ifs_write(D, DrvTrait.fPacket, Mnt);
      break;
    }
  }

  return DrvTrait.fPacket.fPacketGood ? kErrorSuccess : kErrorDisk;
}
}  // namespace Ne::Kernel