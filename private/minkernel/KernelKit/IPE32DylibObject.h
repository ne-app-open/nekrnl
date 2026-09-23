// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifndef KERNELKIT_PE32_SHARED_OBJECT_H
#define KERNELKIT_PE32_SHARED_OBJECT_H

#include <KernelKit/IDylibObject.h>
#include <KernelKit/PE.h>
#include <KernelKit/PE32CodeMgr.h>
#include <KernelKit/ProcessScheduler.h>
#include <NeKit/Config.h>

namespace Ne::Kernel {

/**
 * @brief Shared Library class
 * Load library from this class
 */
class IPE32DylibObject final NE_DYLIB_OBJECT {
 public:
  explicit IPE32DylibObject() = default;
  ~IPE32DylibObject()         = default;

 public:
  NE_COPY_DEFAULT(IPE32DylibObject)

  using DylibTraitsPtr   = DylibTraits*;
  using DylibTraitsPtrX2 = DylibTraitsPtr*;

 private:
  DylibTraitsPtr fMounted{nullptr};

 public:
  DylibTraitsPtrX2 GetAddressOf() { return &fMounted; }

  DylibTraitsPtr Get() { return fMounted; }

 public:
  void Mount(DylibTraitsPtr to_mount) {
    if (!to_mount) return;
    if (!to_mount->ImageObject) return;

    fMounted = to_mount;

    if (fLoader && to_mount) {
      delete fLoader;
      fLoader = nullptr;
    }

    if (!fLoader) {
      fLoader = new PE32Loader(fMounted->ImageObject, fMounted->ImageSz);
    }
  }

  void Unmount() {
    if (fMounted) fMounted = nullptr;
  };

  template <typename SymbolType>
  SymbolType Load(const Char* symbol_name, const SizeT& len, const UInt32& kind) {
    if (symbol_name == nullptr || *symbol_name == 0) return nullptr;
    if (len > kPathLen || len < 1) return nullptr;

    auto ret = static_cast<SymbolType>(fLoader->FindSymbol(symbol_name, kind).Leak().Leak());

    if (!ret) {
      if (kind == kPETypeText) return (VoidPtr) &__ne_pure_call;

      return nullptr;
    }

    return ret;
  }

 private:
  using LoaderType = PE32Loader;

  LoaderType* fLoader{nullptr};
};

using IDylibRef = IPE32DylibObject*;

EXTERN_C IDylibRef rtl_init_dylib_pe32(UserProcess& header);
EXTERN_C Void      rtl_fini_dylib_pe32(UserProcess& header, IDylibRef lib, Bool* successful);

}  // namespace Ne::Kernel

#endif /* ifndef __KERNELKIT_PE32_SHARED_OBJECT_H__ */
