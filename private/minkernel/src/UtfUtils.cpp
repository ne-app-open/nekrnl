// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <NeKit/Utils.h>

/// @author Amlal El Mahrouss (amlal@nekernel.org)

namespace Ne::Kernel {

Size urt_string_len(const Utf8Char* str) {
  if (!str) return 0;
  if (!*str) return 0;

  SizeT len{0};

  while (str[len] != u8'\0') ++len;

  return len;
}

Void urt_set_memory(const voidPtr src, UInt32 dst, Size len) {
  if (!src) return;

  Utf8Char* srcChr = reinterpret_cast<Utf8Char*>(src);
  Size      index  = 0;

  while (index < len) {
    srcChr[index] = dst;
    ++index;
  }
}

Int32 urt_string_cmp(const Utf8Char* src, const Utf8Char* cmp, Size size) {
  if (!src) return 0;

  Int32 counter = 0;

  for (Size index = 0; index < size; ++index) {
    if (src[index] != cmp[index]) ++counter;
  }

  return counter;
}

Int32 urt_copy_memory(const VoidPtr src, VoidPtr dst, Size len) {
  if (!src) return 0;
  if (!dst) return 0;

  Utf8Char* srcChr  = reinterpret_cast<Utf8Char*>(src);
  Utf8Char* dstChar = reinterpret_cast<Utf8Char*>(dst);

  Size index = 0;

  while (index < len) {
    dstChar[index] = srcChr[index];
    ++index;
  }

  return index;
}

}  // namespace Ne::Kernel
