// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifndef SWAPKIT_DISKSWAP_H
#define SWAPKIT_DISKSWAP_H

#include <CompilerKit/CompilerKit.h>
#include <NeKit/Config.h>
#include <hint/CompilerHint.h>

#define kSwapPageFilePath "mnt/c/Pages/pagefile.sys"
#define kSwapPageFilePathU8 u8"mnt/c/Pages/pagefile.sys"

/// @file DiskSwap.h
/// @brief Virtual memory swap disk.

namespace Ne::Kernel {

struct SwapDiskHdr;
class IDiskSwap;

/// @brief Virtual memory interface to swap memory chunks onto disk.
/// @note The class only supports the NeFS as of right now, as it is using forks to write data into
/// disk.
class IDiskSwap final {
 public:
  IDiskSwap()  = default;
  ~IDiskSwap() = default;

  NE_COPY_DEFAULT(IDiskSwap)

 public:
  /***********************************************************************************/
  /// @brief Write memory chunk onto disk.
  /// @param data the data packet.
  /// @return Number of bytes written to swap.
  /***********************************************************************************/
  _Output Int64 Write(SwapDiskHdr* data, const Char* name = kSwapPageFilePath);

  /***********************************************************************************/
  /// @brief Read memory chunk from disk.
  /// @param data the data packet length.
  /// @return Whether the swap was fetched to disk, or not.
  /***********************************************************************************/
  _Output SwapDiskHdr* Read(const UIntPtr& offset, SizeT data_len, const Char* name = kSwapPageFilePath);
};

/// @brief Swap disk header, containing information about the held virtual memory.
/// @param fMagic Ident number.
/// @param fHeaderSz This header size.
/// @param fTeamID Process Team ID.
/// @param fProcessID Process ID.
/// @param fVirtualAddress Virtual address pointed by data.
/// @param fBlobSz Blob's size.
/// @param fBlob Data blob.
typedef struct SwapDiskHdr final {
  UInt32  fMagic;
  SizeT   fHeaderSz;
  UInt64  fTeamID;
  UInt64  fProcessID;
  UIntPtr fOffset;
  UInt64  fVirtualAddress;
  SizeT   fBlobSz;
  UInt8   fBlob[1];
} PACKED ALIGN(8) SwapDiskHdr, *SwapDiskHdrPtr;

}  // namespace Ne::Kernel

#endif
