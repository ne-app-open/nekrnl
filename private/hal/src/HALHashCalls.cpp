// SPDX-License-Identifier: Apache-2.0
// Copyright 2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

// For FNV hashing.
#include <hal/HAL/HAL.h>

/// @brief FNV-1a hash function for strings (64-bit version).
/// @note This function is C exported for assembly usage.
EXTERN_C UInt64 hali_hash_fnv64(const Char* path) {
  if (!path || *path == 0) return 0;

  const UInt64 kFNVSeed  = 0xcbf29ce484222325ULL;
  const UInt64 kFNVPrime = 0x100000001b3ULL;

  // FNV-1a hash algorithm.
  UInt64 hash = kFNVSeed;

  while (*path) {
    hash ^= (Char) (*path++);
    hash *= kFNVPrime;
  }

  // Ensure that the hash is not zero, as zero is reserved for invalid entries.
  MUST_PASS(hash != 0);

  return hash;
}

EXTERN_C UInt64 hali_hash_fnv64_utf8(const Utf8Char* path) {
  if (!path || *path == 0) return 0;

  const UInt64 kFNVSeed  = 0xcbf29ce484222325ULL;
  const UInt64 kFNVPrime = 0x100000001b3ULL;

  // FNV-1a hash algorithm.
  UInt64 hash = kFNVSeed;

  while (*path) {
    hash ^= (Utf8Char) (*path++);
    hash *= kFNVPrime;
  }

  // Ensure that the hash is not zero, as zero is reserved for invalid entries.
  MUST_PASS(hash != 0);

  return hash;
}
