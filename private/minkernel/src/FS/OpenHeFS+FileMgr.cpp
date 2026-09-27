// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifndef __NE_MINIMAL_OS__
#ifdef __FSKIT_INCLUDES_OPENHEFS__

#include <KernelKit/FileMgr.h>
#include <KernelKit/HeapMgr.h>

/// @brief OpenHeFS File System Manager.
/// BUGS: 0

namespace Ne::Kernel {

namespace Detail {
  /// @brief An open OpenHeFS file. The parser is name addressed, so we keep the
  /// path split apart and carry the cursor the parser has no room for.
  struct HEFS_NODE_DESC final {
    Utf8Char fDir[kOpenHeFSFileNameLen];
    Utf8Char fName[kOpenHeFSFileNameLen];
    SizeT    fCursor;
    UInt8    fKind;
  };

  /// @brief Split a path into its directory and its basename.
  /// @return NO when either half does not fit.
  STATIC Bool hefs_split_path(const Char* path, Utf8Char* dir, Utf8Char* name) {
    if (!path || *path != '/') return NO;

    auto len = rt_string_len(path, kOpenHeFSFileNameLen);

    if (len < 2 || len >= kOpenHeFSFileNameLen) return NO;

    auto cut = 0UL;

    for (SizeT i = 0UL; i < len; ++i) {
      if (path[i] == '/') cut = i;
    }

    auto name_len = len - cut - 1;

    if (name_len < 1 || name_len >= kOpenHeFSFileNameLen) return NO;

    auto dir_len = cut ? cut : 1;

    for (SizeT i = 0UL; i < dir_len; ++i) dir[i] = path[i];
    dir[dir_len] = 0;

    for (SizeT i = 0UL; i < name_len; ++i) name[i] = path[cut + 1 + i];
    name[name_len] = 0;

    return YES;
  }
}  // namespace Detail

/// @brief C++ constructor
HeFileSystemMgr::HeFileSystemMgr() {
  mParser = new HeFileSystemParser();
  MUST_PASS(mParser);

  io_construct_main_drive(mDriveTrait);

  kout << "OpenHeFS: Allocated HeFileSystemParser...\r";
}

HeFileSystemMgr::~HeFileSystemMgr() {
  if (mParser) {
    kout << "OpenHeFS: Destroying HeFileSystemParser...\r";

    delete mParser;
    mParser = nullptr;
  }
}

/// @brief Removes a node from the filesystem.
/// @param path The filename
/// @return If it was deleted or not.
bool HeFileSystemMgr::Remove(_Input const Char* path) {
  if (path == nullptr || *path == 0) {
    kout << "OpenHeFS: Remove called with null or empty path\r";
    return NO;
  }

  Utf8Char dir[kOpenHeFSFileNameLen]  = {0};
  Utf8Char name[kOpenHeFSFileNameLen] = {0};

  if (!Detail::hefs_split_path(path, dir, name)) {
    err_local_get() = kErrorInvalidData;
    return NO;
  }

  err_local_get() = kErrorSuccess;

  return mParser->DeleteINode(&mDriveTrait, kOpenHeFSEncodingFlagsUTF8, dir, name,
                              kOpenHeFSFileKindRegular);
}

/// @brief Creates a node with the specified.
/// @param path The filename path.
/// @return The Node pointer.
NodePtr HeFileSystemMgr::Create(_Input const Char* path) {
  if (!path || *path == 0) {
    kout << "OpenHeFS: Create called with null or empty path\r";
    return nullptr;
  }

  Utf8Char dir[kOpenHeFSFileNameLen]  = {0};
  Utf8Char name[kOpenHeFSFileNameLen] = {0};

  if (!Detail::hefs_split_path(path, dir, name)) {
    err_local_get() = kErrorInvalidData;
    return nullptr;
  }

  err_local_get() = kErrorSuccess;

  if (!mParser->CreateINode(&mDriveTrait, kOpenHeFSEncodingFlagsUTF8, dir, name,
                            kOpenHeFSFileKindRegular)) {
    kout << "OpenHeFS: ERROR: Check KPC.\r";

    err_local_get() = kErrorDiskIsFull;

    return nullptr;
  }

  return this->Open(path, "rb");
}

/// @brief Creates a node which is a directory.
/// @param path The filename path.
/// @return The Node pointer.
NodePtr HeFileSystemMgr::CreateDirectory(const Char* path) {
  if (!path || *path == 0) {
    kout << "OpenHeFS: CreateDirectory called with null or empty path\r";
    return nullptr;
  }

  Utf8Char dir[kOpenHeFSFileNameLen] = {0};

  auto len = rt_string_len(path, kOpenHeFSFileNameLen);

  if (len < 1 || len >= kOpenHeFSFileNameLen) {
    err_local_get() = kErrorInvalidData;
    return nullptr;
  }

  for (SizeT i = 0UL; i < len; ++i) dir[i] = path[i];

  err_local_get() = kErrorSuccess;

  if (!mParser->CreateINodeDirectory(&mDriveTrait, kOpenHeFSEncodingFlagsUTF8, dir)) {
    kout << "OpenHeFS: ERROR: Check KPC.\r";

    err_local_get() = kErrorDiskIsFull;
  }

  return nullptr;
}

/// @brief Creates a node which is an alias.
/// @param path The filename path.
/// @return The Node pointer.
NodePtr HeFileSystemMgr::CreateAlias(const Char* path) {
  if (!path || *path == 0) {
    kout << "OpenHeFS: CreateAlias called with null or empty path\r";
    return nullptr;
  }

  Utf8Char dir[kOpenHeFSFileNameLen]  = {0};
  Utf8Char name[kOpenHeFSFileNameLen] = {0};

  if (!Detail::hefs_split_path(path, dir, name)) {
    err_local_get() = kErrorInvalidData;
    return nullptr;
  }

  err_local_get() = kErrorSuccess;

  if (!mParser->CreateINode(&mDriveTrait, kOpenHeFSEncodingFlagsUTF8, dir, name,
                            kOpenHeFSFileKindSymbolicLink)) {
    kout << "OpenHeFS: ERROR: Check KPC.\r";

    err_local_get() = kErrorDiskIsFull;
  }

  return nullptr;
}

NodePtr HeFileSystemMgr::CreateSwapFile(const Char* path) {
  if (!path || *path == 0) {
    kout << "OpenHeFS: CreateSwapFile called with null or empty path\r";
    return nullptr;
  }

  kout << "OpenHEFS: ERROR: Swap Files are not supported natively by OpenHeFS.\r";
  err_local_get() = kErrorInvalidData;

  return nullptr;
}

/// @brief Gets the root directory.
/// @return
const Char* NeFileSystemHelper::Root() {
  return kOpenHeFSRootDirectory;
}

/// @brief Gets the up-dir directory.
/// @return
const Char* NeFileSystemHelper::UpDir() {
  return kOpenHeFSUpDir;
}

/// @brief Gets the separator character.
/// @return
Char NeFileSystemHelper::Separator() {
  return kOpenHeFSSeparator;
}

/// @brief Gets the metafile character.
/// @return
Char NeFileSystemHelper::MetaFile() {
  return '\0';
}

/// @brief Opens a new file.
/// @param path
/// @param r
/// @return
_Output NodePtr HeFileSystemMgr::Open(_Input const Char* path, _Input const Char* r) {
  if (!path || *path == 0) {
    kout << "OpenHeFS: Open called with null or empty path\r";
    return nullptr;
  }

  if (!r || *r == 0) {
    kout << "OpenHeFS: Open called with null or empty mode string\r";
    return nullptr;
  }

  auto desc = new Detail::HEFS_NODE_DESC();

  if (!desc) {
    err_local_get() = kErrorHeapOutOfMemory;
    return nullptr;
  }

  if (!Detail::hefs_split_path(path, desc->fDir, desc->fName)) {
    delete desc;

    err_local_get() = kErrorInvalidData;

    return nullptr;
  }

  desc->fKind   = kOpenHeFSFileKindRegular;
  desc->fCursor = 0UL;

  MUST_PASS(mParser);

  if (!mParser->INodeExists(&mDriveTrait, desc->fDir, desc->fName, desc->fKind)) {
    delete desc;

    err_local_get() = kErrorFileNotFound;

    return nullptr;
  }

  err_local_get() = kErrorSuccess;

  return rtl_node_cast(desc);
}

Void HeFileSystemMgr::Write(_Input NodePtr node, _Input VoidPtr data, _Input Int32 flags,
                            _Input SizeT size) {
  NE_UNUSED(flags);

  if (!node || !data || !size) return;

  auto desc = reinterpret_cast<Detail::HEFS_NODE_DESC*>(node);

  if (size > kOpenHeFSBlockLen) {
    err_local_get() = kErrorDiskIsFull;
    return;
  }

  if (!mParser->INodeManip(&mDriveTrait, data, size, desc->fDir, desc->fName, desc->fKind, NO)) {
    err_local_get() = kErrorFileNotFound;
    return;
  }

  desc->fCursor   = size;
  err_local_get() = kErrorSuccess;
}

_Output VoidPtr HeFileSystemMgr::Read(_Input NodePtr node, _Input Int32 flags, _Input SizeT size) {
  NE_UNUSED(flags);

  if (!node || !size) return nullptr;

  auto desc = reinterpret_cast<Detail::HEFS_NODE_DESC*>(node);

  if (size > kOpenHeFSBlockLen) size = kOpenHeFSBlockLen;

  auto blob = mm_alloc_ptr(size, Yes, No);

  if (!blob) {
    err_local_get() = kErrorHeapOutOfMemory;
    return nullptr;
  }

  rt_set_memory(blob, 0, size);

  if (!mParser->INodeManip(&mDriveTrait, blob, size, desc->fDir, desc->fName, desc->fKind, YES)) {
    mm_free_ptr(blob);

    err_local_get() = kErrorFileNotFound;

    return nullptr;
  }

  desc->fCursor   = size;
  err_local_get() = kErrorSuccess;

  return blob;
}

/// @note name is not used in OpenHeFS to mark data offsets. That's an NeFS-ism.
Void HeFileSystemMgr::Write(_Input const Char* name, _Input NodePtr node, _Input VoidPtr data,
                            _Input Int32 flags, _Input SizeT size) {
  NE_UNUSED(node);

  if (!flags) return;
  if (!size) return;
  if (!data) return;
  if (!name) return;

  STATIC IMountpoint mnt;
  io_construct_main_drive(mnt.A());

  mParser->INodeManip(&mnt.A(), (VoidPtr) data, size, u8"/", (Char8*) name, 0, NO);
}

_Output VoidPtr HeFileSystemMgr::Read(_Input const Char* name, _Input NodePtr node,
                                      _Input Int32 flags, _Input SizeT sz) {
  NE_UNUSED(node);
  NE_UNUSED(flags);
  NE_UNUSED(sz);
  NE_UNUSED(name);

  UInt8* retBlob = new UInt8[sz];

  if (!retBlob) return nullptr;

  rt_set_memory(retBlob, 0, sz);

  STATIC IMountpoint mnt;
  io_construct_main_drive(mnt.A());

  mParser->INodeManip(&mnt.A(), (VoidPtr) retBlob, sz, u8"/", (Char8*) name, 0, YES);

  return retBlob;
}

_Output Bool HeFileSystemMgr::Seek(NodePtr node, SizeT off) {
  if (!node) return NO;

  if (off >= kOpenHeFSBlockLen) {
    err_local_get() = kErrorInvalidData;
    return NO;
  }

  reinterpret_cast<Detail::HEFS_NODE_DESC*>(node)->fCursor = off;

  return YES;
}

/// @brief Tell current offset within catalog.
/// @param node The OpenHeFS node we need.
/// @return kFileMgrNPos if invalid, else current offset.
_Output SizeT HeFileSystemMgr::Tell(NodePtr node) {
  if (!node) return kFileMgrNPos;

  return reinterpret_cast<Detail::HEFS_NODE_DESC*>(node)->fCursor;
}

/// @brief Rewinds the catalog
/// @param node The needed OpenHeFS node.
/// @return False if invalid, nah? calls Seek(node, 0).
_Output Bool HeFileSystemMgr::Rewind(NodePtr node) {
  if (!node) return NO;

  return this->Seek(node, 0UL);
}

/// @brief Returns the parser of OpenHeFS.
_Output HeFileSystemParser* HeFileSystemMgr::GetParser() {
  return mParser;
}

}  // namespace Ne::Kernel

#endif  // ifdef __FSKIT_INCLUDES_OPENHEFS__
#endif  // ifndef __NE_MINIMAL_OS__
