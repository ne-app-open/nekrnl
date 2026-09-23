// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifndef FIRMWAREKIT_HANDOVER_H
#define FIRMWAREKIT_HANDOVER_H

#include <FirmwareKit/EFI/EFI.h>
#include <NeKit/Config.h>

#define kHandoverMagic (0xBADAA)
#define kHandoverVersion (0x0120)

/* Initial bitmap pointer location and size. */
#define kHandoverStructSz sizeof(HEL::BootInfoHeader)

namespace Ne::Kernel::HEL {
/**
@brief The executable type enum.
*/
enum {
  kTypeKernel       = 100,
  kTypeKernelDriver = 101,
  kTypeRsrc         = 102,
  kTypeDLL          = 103,
  kTypeInvalid      = 104,
  kTypeCount        = 4,

};

/**
@brief The executable architecture enum.
*/

enum {
  kArchAMD64 = 122,
  kArchARM64 = 123,
  kArchRISCV = 124,
  kArchCount = 3,
};

struct BootInfoHeader final {
  UInt64 f_Magic;
  UInt64 f_Version;

  Bool f_RecoverMode;

  VoidPtr f_BitMapStart;
  SizeT   f_BitMapSize;

  VoidPtr f_PageStart;

  VoidPtr f_HALImage;
  SizeT   f_HALSz;

  VoidPtr f_KernelImage;
  SizeT   f_KernelSz;

  VoidPtr f_SCIImage;
  SizeT   f_SCIImageSz;

  VoidPtr f_HostImage;
  SizeT   f_HostImageSz;

  VoidPtr f_CCImage;
  SizeT   f_CCImageSz;

  VoidPtr f_StackTop;
  SizeT   f_StackSz;

  WideChar f_FirmwareVendorName[32];
  SizeT    f_FirmwareVendorLen;

  VoidPtr f_FirmwareCustomTables[2];  // On EFI 0: BS 1: ST

  struct {
    VoidPtr      f_SmBios;
    VoidPtr      f_VendorPtr;
    VoidPtr      f_MpPtr;
    Bool         f_MultiProcessingEnabled;
    UIntPtr      f_ImageKey;
    EfiHandlePtr f_ImageHandle;
  } f_HardwareTables;

  struct {
    UIntPtr f_The;
    SizeT   f_Size;
    UInt32  f_Width;
    UInt32  f_Height;
    UInt32  f_PixelFormat;
    UInt32  f_PixelPerLine;
  } f_GOP;

  UInt64 f_NumberOfProcessors;
  UInt64 f_FirmwareSpecific[7];
};

enum {
  kHandoverTableBS,
  kHandoverTableST,
  kHandoverTableCount,
};

/// @brief Alias of bootloader main type.
typedef Int32 (*HandoverProc)(BootInfoHeader* boot_info);
}  // namespace Ne::Kernel::HEL

/// @brief Bootloader information header global variable.
inline Ne::Kernel::HEL::BootInfoHeader* kHandoverHeader = nullptr;

#endif
