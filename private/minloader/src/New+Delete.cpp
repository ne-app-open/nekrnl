// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <BootKit/BootKit.h>
#include <BootKit/Platform.h>
#include <BootKit/Protocol.h>
#include <FirmwareKit/EFI/API.h>

#ifdef __BOOTZ_STANDALONE__

/// @brief Allocates a new object.
/// @param sz the size.
/// @return
void* operator new(size_t sz) {
  if (sz == 0) ++sz;

  void* buf = nullptr;

  while (BS->AllocatePool(EfiMemoryType::EfiLoaderData, sz, &buf) != kEfiOk);

  return buf;
}

/// @brief Allocates a new object.
/// @param sz the size.
/// @return
void* operator new[](size_t sz) {
  if (sz == 0) ++sz;

  void* buf = nullptr;
  BS->AllocatePool(EfiMemoryType::EfiLoaderData, sz, &buf);

  return buf;
}

/// @brief Deletes the object.
/// @param buf the object.
void operator delete(void* buf) noexcept {
  if (!buf) return;

  BS->FreePool(buf);
}

/// @brief Deletes the object.
/// @param buf the object.
void operator delete[](void* buf) noexcept {
  if (!buf) return;

  BS->FreePool(buf);
}

/// @brief Deletes the object (array specific).
/// @param buf the object.
/// @param size it's size.
void operator delete(void* buf, size_t size) noexcept {
  if (!buf) return;

  SetMem(buf, 0, size);
  BS->FreePool(buf);
}

/// @brief Deletes the object (array specific).
/// @param buf the object.
/// @param size it's size.
void operator delete[](void* buf, size_t size) noexcept {
  if (!buf) return;

  SetMem(buf, 0, size);
  BS->FreePool(buf);
}

#endif  // __BOOTZ_STANDALONE__

// EOF
