// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifdef __FSKIT_INCLUDES_NEFS__

#include <FSKit/NeFS.h>
#include <FirmwareKit/EPM.h>

#include <KernelKit/DriveMgr.h>
#include <KernelKit/IFS.h>
#include <KernelKit/KPC.h>
#include <KernelKit/ProcessScheduler.h>
#include <KernelKit/User.h>
#include <NeKit/Crc32.h>
#include <NeKit/KString.h>
#include <NeKit/KernelPanic.h>
#include <NeKit/Utils.h>
#include <modules/AHCI/AHCI.h>
#include <modules/ATA/ATA.h>

using namespace Ne::Kernel;

#ifdef __NE_NO_BUILTIN__
/***********************************************************************************/
/**
  Define those external symbols, to make the editor shutup
*/
/***********************************************************************************/

/***********************************************************************************/
/// @brief get sector count.
/***********************************************************************************/
Ne::Kernel::SizeT drv_std_get_sector_count();

/***********************************************************************************/
/// @brief get device size.
/***********************************************************************************/
Ne::Kernel::SizeT drv_std_get_size();

#endif

///! BUGS: 0

/***********************************************************************************/
/// This file implements the New extended File System.
///    New extended File System implements a flat linked-list based algorithm.
///        /
///    /Path1/        /Path2/
///  /readme.rtf        /ListContents.pef /readme.lnk <-- symlink.
///                                /Path1/readme.rtf
/***********************************************************************************/
static inline bool is_valid_size(SizeT size, SizeT max_size) {
  return size > 0 && size <= max_size;
}

static inline bool is_valid_lba(Lba lba, DriveTrait& drive) {
  NEFS_ROOT_PARTITION_BLOCK part_block;
  drive.fPacket.fPacketLba     = kNeFSRootCatalogStartAddress;
  drive.fPacket.fPacketSize    = sizeof(NEFS_ROOT_PARTITION_BLOCK);
  drive.fPacket.fPacketContent = reinterpret_cast<VoidPtr>(&part_block);
  drive.fInput(drive.fPacket);

  if (!drive.fPacket.fPacketGood) {
    return false;
  }

  // Compute the maximum LBA (DiskSize / sector size)
  SizeT sectorSize = drive.fSectorSz;
  Lba   maxLba     = part_block.DiskSize / sectorSize;

  return (lba >= part_block.StartCatalog) && (lba < maxLba);
}

STATIC IMountpoint kMountpoint;
/***********************************************************************************/
/// @brief Creates a new fork inside the New filesystem partition.
/// @param catalog it's catalog
/// @param the_fork the fork itself.
/// @return the fork
/***********************************************************************************/
_Output BOOL NeFileSystemParser::CreateFork(_Input NEFS_FORK_STRUCT& the_fork) {
  if (the_fork.ForkName[0] == 0 || the_fork.CatalogName[0] == 0 || the_fork.DataSize == 0) {
    return NO;
  }

  auto catalog = this->GetCatalog(the_fork.CatalogName);
  if (!catalog) return NO;

  Lba lba = catalog->DataFork;
  if (lba < kNeFSCatalogStartAddress) {
    delete catalog;
    return NO;
  }

  auto&            drv      = kMountpoint.A();
  Lba              lba_prev = lba;
  NEFS_FORK_STRUCT prev_fork{};
  NEFS_FORK_STRUCT cur_fork{};

  while (true) {
    drv.fPacket.fPacketLba     = lba;
    drv.fPacket.fPacketSize    = sizeof(NEFS_FORK_STRUCT);
    drv.fPacket.fPacketContent = reinterpret_cast<VoidPtr>(&cur_fork);
    drv.fInput(drv.fPacket);
    if (!drv.fPacket.fPacketGood) {
      delete catalog;
      return NO;
    }

    if (cur_fork.Flags & kNeFSFlagCreated) {
      if (KStringBuilder::Equals(cur_fork.ForkName, the_fork.ForkName) &&
          KStringBuilder::Equals(cur_fork.CatalogName, the_fork.CatalogName)) {
        break;
      }
      lba_prev  = lba;
      lba       = cur_fork.NextSibling;
      prev_fork = cur_fork;
    } else {
      if (lba >= kNeFSCatalogStartAddress) {
        prev_fork.NextSibling      = lba;
        drv.fPacket.fPacketLba     = lba_prev;
        drv.fPacket.fPacketSize    = sizeof(NEFS_FORK_STRUCT);
        drv.fPacket.fPacketContent = reinterpret_cast<VoidPtr>(&prev_fork);
        drv.fOutput(drv.fPacket);
      }
      break;
    }
  }

  SizeT sectorCount        = (the_fork.DataSize + kNeFSSectorSz - 1) / kNeFSSectorSz;
  the_fork.DataOffset      = lba + 1;
  the_fork.PreviousSibling = lba_prev;
  the_fork.NextSibling     = lba + 1 + sectorCount;
  the_fork.Flags |= kNeFSFlagCreated;

  drv.fPacket.fPacketLba     = lba;
  drv.fPacket.fPacketSize    = sizeof(NEFS_FORK_STRUCT);
  drv.fPacket.fPacketContent = reinterpret_cast<VoidPtr>(&the_fork);
  drv.fOutput(drv.fPacket);

  fs_ifs_write(&kMountpoint, drv, IMountpoint::kDriveIndexA);

  delete catalog;
  return YES;
}

/***********************************************************************************/
/// @brief Find fork inside filesystem.
/// @param catalog the catalog.
/// @param name the fork name.
/// @return the newly found fork.
/***********************************************************************************/
_Output NEFS_FORK_STRUCT* NeFileSystemParser::FindFork(_Input NEFS_CATALOG_STRUCT* catalog,
                                                       _Input const Char*          name,
                                                       _Input Boolean              is_data) {
  if (!catalog || !name) return nullptr;

  auto&            drive = kMountpoint.A();
  Lba              lba   = is_data ? catalog->DataFork : catalog->ResourceFork;
  NEFS_FORK_STRUCT local_buf{};

  while (lba >= kNeFSCatalogStartAddress) {
    drive.fPacket.fPacketLba     = lba;
    drive.fPacket.fPacketSize    = sizeof(NEFS_FORK_STRUCT);
    drive.fPacket.fPacketContent = reinterpret_cast<VoidPtr>(&local_buf);

    rt_copy_memory_safe((VoidPtr) "fs/nefs-packet", drive.fPacket.fPacketMime,
                        rt_string_len("fs/nefs-packet"), sizeof(drive.fPacket.fPacketMime));

    if (auto res = fs_ifs_read(&kMountpoint, drive, this->mDriveIndex); res) {
      switch (res) {
        case 1:
          err_global_get() = kErrorDiskReadOnly;
          break;
        case 2:
          err_global_get() = kErrorDiskIsFull;
          break;
        case 3:
          err_global_get() = kErrorNoSuchDisk;
          break;
        default:
          break;
      }
      return nullptr;
    }

    if (KStringBuilder::Equals(local_buf.ForkName, name) &&
        KStringBuilder::Equals(local_buf.CatalogName, catalog->Name)) {
      auto result = new NEFS_FORK_STRUCT();
      rt_copy_memory_safe(&local_buf, result, sizeof(NEFS_FORK_STRUCT), sizeof(NEFS_FORK_STRUCT));
      return result;
    }

    lba = local_buf.NextSibling;
  }

  return nullptr;
}

/***********************************************************************************/
/// @brief Simpler factory to create a catalog (assumes you want to create a
/// file.)
/// @param name
/// @return catalog pointer.
/***********************************************************************************/
_Output NEFS_CATALOG_STRUCT* NeFileSystemParser::CreateCatalog(_Input const Char* name) {
  return this->CreateCatalog(name, 0, kNeFSCatalogKindFile);
}

/***********************************************************************************/
/// @brief Creates a new catalog into the disk.
/// @param name the catalog name.
/// @param flags the flags of the catalog.
/// @param kind the catalog kind.
/// @return catalog pointer.
/***********************************************************************************/
_Output NEFS_CATALOG_STRUCT* NeFileSystemParser::CreateCatalog(_Input const Char*  name,
                                                               _Input const Int32& flags,
                                                               _Input const Int32& kind) {
  kout << "CreateCatalog(*...*)\n";

  if (!name) return nullptr;
  SizeT nameLen = rt_string_len(name);
  if (nameLen == 0) return nullptr;

  Lba out_lba = 0UL;

  kout << "Checking for path separator...\n";

  /// a directory should have a slash in the end.
  if (kind == kNeFSCatalogKindDir && name[nameLen - 1] != NeFileSystemHelper::Separator())
    return nullptr;

  /// a file shouldn't have a slash in the end.
  if (kind != kNeFSCatalogKindDir && name[nameLen - 1] == NeFileSystemHelper::Separator())
    return nullptr;

  NEFS_CATALOG_STRUCT* catalog_copy = this->FindCatalog(name, out_lba);

  if (catalog_copy) {
    kout << "Catalog already exists: " << name << ".\n";
    err_global_get() = kErrorFileExists;
    delete catalog_copy;
    catalog_copy = nullptr;
    return nullptr;
  }

  Char* parent_name = (Char*) mm_alloc_ptr(nameLen + 1, Yes, No);
  rt_copy_memory_safe(const_cast<Char*>(name), parent_name, nameLen + 1, nameLen + 1);

  if (nameLen < 2) {
    mm_free_ptr(parent_name);
    err_global_get() = kErrorFileNotFound;
    return nullptr;
  }

  SizeT index_reverse_copy = nameLen - 1;
  if (parent_name[index_reverse_copy] == NeFileSystemHelper::Separator()) {
    parent_name[index_reverse_copy] = 0;
    --index_reverse_copy;
  }
  while (index_reverse_copy > 0 &&
         parent_name[index_reverse_copy] != NeFileSystemHelper::Separator()) {
    parent_name[index_reverse_copy] = 0;
    --index_reverse_copy;
  }
  if (index_reverse_copy == 0 && parent_name[0] != NeFileSystemHelper::Separator()) {
    mm_free_ptr(parent_name);
    err_global_get() = kErrorFileNotFound;
    return nullptr;
  }

  NEFS_CATALOG_STRUCT* catalog = this->FindCatalog(parent_name, out_lba);
  mm_free_ptr(parent_name);

  auto& drive = kMountpoint.A();
  if (catalog && catalog->Kind == kNeFSCatalogKindFile) {
    kout << "Parent is a file.\n";
    delete catalog;
    return nullptr;
  } else if (!catalog) {
    Char part_block[sizeof(NEFS_ROOT_PARTITION_BLOCK)] = {0};
    drive.fPacket.fPacketContent                       = reinterpret_cast<VoidPtr>(part_block);
    drive.fPacket.fPacketSize                          = sizeof(NEFS_ROOT_PARTITION_BLOCK);
    drive.fPacket.fPacketLba                           = kNeFSRootCatalogStartAddress;
    drive.fInput(drive.fPacket);

    NEFS_ROOT_PARTITION_BLOCK* blk_nefs = (NEFS_ROOT_PARTITION_BLOCK*) part_block;
    out_lba                             = blk_nefs->StartCatalog;
  }

  if (drive.fPacket.fPacketReadOnly) {
    delete catalog;
    return nullptr;
  }

  NEFS_CATALOG_STRUCT* child_catalog = new NEFS_CATALOG_STRUCT();
  child_catalog->Checksum            = 0;
  child_catalog->ResourceForkSize    = 0UL;
  child_catalog->DataForkSize        = 0UL;
  child_catalog->CatalogFlags        = kNeFSStatusUnlocked;
  child_catalog->NextSibling         = out_lba;
  child_catalog->PrevSibling         = out_lba;
  child_catalog->Kind                = kind;
  child_catalog->Flags |= kNeFSFlagCreated;
  child_catalog->CatalogFlags = flags;

  SizeT i = nameLen;
  --i;
  if (kind == kNeFSCatalogKindDir) --i;
  while (name[i] != '/') --i;
  rt_copy_memory_safe((VoidPtr) (name + i), (VoidPtr) child_catalog->Name, rt_string_len(name) - i,
                      kNeFSCatalogNameLen);

  NEFS_CATALOG_STRUCT temporary_catalog{};
  Lba                 start_free = out_lba;

  rt_copy_memory_safe((VoidPtr) "fs/nefs-packet", drive.fPacket.fPacketMime,
                      rt_string_len("fs/nefs-packet"), sizeof(drive.fPacket.fPacketMime));

  Char buf_part_block[sizeof(NEFS_ROOT_PARTITION_BLOCK)] = {0};
  drive.fPacket.fPacketContent = reinterpret_cast<VoidPtr>(buf_part_block);
  drive.fPacket.fPacketSize    = sizeof(NEFS_ROOT_PARTITION_BLOCK);
  drive.fPacket.fPacketLba     = kNeFSRootCatalogStartAddress;
  drive.fInput(drive.fPacket);

  NEFS_ROOT_PARTITION_BLOCK* part_block = (NEFS_ROOT_PARTITION_BLOCK*) buf_part_block;
  drive.fPacket.fPacketContent          = reinterpret_cast<VoidPtr>(&temporary_catalog);
  drive.fPacket.fPacketSize             = sizeof(NEFS_CATALOG_STRUCT);
  drive.fPacket.fPacketLba              = start_free;
  drive.fInput(drive.fPacket);

  if (part_block->FreeCatalog < 1) {
    delete child_catalog;
    delete catalog;
    return nullptr;
  }

  kout << "Start finding catalog to allocate or empty space...\n";

  SizeT catalogSectors = (sizeof(NEFS_CATALOG_STRUCT) + drive.fSectorSz - 1) / drive.fSectorSz;
  while (start_free < part_block->StartCatalog + (part_block->CatalogCount * catalogSectors)) {
    drive.fPacket.fPacketContent = reinterpret_cast<VoidPtr>(&temporary_catalog);
    drive.fPacket.fPacketSize    = sizeof(NEFS_CATALOG_STRUCT);
    drive.fPacket.fPacketLba     = start_free;
    drive.fInput(drive.fPacket);

    if ((temporary_catalog.Flags & kNeFSFlagCreated) == 0) {
      child_catalog->NextSibling = start_free + catalogSectors;

      NEFS_CATALOG_STRUCT placeholder{};
      drive.fPacket.fPacketContent = reinterpret_cast<VoidPtr>(&placeholder);
      drive.fPacket.fPacketSize    = sizeof(NEFS_CATALOG_STRUCT);
      drive.fPacket.fPacketLba     = start_free;
      drive.fOutput(drive.fPacket);

      child_catalog->DataFork     = part_block->DiskSize - start_free;
      child_catalog->ResourceFork = child_catalog->DataFork;

      drive.fPacket.fPacketContent = reinterpret_cast<VoidPtr>(child_catalog);
      drive.fPacket.fPacketSize    = sizeof(NEFS_CATALOG_STRUCT);
      drive.fPacket.fPacketLba     = start_free;
      drive.fOutput(drive.fPacket);

      drive.fPacket.fPacketContent = reinterpret_cast<VoidPtr>(buf_part_block);
      drive.fPacket.fPacketSize    = sizeof(NEFS_ROOT_PARTITION_BLOCK);
      drive.fPacket.fPacketLba     = kNeFSRootCatalogStartAddress;
      drive.fInput(drive.fPacket);

      part_block->FreeSectors -= catalogSectors;
      part_block->CatalogCount += 1;
      part_block->FreeCatalog -= 1;

      drive.fPacket.fPacketContent = reinterpret_cast<VoidPtr>(part_block);
      drive.fPacket.fPacketSize    = sizeof(NEFS_ROOT_PARTITION_BLOCK);
      drive.fPacket.fPacketLba     = kNeFSRootCatalogStartAddress;
      drive.fOutput(drive.fPacket);

      delete catalog;
      NEFS_CATALOG_STRUCT* found_catalog = new NEFS_CATALOG_STRUCT();
      rt_copy_memory_safe(&temporary_catalog, found_catalog, sizeof(NEFS_CATALOG_STRUCT),
                          sizeof(NEFS_CATALOG_STRUCT));

      delete child_catalog;
      return found_catalog;
    } else if ((temporary_catalog.Flags & kNeFSFlagCreated) &&
               KStringBuilder::Equals(temporary_catalog.Name, name)) {
      rt_copy_memory_safe(&temporary_catalog, child_catalog, sizeof(NEFS_CATALOG_STRUCT),
                          sizeof(NEFS_CATALOG_STRUCT));
      delete catalog;
      return child_catalog;
    }

    start_free += catalogSectors;
  }

  delete child_catalog;
  delete catalog;
  return nullptr;
}

/***********************************************************************************/
/// @brief Make a EPM+NeFS drive out of the disk.
/// @param drive The drive to write on.
/// @return If it was sucessful, see err_global_get().
/***********************************************************************************/
bool NeFileSystemParser::Format(_Input _Output DriveTrait* drive, _Input const Int32 flags,
                                const Char* part_name) {
  if (!part_name || *part_name == 0) return false;
  NE_UNUSED(flags);

  // verify disk.
  drive->fVerify(drive->fPacket);
  rt_copy_memory_safe((VoidPtr) "fs/nefs-packet", drive->fPacket.fPacketMime,
                      rt_string_len("fs/nefs-packet"), sizeof(drive->fPacket.fPacketMime));
  if (!drive->fPacket.fPacketGood) {
    err_global_get() = kErrorDiskIsCorrupted;
    return false;
  }

  Char fs_buf[sizeof(NEFS_ROOT_PARTITION_BLOCK)] = {0};
  Lba  start                                     = drive->fLbaStart;

  drive->fPacket.fPacketContent = reinterpret_cast<VoidPtr>(fs_buf);
  drive->fPacket.fPacketSize    = sizeof(NEFS_ROOT_PARTITION_BLOCK);
  drive->fPacket.fPacketLba     = start;
  drive->fInput(drive->fPacket);

  NEFS_ROOT_PARTITION_BLOCK* part_block = (NEFS_ROOT_PARTITION_BLOCK*) fs_buf;
  if (rt_string_cmp(kNeFSIdent, part_block->Ident, kNeFSIdentLen) == 0) return true;

  const auto kNeFSUntitledHD = part_name;
  rt_copy_memory_safe((VoidPtr) kNeFSIdent, (VoidPtr) part_block->Ident, kNeFSIdentLen,
                      sizeof(part_block->Ident));
  rt_copy_memory_safe((VoidPtr) kNeFSUntitledHD, (VoidPtr) part_block->PartitionName,
                      rt_string_len(kNeFSUntitledHD), sizeof(part_block->PartitionName));

  SizeT sectorCount    = drv_std_get_sector_count();
  SizeT sectorSize     = drive->fSectorSz;
  SizeT totalBytes     = sectorCount * sectorSize;
  SizeT catalogEntries = totalBytes / sizeof(NEFS_CATALOG_STRUCT);
  SizeT catalogSectors = (sizeof(NEFS_CATALOG_STRUCT) + sectorSize - 1) / sectorSize;

  part_block->Version      = kNeFSVersionInteger;
  part_block->Kind         = kNeFSPartitionTypeStandard;
  part_block->StartCatalog = start + catalogSectors;
  part_block->Flags        = 0UL;
  part_block->CatalogCount = catalogEntries;
  part_block->FreeCatalog  = catalogEntries - 1;
  part_block->SectorCount  = sectorCount;
  part_block->DiskSize     = totalBytes;
  part_block->SectorSize   = sectorSize;
  part_block->FreeSectors  = sectorCount - catalogSectors;

  drive->fPacket.fPacketContent = reinterpret_cast<VoidPtr>(fs_buf);
  drive->fPacket.fPacketSize    = sizeof(NEFS_ROOT_PARTITION_BLOCK);
  drive->fPacket.fPacketLba     = start;
  drive->fOutput(drive->fPacket);

  (Void)(kout << "Drive kind: " << drive->fProtocol() << kendl);
  (Void)(kout << "Partition name: " << part_block->PartitionName << kendl);
  (Void)(kout << "Start catalog: " << hex_number(part_block->StartCatalog) << kendl);
  (Void)(kout << "Number of catalogs: " << hex_number(part_block->CatalogCount) << kendl);
  (Void)(kout << "Free catalog: " << hex_number(part_block->FreeCatalog) << kendl);
  (Void)(kout << "Free sectors: " << hex_number(part_block->FreeSectors) << kendl);
  (Void)(kout << "Sector size: " << hex_number(part_block->SectorSize) << kendl);

  return true;
}

/***********************************************************************************/
/// @brief Writes the data fork into a specific catalog.
/// @param catalog the catalog itself
/// @param data the data.
/// @return if the catalog wrote the contents successfully.
/***********************************************************************************/
bool NeFileSystemParser::WriteCatalog(_Input const Char* catalog_name, Bool is_rsrc_fork,
                                      _Input VoidPtr data, _Input SizeT size_of_data,
                                      _Input const Char* fork_name) {
  if (size_of_data < 1) return NO;

  auto catalog = this->GetCatalog(catalog_name);
  if (!catalog) {
    (Void)(kout << "NeFS: WriteCatalog failed to find catalog: " << catalog_name << kendl);
    return false;
  }

  SizeT maxSize = is_rsrc_fork ? catalog->ResourceForkSize : catalog->DataForkSize;

  if (!is_valid_size(size_of_data, maxSize)) {
    (Void)(kout << "NeFS: WriteCatalog called with invalid size: " << hex_number(size_of_data));
    (Void)(kout << kendl);

    delete catalog;
    return false;
  }

  Lba   startFork = is_rsrc_fork ? catalog->ResourceFork : catalog->DataFork;
  auto& drive     = kMountpoint.A();

  if (!is_valid_lba(startFork, drive)) {
    (Void)(kout << "NeFS: WriteCatalog called with invalid LBA: " << hex_number(startFork));
    (Void)(kout << kendl);

    if (catalog) delete catalog;
    catalog = nullptr;

    return false;
  }

  NEFS_ROOT_PARTITION_BLOCK part_block;
  drive.fPacket.fPacketLba     = kNeFSRootCatalogStartAddress;
  drive.fPacket.fPacketSize    = sizeof(part_block);
  drive.fPacket.fPacketContent = reinterpret_cast<VoidPtr>(&part_block);
  drive.fInput(drive.fPacket);

  auto buf = new UInt8[size_of_data];
  rt_set_memory(buf, 0, size_of_data);
  rt_copy_memory_safe(data, buf, size_of_data, size_of_data);
  rt_copy_memory_safe((VoidPtr) "fs/nefs-packet", drive.fPacket.fPacketMime,
                      rt_string_len("fs/nefs-packet"), sizeof(drive.fPacket.fPacketMime));

  NEFS_FORK_STRUCT* fork_data_input = new NEFS_FORK_STRUCT();
  NEFS_FORK_STRUCT  prev_fork{};

  while (startFork >= part_block.StartCatalog && drive.fPacket.fPacketGood) {
    drive.fPacket.fPacketContent = reinterpret_cast<VoidPtr>(fork_data_input);
    drive.fPacket.fPacketSize    = sizeof(NEFS_FORK_STRUCT);
    drive.fPacket.fPacketLba     = startFork;
    drive.fInput(drive.fPacket);

    if (!drive.fPacket.fPacketGood) {
      break;
    }

    if ((fork_data_input->Flags & kNeFSFlagCreated) &&
        KStringBuilder::Equals(fork_data_input->ForkName, fork_name) &&
        KStringBuilder::Equals(fork_data_input->CatalogName, catalog_name) &&
        fork_data_input->DataSize == size_of_data) {
      SizeT bytes_left = size_of_data;
      SizeT offset     = 0;
      Lba   base_lba   = fork_data_input->DataOffset;

      while (bytes_left > 0) {
        SizeT chunk                  = (bytes_left > kNeFSSectorSz) ? kNeFSSectorSz : bytes_left;
        drive.fPacket.fPacketContent = reinterpret_cast<VoidPtr>(buf + offset);
        drive.fPacket.fPacketSize    = chunk;
        drive.fPacket.fPacketLba     = base_lba + (offset / kNeFSSectorSz);
        drive.fOutput(drive.fPacket);
        offset += chunk;
        bytes_left -= chunk;
      }

      delete fork_data_input;
      delete[] buf;
      delete catalog;
      return true;
    }

    prev_fork = *fork_data_input;
    startFork = fork_data_input->NextSibling;
  }

  delete fork_data_input;
  delete[] buf;
  delete catalog;
  return false;
}

/***********************************************************************************/
/// @brief
/// @param catalog_name the catalog name.
/// @return the newly found catalog.
/***********************************************************************************/
_Output NEFS_CATALOG_STRUCT* NeFileSystemParser::FindCatalog(_Input const Char* catalog_name,
                                                             Lba& out_lba, Bool search_hidden,
                                                             Bool local_search) {
  if (!catalog_name || *catalog_name == 0) return nullptr;

  NEFS_ROOT_PARTITION_BLOCK part{};
  auto&                     drive = kMountpoint.A();

  rt_copy_memory_safe((VoidPtr) "fs/nefs-packet", drive.fPacket.fPacketMime,
                      rt_string_len("fs/nefs-packet"), sizeof(drive.fPacket.fPacketMime));
  drive.fPacket.fPacketContent = reinterpret_cast<VoidPtr>(&part);
  drive.fPacket.fPacketSize    = sizeof(NEFS_ROOT_PARTITION_BLOCK);
  drive.fPacket.fPacketLba     = kNeFSRootCatalogStartAddress;
  drive.fInput(drive.fPacket);

  auto start_catalog_lba = kNeFSCatalogStartAddress;

  // Helper lambda to scan from a given LBA
  auto scan_from = [&](Lba lba_start, Bool allow_hidden) -> NEFS_CATALOG_STRUCT* {
    Lba                 cursor = lba_start;
    NEFS_CATALOG_STRUCT tmp{};
    while (cursor >= part.StartCatalog && drive.fPacket.fPacketGood) {
      drive.fPacket.fPacketLba     = cursor;
      drive.fPacket.fPacketContent = reinterpret_cast<VoidPtr>(&tmp);
      drive.fPacket.fPacketSize    = sizeof(NEFS_CATALOG_STRUCT);
      drive.fInput(drive.fPacket);

      if (KStringBuilder::Equals(
              tmp.Name, catalog_name + (rt_string_len(catalog_name) - rt_string_len(tmp.Name)))) {
        if (tmp.Status == kNeFSStatusLocked && !allow_hidden) {
          err_global_get() = kErrorFileLocked;
          return nullptr;
        }
        if (!(tmp.Flags & kNeFSFlagCreated)) {
          err_global_get() = kErrorFileNotFound;
          return nullptr;
        }
        NEFS_CATALOG_STRUCT* catalog_ptr = new NEFS_CATALOG_STRUCT();
        rt_copy_memory_safe(&tmp, catalog_ptr, sizeof(NEFS_CATALOG_STRUCT),
                            sizeof(NEFS_CATALOG_STRUCT));
        out_lba = cursor;
        return catalog_ptr;
      }
      cursor = tmp.NextSibling;
    }
    return nullptr;
  };

  if (!KStringBuilder::Equals(catalog_name, NeFileSystemHelper::Root()) && local_search) {
    Char  parent_name[kNeFSCatalogNameLen] = {0};
    SizeT nameLen                          = rt_string_len(catalog_name);
    rt_copy_memory_safe(const_cast<Char*>(catalog_name), parent_name, nameLen + 1,
                        kNeFSCatalogNameLen);

    SizeT indexReverseCopy = nameLen - 1;
    if (parent_name[indexReverseCopy] == NeFileSystemHelper::Separator()) {
      parent_name[indexReverseCopy] = 0;
      --indexReverseCopy;
    }
    while (indexReverseCopy > 0 &&
           parent_name[indexReverseCopy] != NeFileSystemHelper::Separator()) {
      parent_name[indexReverseCopy] = 0;
      --indexReverseCopy;
    }
    if (indexReverseCopy == 0 && parent_name[0] != NeFileSystemHelper::Separator()) {
      return nullptr;
    }

    NEFS_CATALOG_STRUCT* parent_catalog =
        this->FindCatalog(parent_name, out_lba, search_hidden, NO);
    if (parent_catalog) {
      start_catalog_lba = parent_catalog->NextSibling;
      delete parent_catalog;
      NEFS_CATALOG_STRUCT* found = scan_from(start_catalog_lba, search_hidden);
      if (found) return found;
    }
  }

  return scan_from(part.StartCatalog, search_hidden);
}

/***********************************************************************************/
/// @brief Get catalog from filesystem.
/// @param name the catalog's name/
/// @return
/***********************************************************************************/
_Output NEFS_CATALOG_STRUCT* NeFileSystemParser::GetCatalog(_Input const Char* name) {
  Lba unused = 0;
  return this->FindCatalog(name, unused, YES, YES);
}

/***********************************************************************************/
/// @brief Closes a catalog, (frees it).
/// @param catalog the catalog to close.
/// @return
/***********************************************************************************/
_Output Boolean NeFileSystemParser::CloseCatalog(_Input _Output NEFS_CATALOG_STRUCT* catalog) {
  if (!catalog) return false;
  delete catalog;
  catalog = nullptr;
  return true;
}

/***********************************************************************************/
/// @brief Mark catalog as removed.
/// @param catalog The catalog structure.
/// @return if the catalog was removed or not.
/***********************************************************************************/
_Output Boolean NeFileSystemParser::RemoveCatalog(_Input const Char* catalog_name) {
  if (!catalog_name || KStringBuilder::Equals(catalog_name, NeFileSystemHelper::Root())) {
    err_global_get() = kErrorInternal;
    return false;
  }

  Lba  out_lba = 0;
  auto catalog = this->FindCatalog(catalog_name, out_lba, YES, YES);
  if (!catalog) return false;

  auto&            drive = kMountpoint.A();
  NEFS_FORK_STRUCT fork_buf{};
  Lba              fork_lba = catalog->DataFork;
  while (fork_lba >= kNeFSCatalogStartAddress) {
    drive.fPacket.fPacketLba     = fork_lba;
    drive.fPacket.fPacketSize    = sizeof(fork_buf);
    drive.fPacket.fPacketContent = reinterpret_cast<VoidPtr>(&fork_buf);
    drive.fInput(drive.fPacket);

    fork_buf.Flags &= (~kNeFSFlagCreated);
    fork_buf.Flags |= kNeFSFlagDeleted;

    drive.fPacket.fPacketContent = reinterpret_cast<VoidPtr>(&fork_buf);
    drive.fPacket.fPacketSize    = sizeof(fork_buf);
    drive.fPacket.fPacketLba     = fork_lba;
    drive.fOutput(drive.fPacket);

    fork_lba = fork_buf.NextSibling;
  }

  fork_lba = catalog->ResourceFork;
  while (fork_lba >= kNeFSCatalogStartAddress) {
    drive.fPacket.fPacketLba     = fork_lba;
    drive.fPacket.fPacketSize    = sizeof(fork_buf);
    drive.fPacket.fPacketContent = reinterpret_cast<VoidPtr>(&fork_buf);
    drive.fInput(drive.fPacket);

    fork_buf.Flags &= (~kNeFSFlagCreated);
    fork_buf.Flags |= kNeFSFlagDeleted;

    drive.fPacket.fPacketContent = reinterpret_cast<VoidPtr>(&fork_buf);
    drive.fPacket.fPacketSize    = sizeof(fork_buf);
    drive.fPacket.fPacketLba     = fork_lba;
    drive.fOutput(drive.fPacket);

    fork_lba = fork_buf.NextSibling;
  }

  if (out_lba >= kNeFSCatalogStartAddress || (catalog->Flags & kNeFSFlagCreated)) {
    catalog->Flags &= (~kNeFSFlagCreated);
    catalog->Flags |= kNeFSFlagDeleted;

    rt_copy_memory_safe((VoidPtr) "fs/nefs-packet", drive.fPacket.fPacketMime,
                        rt_string_len("fs/nefs-packet"), sizeof(drive.fPacket.fPacketMime));
    drive.fPacket.fPacketLba     = out_lba;
    drive.fPacket.fPacketSize    = sizeof(NEFS_CATALOG_STRUCT);
    drive.fPacket.fPacketContent = reinterpret_cast<VoidPtr>(catalog);
    drive.fOutput(drive.fPacket);

    Char partitionBlockBuf[sizeof(NEFS_ROOT_PARTITION_BLOCK)] = {0};
    drive.fPacket.fPacketLba                                  = kNeFSRootCatalogStartAddress;
    drive.fPacket.fPacketContent = reinterpret_cast<VoidPtr>(partitionBlockBuf);
    drive.fPacket.fPacketSize    = sizeof(NEFS_ROOT_PARTITION_BLOCK);
    drive.fInput(drive.fPacket);

    NEFS_ROOT_PARTITION_BLOCK* part_block = (NEFS_ROOT_PARTITION_BLOCK*) partitionBlockBuf;
    if (part_block->CatalogCount > 0) --part_block->CatalogCount;
    ++part_block->FreeSectors;

    drive.fPacket.fPacketContent = reinterpret_cast<VoidPtr>(part_block);
    drive.fPacket.fPacketSize    = sizeof(NEFS_ROOT_PARTITION_BLOCK);
    drive.fPacket.fPacketLba     = kNeFSRootCatalogStartAddress;
    drive.fOutput(drive.fPacket);

    delete catalog;
    catalog = nullptr;
    return true;
  }

  delete catalog;
  catalog = nullptr;
  return false;
}

/// ***************************************************************** ///
/// Reading,Seek,Tell are unimplemented on catalogs, refer to forks I/O instead.
/// ***************************************************************** ///

/***********************************************************************************/
/// @brief Read the catalog data fork.
/// @param catalog
/// @param dataSz
/// @return
/***********************************************************************************/
VoidPtr NeFileSystemParser::ReadCatalog(_Input _Output NEFS_CATALOG_STRUCT* catalog,
                                        _Input Bool is_rsrc_fork, _Input SizeT dataSz,
                                        _Input const Char* forkName) {
  if (!catalog) {
    err_global_get() = kErrorInvalidData;
    return nullptr;
  }
  // Validate size against fork size
  SizeT maxSize = is_rsrc_fork ? catalog->ResourceForkSize : catalog->DataForkSize;
  if (!is_valid_size(dataSz, maxSize)) {
    kout << "NeFS: ReadCatalog called with invalid size: ";
    hex_number(dataSz);
    (Void)(kout << kendl);
    return nullptr;
  }

  Lba   dataForkLba = is_rsrc_fork ? catalog->ResourceFork : catalog->DataFork;
  auto& drive       = kMountpoint.A();
  if (!is_valid_lba(dataForkLba, drive)) {
    kout << "NeFS: ReadCatalog called with invalid LBA: ";
    hex_number(dataForkLba);
    (Void)(kout << kendl);
    return nullptr;
  }

  auto* fs_buf = new NEFS_FORK_STRUCT();
  rt_copy_memory_safe((VoidPtr) "fs/nefs-packet", drive.fPacket.fPacketMime,
                      rt_string_len("fs/nefs-packet"), sizeof(drive.fPacket.fPacketMime));

  NEFS_FORK_STRUCT* fs_fork_data = nullptr;
  while (dataForkLba >= kNeFSCatalogStartAddress) {
    drive.fPacket.fPacketLba     = dataForkLba;
    drive.fPacket.fPacketSize    = sizeof(NEFS_FORK_STRUCT);
    drive.fPacket.fPacketContent = reinterpret_cast<VoidPtr>(fs_buf);
    drive.fInput(drive.fPacket);

    fs_fork_data = fs_buf;
    (Void)(kout << "ForkName: " << fs_fork_data->ForkName << kendl);
    (Void)(kout << "CatalogName: " << fs_fork_data->CatalogName << kendl);

    if (KStringBuilder::Equals(forkName, fs_fork_data->ForkName) &&
        KStringBuilder::Equals(catalog->Name, fs_fork_data->CatalogName)) {
      break;
    }
    dataForkLba = fs_fork_data->NextSibling;
  }

  if (dataForkLba < kNeFSCatalogStartAddress) {
    delete fs_buf;
    return nullptr;
  }
  return fs_fork_data;
}

/***********************************************************************************/
/// @brief Seek in the data fork.
/// @param catalog the catalog offset.
/// @param off where to seek.
/// @return if the seeking was successful.
/***********************************************************************************/
bool NeFileSystemParser::Seek(_Input _Output NEFS_CATALOG_STRUCT* catalog, SizeT off) {
  NE_UNUSED(catalog);
  NE_UNUSED(off);
  err_global_get() = kErrorUnimplemented;
  return false;
}

/***********************************************************************************/
/// @brief Tell where we are inside the data fork.
/// @param catalog
/// @return The position on the file.
/***********************************************************************************/
SizeT NeFileSystemParser::Tell(_Input _Output NEFS_CATALOG_STRUCT* catalog) {
  NE_UNUSED(catalog);
  err_global_get() = kErrorUnimplemented;
  return 0;
}

namespace Ne::Kernel::NeFS {
/***********************************************************************************/
/// @brief Construct NeFS drives.
/***********************************************************************************/
Boolean fs_init_nefs(Void) {
  kout << "Creating NeFS disk...\n";

  io_construct_main_drive(kMountpoint.A());
  
  if (kMountpoint.A().fPacket.fPacketReadOnly == YES)
    ke_stop(RUNTIME_CHECK_FILESYSTEM, "Main disk cannot be mounted.");

  NeFileSystemParser parser;
  return parser.Format(&kMountpoint.A(), 0, kNeFSVolumeName);
}
}  // namespace Ne::Kernel::NeFS

#endif  // ifdef __FSKIT_INCLUDES_NEFS__
