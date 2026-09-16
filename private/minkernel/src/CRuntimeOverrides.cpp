// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <NeKit/Utils.h>

using namespace Ne::Kernel;

/// @brief Standard NeKernel Size type.
using ksz_t = long long unsigned int;

/// =========================================================== ///
/// @brief C Standard Library overrides.                     ///
/// =========================================================== ///

EXTERN_C void* memset(void* dst, int c, ksz_t len) {
  return Ne::Kernel::rt_set_memory_safe(dst, c, static_cast<Size>(len), static_cast<Size>(len));
}

EXTERN_C void* memcpy(void* dst, const void* src, ksz_t len) {
  Ne::Kernel::rt_copy_memory_safe(const_cast<void*>(src), dst, static_cast<Size>(len),
                              static_cast<Size>(len));
  return dst;
}

EXTERN_C Int32 strcmp(const Char* a, const Char* b) {
  return Ne::Kernel::rt_string_cmp(a, b, rt_string_len(a));
}
