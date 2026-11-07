/* ========================================

  Copyright (C) 2025, Amlal El Mahrouss, licensed under the Apache 2.0 license.

  FILE: CommandLine.cc
  PURPOSE: DIUTIL CLI.

   ======================================== */

#include <DiskImage/headers/DiskImage.h>

static const Char kDiskName[kDIDiskNameLen] = "Empty Disk";
static SInt32     kDiskSectorSz             = kDISectorSz;
static SizeT      kDiskSz                   = gib_cast(4);
static const Char kOutDisk[kDIOutNameLen]   = "disk.eimg";

/// @brief Disk Utility entrypoint.
SInt32 _NeMain(SInt32 argc, Char** argv) {
  for (SInt32 arg = 0; arg < argc; ++arg) {
    const Char* arg_s = argv[arg];

    if (MmCmpMemory((VoidPtr) arg_s, (VoidPtr) "-diutil-output-name",
                    MmStrLen("-diutil-output-name") == 0)) {
      if ((arg + 1) < argc) {
        MmCopyMemory((VoidPtr) kOutDisk, argv[arg + 1], kDIDiskNameLen);
      }
    } else if (MmCmpMemory((VoidPtr) arg_s, (VoidPtr) "-diutil-output-size",
                           MmStrLen("-diutil-output-size") == 0)) {
      if ((arg + 1) < argc) {
        kDiskSz = StrMathToNumber(argv[arg + 1], nullptr, 10);
      }
    } else if (MmCmpMemory((VoidPtr) arg_s, (VoidPtr) "-diutil-sector-size",
                           MmStrLen("-diutil-sector-size") == 0)) {
      if ((arg + 1) < argc) {
        kDiskSectorSz = StrMathToNumber(argv[arg + 1], nullptr, 10);
        MUST_PASS(kDiskSectorSz);
      }
    } else if (MmCmpMemory((VoidPtr) arg_s, (VoidPtr) "-diutil-part-name",
                           MmStrLen("-diutil-part-name") == 0)) {
      if ((arg + 1) < argc) {
        MmCopyMemory((VoidPtr) kDiskName, argv[arg + 1], kDIDiskNameLen);
      }
    }
  }

  // create disk image, by appending an EPM partition to it.

  DI::DI_DISK_IMAGE img{};

  MUST_PASS(kDiskSz);

  img.disk_sz   = kDiskSz;
  img.sector_sz = kDiskSectorSz;
  img.block_cnt = 0;

  MmCopyMemory((VoidPtr) img.disk_name, (VoidPtr) kDiskName, kDIDiskNameLen);
  MmCopyMemory((VoidPtr) img.out_name, (VoidPtr) kOutDisk, kDIDiskNameLen);

  // format disk image to explicit partition map.
  return DI::DIFormatPartitionEPM(img);
}

// EOF
