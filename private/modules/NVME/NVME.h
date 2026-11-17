// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifndef __MODULE_NVME_H__
#define __MODULE_NVME_H__

#include <NeKit/Config.h>

/// @file NVME.h
/// @brief Non Volatile Memory.

#define NE_ALIGN_NVME ATTRIBUTE(aligned(sizeof(Ne::Kernel::UInt32)))

namespace Ne::Kernel {
struct NE_ALIGN_NVME HAL_NVME_BAR_0 final {
  UInt32 fCapabilities;
  UInt32 fVersion;
  UInt32 fIntMaskSet;
  UInt32 fIntMaskClr;
  UInt32 fContrlConf;
  UInt32 fContrlStat;
  UInt32 fAdminQueueAttr;
  UInt32 fAdminSubmissionQueue;
  UInt32 fAdminCompletionQueue;
};

struct NE_ALIGN_NVME HAL_NVME_QUEUE final {
  UInt32 fOpcode;
  UInt32 fNSID;
  UInt32 fReserved[3];
  UInt32 fMetadataPtr[5];
  UInt32 fDataPtr[9];
  UInt32 CommandSpecific[15];
};

enum {
  kInvalidNVME               = 0xFF,
  kCreateCompletionQueueNVME = 0x05,
  kCreateSubmissionQueueNVME = 0x01,
  kIdentifyNVME              = 0x06,
  kReadNVME                  = 0x02,
  kWriteNVME                 = 0x01,
  kCountNVME                 = 5,
};

/// @brief Creates an admin command for a DMA operation.
template <Int32 Opcode>
inline Bool nvme_create_admin_command(HAL_NVME_QUEUE* entry, UInt32 nsid, UInt32 prpTransfer[3],
                                      UInt32 startingLba[2], UInt32 lowTransferBlocks) {
  if (entry == nullptr) return false;

  entry->CommandSpecific[9]  = startingLba[0];
  entry->CommandSpecific[10] = startingLba[1];

  entry->CommandSpecific[11] = lowTransferBlocks;

  entry->CommandSpecific[5] = prpTransfer[0];
  entry->CommandSpecific[6] = prpTransfer[1];
  entry->CommandSpecific[7] = prpTransfer[2];

  entry->CommandSpecific[0] = nsid;

  return true;
}

/// @brief Creates an I/O command for a DMA operation.
template <Int32 Opcode>
inline Bool nvme_create_io_command(HAL_NVME_QUEUE* entry, UInt64 baseAddress,
                                   UInt32 identLoAndQueueSizeHi, UInt32 flagsLoAndQueueComplIdHi,
                                   UInt32 identify, Bool provideIdentify = false,
                                   Bool namespaceIdentify = false) {
  if (entry == nullptr) return false;
  if (baseAddress == 0) return false;
  if (identLoAndQueueSizeHi == 0) return false;
  if (identify == 0) return false;

  entry->fOpcode = Opcode;

  entry->CommandSpecific[5] = (baseAddress & 0xFF);
  entry->CommandSpecific[6] = static_cast<UInt32>(baseAddress);

  if (!provideIdentify) {
    entry->CommandSpecific[9]  = identLoAndQueueSizeHi;
    entry->CommandSpecific[10] = flagsLoAndQueueComplIdHi;
  } else {
    entry->CommandSpecific[9] = identify;

    if (namespaceIdentify) {
      entry->CommandSpecific[0] = YES;
    }
  }

  return true;
}
}  // namespace Ne::Kernel

#endif  // ifndef __MODULE_NVME_H__
