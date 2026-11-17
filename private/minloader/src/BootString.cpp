// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <BootKit/BootKit.h>
#include <BootKit/Platform.h>
#include <BootKit/Protocol.h>

/// BUGS: 0

/////////////////////////////////////////////////////////////////////////////////////////////////////////

SizeT Boot::BCopyMem(CharacterTypeUTF16* dest, CharacterTypeUTF16* src,
                             const SizeT len) {
  if (!dest || !src) return 0;

  SizeT index = 0UL;
  for (; index < len; ++index) {
    dest[index] = src[index];
  }

  return index;
}

SizeT Boot::BStrLen(const CharacterTypeUTF16* ptr) {
  if (!ptr) return 0;

  SizeT cnt = 0;

  while (*ptr != (CharacterTypeUTF16) 0) {
    ++ptr;
    ++cnt;
  }

  return cnt;
}

SizeT Boot::BSetMem(CharacterTypeUTF16* src, const CharacterTypeUTF16 byte,
                            const SizeT len) {
  if (!src) return 0;

  SizeT cnt = 0UL;

  while (*src != 0) {
    if (cnt > len) break;

    *src = byte;
    ++src;

    ++cnt;
  }

  return cnt;
}

SizeT Boot::BSetMem(CharacterTypeASCII* src, const CharacterTypeASCII byte,
                            const SizeT len) {
  if (!src) return 0;

  SizeT cnt = 0UL;

  while (*src != 0) {
    if (cnt > len) break;

    *src = byte;
    ++src;

    ++cnt;
  }

  return cnt;
}

// EOF
