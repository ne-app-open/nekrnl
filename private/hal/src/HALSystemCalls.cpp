// SPDX-License-Identifier: Apache-2.0
// Copyright 2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

/// For Entries Support.
#include <ArchKit/ArchKit.h>
#include <hal/HAL/HAL.h>

using Ne::Kernel::HAL::kRegisteredSystemCalls;

/// @brief Add a new entry to the HAL system calls table.
EXTERN_C SInt32 hali_install_entry(HAL::hal_proc_type proc, const UInt64 level, const UInt64 hash) {
  if (!hash) return -1;
  if (!proc) return -1;

  STATIC std::atomic_flag kLocked = ATOMIC_FLAG_INIT;

  while (kLocked.test_and_set(std::memory_order_acquire));

  // Calculate the index in the system calls table based on the hash.
  auto i = hash % kMaxDispatchCallCount;

  if (i > kMaxDispatchCallCount) {
    kLocked.clear(std::memory_order_release);
    return -1;
  }
  if (kRegisteredSystemCalls[i].fActive) {
    kLocked.clear(std::memory_order_release);
    return -1;
  }

  kRegisteredSystemCalls[i].fHash      = hash;
  kRegisteredSystemCalls[i].fProc      = proc;
  kRegisteredSystemCalls[i].fActive    = YES;
  kRegisteredSystemCalls[i].fAuthLevel = level;

  kLocked.clear(std::memory_order_release);

  return (SInt32)i;
}

/// @brief Remove an entry from the HAL system calls table.
EXTERN_C Void hali_remove_entry(const UInt64 hash) {
  if (!hash) return;

  STATIC std::atomic_flag kLocked = ATOMIC_FLAG_INIT;

  while (kLocked.test_and_set(std::memory_order_acquire));

  // Calculate the index in the system calls table based on the hash.
  auto i = hash % kMaxDispatchCallCount;

  if (i > kMaxDispatchCallCount) {
    kLocked.clear(std::memory_order_release);
    return;
  }
  if (kRegisteredSystemCalls[i].fHash != hash) {
    kLocked.clear(std::memory_order_release);
    return;
  }

  kRegisteredSystemCalls[i].fProc      = nullptr;
  kRegisteredSystemCalls[i].fHash      = 0;
  kRegisteredSystemCalls[i].fActive    = NO;
  kRegisteredSystemCalls[i].fAuthLevel = 0;

  kLocked.clear(std::memory_order_release);
}
