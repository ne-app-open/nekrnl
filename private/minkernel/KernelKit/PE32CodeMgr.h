// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifndef KERNELKIT_PE32CODEMGR_H
#define KERNELKIT_PE32CODEMGR_H

////////////////////////////////////////////////////

// LAST REV: Mon Feb 12 13:52:01 CET 2024

////////////////////////////////////////////////////

#include <KernelKit/FileMgr.h>
#include <KernelKit/ILoader.h>
#include <KernelKit/PE.h>
#include <NeKit/ErrorOr.h>
#include <NeKit/KString.h>

#ifndef KERNELKIT_PROCESS_SCHEDULER_H
#include <KernelKit/ProcessScheduler.h>
#endif

#define kPeApplicationMime "application/vnd-portable-executable"

namespace Ne::Kernel {

///
/// \name PE32Loader
/// \brief PE32+ loader class.
///
class PE32Loader NE_EXEC_LOADER {
 private:
  explicit PE32Loader() = delete;

 public:
  explicit PE32Loader(const VoidPtr blob, const SizeT len);
  explicit PE32Loader(const Char* path);
  ~PE32Loader() override;

 public:
  NE_COPY_DEFAULT(PE32Loader)

 public:
  const Char* Path() override;
  const Char* AsString() override;
  const Char* MIME() override;

 public:
  ErrorOr<VoidPtr> FindStart() override;
  ErrorOr<VoidPtr> FindSectionByName(const Char* name);
  ErrorOr<VoidPtr> FindSymbol(const Char* name, Int32 kind) override;
  ErrorOr<VoidPtr> GetBlob() override;

 public:
  BOOL IsLoaded();

 private:
  /// @brief Instantiate the image in memory, then map it. Idempotent.
  ErrorOr<VoidPtr> LoadImage();

 private:
#ifdef __FSKIT_INCLUDES_NEFS__
  OwnPtr<FileStream<Char, NeFileSystemMgr>> fFile;
#elif defined(__FSKIT_INCLUDES_OPENHEFS__)
  OwnPtr<FileStream<Char, HeFileSystemMgr>> fFile;
#else
  OwnPtr<FileStream<Char>> fFile;
#endif  // __FSKIT_INCLUDES_NEFS__

  Ref<KString> fPath;
  ErrorOrAny   fCachedBlob{};
  SizeT        fCachedBlobSz{};
  VoidPtr      fImage{};
  BOOL         fBad{};
};

enum { kPEPlatformInvalid, kPEPlatformAMD64 = 100, kPEPlatformARM64 };
enum { kPETypeInvalid, kPETypeText = 100, kPETypeData, kPETypeBSS };

using PE_SECTION_INFO = LDR_SECTION_HEADER;

/// \brief Code Manager process creation functions.

ProcessID rtl_create_user_process(PE32Loader&                        exec,
                                  const UserProcess::ExecutableKind& process_kind);

ProcessID rtl_kernel_user_process(PE32Loader&                        exec,
                                  const UserProcess::ExecutableKind& process_kind);

}  // namespace Ne::Kernel

#endif
