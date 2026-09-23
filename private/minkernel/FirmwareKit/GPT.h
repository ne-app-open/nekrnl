// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifndef FIRMWAREKIT_GPT_H
#define FIRMWAREKIT_GPT_H

#include <FirmwareKit/EFI/EFI.h>
#include <NeKit/Config.h>

#define kSectorAlignGPT_PartTbl (420U)
#define kSectorAlignGPT_PartEntry (72U)
#define kMagicLenGPT (8U)
#define kMagicGPT ("EFI PART")  // "EFI PART"
#define kGPTPartitionTableLBA (512 + sizeof(GPT_PARTITION_TABLE))

namespace Ne::Kernel {

struct GPT_PARTITION_TABLE;
struct GPT_PARTITION_ENTRY;
struct _MBR_PARTITION_RECORD;
struct _MASTER_BOOT_RECORD;

typedef struct _MBR_PARTITION_RECORD final {
  UInt8 BootIndicator;
  UInt8 StartHead;
  UInt8 StartSector;
  UInt8 StartTrack;
  UInt8 OSIndicator;
  UInt8 EndHead;
  UInt8 EndSector;
  UInt8 EndTrack;
  UInt8 StartingLBA[4];
  UInt8 SizeInLBA[4];
} PACKED MBR_PARTITION_RECORD;

typedef struct _MASTER_BOOT_RECORD final {
  UInt8                BootStrapCode[440];
  UInt8                UniqueMbrSignature[4];
  UInt8                Unknown[2];
  MBR_PARTITION_RECORD Partition[4];
  UInt16               Signature;
} PACKED MASTER_BOOT_RECORD;

struct PACKED GPT_PARTITION_TABLE final {
  Char     Signature[kMagicLenGPT];
  UInt32   Revision;
  UInt32   HeaderSize;
  UInt32   CRC32;
  UInt32   Reserved1;
  UInt64   LBAHeader;
  UInt64   LBAAltHeader;
  UInt64   FirstGPTEntry;
  UInt64   LastGPTEntry;
  EFI_GUID Guid;
  UInt64   StartingLBA;
  UInt32   NumPartitionEntries;
  UInt32   SizeOfEntries;
  UInt32   CRC32PartEntry;
  UInt8    Reserved2[kSectorAlignGPT_PartTbl];
};

struct PACKED GPT_PARTITION_ENTRY final {
  EFI_GUID PartitionTypeGUID;
  EFI_GUID UniquePartitionGUID;
  UInt64   StartLBA;
  UInt64   EndLBA;
  UInt64   Attributes;
  UInt8    Name[kSectorAlignGPT_PartEntry];
};

}  // namespace Ne::Kernel

#endif
