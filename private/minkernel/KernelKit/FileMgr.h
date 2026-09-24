// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifndef KERNELKIT_FILEMGR_H
#define KERNELKIT_FILEMGR_H

/// @file FileMgr.h
/// @brief File Manager Subsystem.
/// @author Amlal El Mahrouss (amlal@nekernel.org)

//! Include filesystems that the kernel supports.

#include <FSKit/NeFS.h>
#include <FSKit/OpenHeFS.h>

#include <CompilerKit/CompilerKit.h>
#include <KernelKit/DebugOutput.h>
#include <KernelKit/HeapMgr.h>
#include <KernelKit/KPC.h>
#include <NeKit/ErrorOr.h>
#include <NeKit/Ref.h>
#include <NeKit/Stream.h>
#include <hint/CompilerHint.h>

/// @brief Filesystem manager, abstraction over mounted filesystem.
/// Works like an VFS (Virtual File System) or IFS subsystem on NT/OS 2.

#define kRestrictR "r"
#define kRestrictRB "rb"

#define kRestrictW "w"
#define kRestrictWR "rw"
#define kRestrictWRB "rwb"

#define kRestrictF "f"
#define kRestrictWRBF "rwbf"
#define kRestrictWRF "rwf"

#define kRestrictMax (5U)

#define rtl_node_cast(PTR) reinterpret_cast<Ne::Kernel::NodePtr>(PTR)

#define kFileMimeGeneric "ne-application-kind/all"

/** @brief invalid position. (n-pos) */
#define kFileMgrNPos (~0UL)

/** @brief maximum length in filemgr. */
#define kFileMgrNameLen (4096UL)

namespace Ne::Kernel {

enum {
  kFileIOInvalid  = 0,
  kFileWriteAll   = 100,
  kFileReadAll    = 101,
  kFileReadChunk  = 102,
  kFileWriteChunk = 103,
  // File flags (HFS+, NeFS specific)
  kFileFlagRsrc = 104,
  kFileFlagData = 105,
  kFileIOCnt    = (kFileFlagData - kFileWriteAll) + 1,
};

using NodePtr = VoidPtr;

/***********************************************************************************/
/// @brief This data structure helps with filesystem status.
/***********************************************************************************/
struct FILEMGR_STAT final {
  UInt64 fCreationTime;
  UInt64 fLastAccessTime;
  UInt64 fLastWriteTime;
  UInt64 fChangeTime;

  UInt64 fAllocationSize;
  UInt64 fEndOfFile;

  UInt32 fNumberOfLinks;
  UInt32 fInodeNumber;

  UInt32 fMode;
  UInt32 fUID;
  UInt32 fGID;

  UInt8 fType;
  UInt8 fDirectory;
  UInt8 fDeletePending;
  UInt8 fReserved;
};

#ifndef kFileMgrDirEntResvLen
#define kFileMgrDirEntResvLen (2)
#endif

struct FILEMGR_DIRENT final {
  UInt32 fInodeNumber;
  UInt8  fType;
  UInt8  fNameLength;
  UInt8  fReserved[kFileMgrDirEntResvLen];
  Char   fName[kFileMgrNameLen];
};

/**
@brief Filesystem Mgr Interface class
@brief Used to provide common I/O for a specific filesystem.
*/
class IFilesystemMgr {
 public:
  explicit IFilesystemMgr() = default;
  virtual ~IFilesystemMgr() = default;

 public:
  NE_COPY_DEFAULT(IFilesystemMgr)

 public:
  /// @brief Mounts a new filesystem into an active state.
  /// @param interface the filesystem interface
  /// @return
  static bool Mount(IFilesystemMgr* interface);

  /// @brief Unmounts the active filesystem
  /// @return
  static IFilesystemMgr* Unmount();

  /// @brief Getter, gets the active filesystem.
  /// @return
  static IFilesystemMgr* GetMounted();

 public:
  virtual NodePtr Create(_Input const Char* path)          = 0;
  virtual NodePtr CreateAlias(_Input const Char* path)     = 0;
  virtual NodePtr CreateDirectory(_Input const Char* path) = 0;
  virtual NodePtr CreateSwapFile(const Char* path)         = 0;

 public:
  virtual bool Remove(_Input const Char* path) = 0;

 public:
  virtual NodePtr Open(_Input const Char* path, _Input const Char* r) = 0;

 public:
  virtual Void Write(_Input NodePtr node, _Input VoidPtr data, _Input Int32 flags,
                     _Input SizeT size) = 0;

  virtual _Output VoidPtr Read(_Input NodePtr node, _Input Int32 flags, _Input SizeT sz) = 0;

  virtual Void Write(_Input const Char* name, _Input NodePtr node, _Input VoidPtr data,
                     _Input Int32 flags, _Input SizeT size) = 0;

  virtual _Output VoidPtr Read(_Input const Char* name, _Input NodePtr node, _Input Int32 flags,
                               _Input SizeT sz) = 0;

 public:
  virtual bool Seek(_Input NodePtr node, _Input UIntPtr off) = 0;

 public:
  virtual SizeT Tell(_Input NodePtr node)   = 0;
  virtual bool  Rewind(_Input NodePtr node) = 0;

  virtual BOOL GetInfo(_Input NodePtr node, _Output FILEMGR_STAT* out) {
    NE_UNUSED(node);
    NE_UNUSED(out);

    err_local_get() = kErrorUnimplemented;

    return NO;
  }

  virtual BOOL ReadDir(_Input NodePtr node, _Input UInt64 cookie, _Output FILEMGR_DIRENT* out,
                       _Output UInt64* next_cookie) {
    NE_UNUSED(node);
    NE_UNUSED(out);
    NE_UNUSED(cookie);
    NE_UNUSED(next_cookie);

    err_local_get() = kErrorUnimplemented;

    return NO;
  }

  virtual Int32 ReadLink(_Input NodePtr node, _Output Char* buf, _Input SizeT buf_size) {
    NE_UNUSED(node);
    NE_UNUSED(buf);
    NE_UNUSED(buf_size);

    err_local_get() = kErrorUnimplemented;

    return -1;
  }
};

#ifdef __FSKIT_INCLUDES_NEFS__
/**
 * @brief Based of IFilesystemMgr, takes care of managing NeFS
 * disks.
 */
class NeFileSystemMgr final : public IFilesystemMgr {
 public:
  explicit NeFileSystemMgr();
  ~NeFileSystemMgr() override;

 public:
  NE_COPY_DEFAULT(NeFileSystemMgr)

 public:
  NodePtr Create(const Char* path) override;
  NodePtr CreateAlias(const Char* path) override;
  NodePtr CreateDirectory(const Char* path) override;
  NodePtr CreateSwapFile(const Char* path) override;

 public:
  bool    Remove(_Input const Char* path) override;
  NodePtr Open(_Input const Char* path, _Input const Char* r) override;
  Void    Write(_Input NodePtr node, _Input VoidPtr data, _Input Int32 flags,
                _Input SizeT sz) override;
  VoidPtr Read(_Input NodePtr node, _Input Int32 flags, _Input SizeT sz) override;
  bool    Seek(_Input NodePtr node, _Input SizeT off) override;
  SizeT   Tell(_Input NodePtr node) override;
  bool    Rewind(_Input NodePtr node) override;

  Void Write(_Input const Char* name, _Input NodePtr node, _Input VoidPtr data, _Input Int32 flags,
             _Input SizeT size) override;

  _Output VoidPtr Read(_Input const Char* name, _Input NodePtr node, _Input Int32 flags,
                       _Input SizeT sz) override;

 public:
  /// @brief Get NeFS parser class.
  /// @return The filesystem parser class.
  NeFileSystemParser* GetParser();

 private:
  NeFileSystemParser* mParser{nullptr};
};

#endif  // ifdef __FSKIT_INCLUDES_NEFS__

#ifdef __FSKIT_INCLUDES_OPENHEFS__
/**
 * @brief Based of IFilesystemMgr, takes care of managing NeFS
 * disks.
 */
class HeFileSystemMgr final : public IFilesystemMgr {
 public:
  explicit HeFileSystemMgr();
  ~HeFileSystemMgr() override;

 public:
  NE_COPY_DEFAULT(HeFileSystemMgr)

 public:
  NodePtr Create(const Char* path) override;
  NodePtr CreateAlias(const Char* path) override;
  NodePtr CreateDirectory(const Char* path) override;
  NodePtr CreateSwapFile(const Char* path) override;

 public:
  bool    Remove(_Input const Char* path) override;
  NodePtr Open(_Input const Char* path, _Input const Char* r) override;
  Void    Write(_Input NodePtr node, _Input VoidPtr data, _Input Int32 flags,
                _Input SizeT sz) override;
  VoidPtr Read(_Input NodePtr node, _Input Int32 flags, _Input SizeT sz) override;
  bool    Seek(_Input NodePtr node, _Input SizeT off) override;
  SizeT   Tell(_Input NodePtr node) override;
  bool    Rewind(_Input NodePtr node) override;

  Void Write(_Input const Char* name, _Input NodePtr node, _Input VoidPtr data, _Input Int32 flags,
             _Input SizeT size) override;

  _Output VoidPtr Read(_Input const Char* name, _Input NodePtr node, _Input Int32 flags,
                       _Input SizeT sz) override;

 public:
  /// @brief Get NeFS parser class.
  /// @return The filesystem parser class.
  HeFileSystemParser* GetParser();

 private:
  HeFileSystemParser* mParser{nullptr};
  DriveTrait          mDriveTrait;
};

#endif  // ifdef __FSKIT_INCLUDES_OPENHEFS__

/**
 * FileStream class.
 * @tparam Encoding file encoding (char, wchar_t...)
 * @tparam FSClass Filesystem contract who takes care of it.
 */
template <typename Encoding = Char, typename FSClass = IFilesystemMgr>
class FileStream final {
 public:
  FileStream(const Encoding* path, const Encoding* restrict_type);
  ~FileStream();

 public:
  FileStream& operator=(const FileStream&);
  FileStream(const FileStream&);

 public:
  Ref<Int64> Write(UIntPtr offset, const VoidPtr data, SizeT len) {
    if (this->fFileRestrict != kFileMgrRestrictReadWrite &&
        this->fFileRestrict != kFileMgrRestrictReadWriteBinary &&
        this->fFileRestrict != kFileMgrRestrictWrite &&
        this->fFileRestrict != kFileMgrRestrictWriteBinary)
      return Ref<Int64>(0L);

    if (data == nullptr) return Ref<Int64>(0L);

    auto man = FSClass::GetMounted();

    if (man) {
      man->Seek(fFile, offset);
      man->Write(fFile, data, 0, len);
      return Ref<Int64>(len);
    }

    return Ref<Int64>(0L);
  }

  Ref<Int64> Write(const Char* name, const VoidPtr data, SizeT len) {
    if (this->fFileRestrict != kFileMgrRestrictReadWrite &&
        this->fFileRestrict != kFileMgrRestrictReadWriteBinary &&
        this->fFileRestrict != kFileMgrRestrictWrite &&
        this->fFileRestrict != kFileMgrRestrictWriteBinary)
      return Ref<Int64>(0L);

    if (data == nullptr) return Ref<Int64>(0L);

    auto man = FSClass::GetMounted();

    if (man) {
      man->Write(name, fFile, data, 0, len);
      return Ref<Int64>(len);
    }

    return Ref<Int64>(0L);
  }

  ErrorOrAny Read(const Char* name, SizeT sz) {
    if (this->fFileRestrict != kFileMgrRestrictReadWrite &&
        this->fFileRestrict != kFileMgrRestrictReadWriteBinary &&
        this->fFileRestrict != kFileMgrRestrictRead &&
        this->fFileRestrict != kFileMgrRestrictReadBinary)
      return ErrorOrAny(kErrorInvalidData);

    auto man = FSClass::GetMounted();

    if (man) {
      VoidPtr ret = man->Read(name, fFile, kFileReadAll, sz);
      return ErrorOrAny(ret);
    }

    return ErrorOrAny(kErrorInvalidData);
  }

  ErrorOrAny Read(UIntPtr offset, SizeT sz) {
    if (this->fFileRestrict != kFileMgrRestrictReadWrite &&
        this->fFileRestrict != kFileMgrRestrictReadWriteBinary &&
        this->fFileRestrict != kFileMgrRestrictRead &&
        this->fFileRestrict != kFileMgrRestrictReadBinary)
      return ErrorOrAny(kErrorInvalidData);

    auto man = FSClass::GetMounted();

    if (man) {
      man->Seek(fFile, offset);
      auto ret = man->Read(fFile, kFileReadChunk, sz);

      return ErrorOrAny(ret);
    }

    return ErrorOrAny(kErrorInvalidData);
  }

 public:
  /// @brief Leak node pointer.
  /// @return The node pointer.
  NodePtr Leak() { return fFile; }

  /// @brief Leak MIME.
  /// @return The MIME.
  Char* MIME() { return fMime; }

  enum {
    kFileMgrRestrictRead = 100,
    kFileMgrRestrictReadBinary,
    kFileMgrRestrictWrite,
    kFileMgrRestrictFork,
    kFileMgrRestrictWriteBinary,
    kFileMgrRestrictReadWrite,
    kFileMgrRestrictReadWriteFork,
    kFileMgrRestrictReadWriteBinary,
    kFileMgrRestrictReadWriteBinaryFork,
  };

 private:
  NodePtr fFile{nullptr};
  Int32   fFileRestrict{kFileMgrRestrictReadBinary};
  Char*   fMime{const_cast<Char*>(kFileMimeGeneric)};
};

using FileStreamASCII = FileStream<Char>;
using FileStreamUTF8  = FileStream<Utf8Char>;
using FileStreamUTF16 = FileStream<Utf16Char>;
using FileStreamWide  = FileStream<WideChar>;

#if __FSKIT_DEFAULT_ENCODING__ == __FSKIT_ENCODING_UTF8__
using FileStreamDefault = FileStreamUTF8;
#elif __FSKIT_DEFAULT_ENCODING__ == __FSKIT_ENCODING_UTF16__
using FileStreamDefault = FileStreamUTF16;
#elif __FSKIT_DEFAULT_ENCODING__ == __FSKIT_ENCODING_WIDE__
using FileStreamDefault = FileStreamWide;
#else
using FileStreamDefault = FileStreamASCII;
#endif

typedef UInt64 CursorType;

inline STATIC const auto kRestrictStrLen = 8U;

/// @brief restrict information about the file descriptor.
struct FILEMGR_RESTRICT final {
  Char  fRestrict[kRestrictStrLen];
  Int32 fMappedTo;
};

/// @brief constructor
template <typename Encoding, typename Class>
inline FileStream<Encoding, Class>::FileStream(const Encoding* path,
                                               const Encoding* restrict_type) {
  MUST_PASS(Class::GetMounted());
  fFile = Class::GetMounted()->Open(path, restrict_type);

  SizeT                  kRestrictCount  = kRestrictMax;
  const FILEMGR_RESTRICT kRestrictList[] = {{
                                                .fRestrict = kRestrictR,
                                                .fMappedTo = kFileMgrRestrictRead,
                                            },
                                            {
                                                .fRestrict = kRestrictRB,
                                                .fMappedTo = kFileMgrRestrictReadBinary,
                                            },
                                            {
                                                .fRestrict = kRestrictWRB,
                                                .fMappedTo = kFileMgrRestrictReadWriteBinary,
                                            },
                                            {
                                                .fRestrict = kRestrictW,
                                                .fMappedTo = kFileMgrRestrictWrite,
                                            },
                                            {
                                                .fRestrict = kRestrictWR,
                                                .fMappedTo = kFileMgrRestrictReadWrite,
                                            }};

  for (SizeT index{}; index < kRestrictCount; ++index) {
    if (rt_string_cmp(restrict_type, kRestrictList[index].fRestrict,
                      rt_string_len(kRestrictList[index].fRestrict)) == 0) {
      fFileRestrict = kRestrictList[index].fMappedTo;
      break;
    }
  }

  kout << "FileMgr: Open file at: " << path << ".\r";
}

/// @brief destructor of the file stream.
template <typename Encoding, typename Class>
inline FileStream<Encoding, Class>::~FileStream() {
  if (mm_is_valid_ptr(fFile)) mm_free_ptr(fFile);

  kout << "FileMgr: ~FileStream()\r";

  fFile = nullptr;
}

}  // namespace Ne::Kernel

#endif  // ifndef KERNELKIT_FILEMGR_H
