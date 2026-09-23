// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <KernelKit/DebugOutput.h>
#include <KernelKit/IPE32DylibObject.h>
#include <KernelKit/PE.h>
#include <KernelKit/ProcessScheduler.h>
#include <KernelKit/ThreadLocalStorage.h>
#include <NeKit/Config.h>

#define kPeStackSizeSymbol "__NESizeOfReserveStack"
#define kPeHeapSizeSymbol "__NESizeOfReserveHeap"
#define kPeNameSymbol "__NEProgramName"
#define kPeImageStart "__ImageStart"

namespace Ne::Kernel {

/***********************************************************************************/
/// @file IPE32DylibObject.cpp
/// @brief PE32's Dylib runtime.
///! @author Amlal El Mahrouss (amlal@nekernel.org)
/***********************************************************************************/

/***********************************************************************************/
/** @brief Library initializer. */
/***********************************************************************************/

EXTERN_C IDylibRef rtl_init_dylib_pe32(UserProcess& process) {
  IDylibRef dll_obj = tls_new_class<IPE32DylibObject>();

  if (!dll_obj) {
    process.Crash();
    return nullptr;
  }

  auto traits = new IPE32DylibObject::DylibTraits();

  if (!traits) {
    tls_delete_class(dll_obj);
    process.Crash();

    return nullptr;
  }

  traits->ImageObject = process.Image.LeakBlob().Leak().Leak();
  traits->ImageSz     = process.Image.LeakBlobSz();

  dll_obj->Mount(traits);

  if (!dll_obj->Get()) {
    delete traits;

    tls_delete_class(dll_obj);
    dll_obj = nullptr;

    process.Crash();

    return nullptr;
  }

  dll_obj->Get()->ImageEntrypointOffset =
      dll_obj->Load<VoidPtr>(kPeImageStart, rt_string_len(kPeImageStart, 0), kPETypeText);

  return dll_obj;
}

/***********************************************************************************/
/** @brief Frees the dll_obj. */
/** @note Please check if the dll_obj got freed! */
/** @param dll_obj The dll_obj to free. */
/** @param successful Reports if successful or not. */
/***********************************************************************************/

EXTERN_C Void rtl_fini_dylib_pe32(UserProcess& process, IDylibRef dll_obj, BOOL* successful) {
  MUST_PASS(successful);

  if (!successful) {
    return;
  }

  // sanity check (will also trigger a bug check if this fails)
  if (dll_obj == nullptr) {
    *successful = false;
    process.Crash();
  }

  delete dll_obj->Get();
  delete dll_obj;

  dll_obj = nullptr;

  *successful = true;
}

}  // namespace Ne::Kernel
