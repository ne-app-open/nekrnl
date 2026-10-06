// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <SystemKit/Err.h>
#include <SystemKit/Syscall.h>
#include <SystemKit/System.h>
#include <SystemKit/Verify.h>

using namespace System;

/// @note This uses the FNV 64-bit variant.
IMPORT_C UInt64 nesys_hash_64(const Char* path) {
  if (!path || *path == 0) return 0;

  const UInt64 kFNVSeed  = 0xcbf29ce484222325ULL;
  const UInt64 kFNVPrime = 0x100000001b3ULL;

  UInt64 hash = kFNVSeed;

  MUST_PASS(hash != 0);

  while (*path) {
    hash ^= (Char) (*path++);
    hash *= kFNVPrime;
  }

  return hash;
}
