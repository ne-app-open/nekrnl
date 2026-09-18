// SPDX-License-Identifier: Apache-2.0
// Copyright 2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

/// For entries
#include <ArchKit/ArchKit.h>

// For common HAL routines
#include <hal/HAL/HAL.h>

static Ne::Kernel::Array<HAL_DISPATCH_ENTRY, kMaxDispatchCallCount> kDispatchCalls;

EXTERN_C Ne::Kernel::SSizeT hal_install_dispatch(const Ne::Kernel::Char* name,
                                                 rt_syscall_proc         proc) {
  if (!name || *name == 0) return -1;

  auto hash = ke_hash_64(name);

  if (!hash || !proc) return -1;

  for (Ne::Kernel::SizeT i = 0UL; i < kMaxDispatchCallCount; ++i) {
    if (kDispatchCalls[i].fHooked) continue;

    STATIC std::atomic_flag kLocked = ATOMIC_FLAG_INIT;

    while (kLocked.test_and_set(std::memory_order_acquire));

    kDispatchCalls[i].fHash   = hash;
    kDispatchCalls[i].fProc   = proc;
    kDispatchCalls[i].fHooked = YES;

    kLocked.clear(std::memory_order_release);

    return i;
  }

  return -1;
}

/// Interrupt handler for HAL.dll
EXTERN_C Ne::Kernel::Void hal_call_enter(Ne::Kernel::UIntPtr rcx_hash, Ne::Kernel::UIntPtr arg) {
  if (!arg || !rcx_hash) return;

  for (SizeT i = 0UL; i < kMaxDispatchCallCount; ++i) {
    if (kDispatchCalls[i].fHooked && rcx_hash == kDispatchCalls[i].fHash) {
      if (kDispatchCalls[i].fProc) {
        (kDispatchCalls[i].fProc)((Ne::Kernel::VoidPtr) arg);
      }
    }
  }
}
