// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-open/nekrnl

#include <SystemKit/Jail.h>
#include <SystemKit/Syscall.h>

IMPORT_C struct JAIL* JailGetCurrent(Void) {
  auto ptr = nesys_syscall_arg_1(SYSCALL_HASH("JailGetCurrent"));

#ifdef _DEBUG
  _rtl_assert(ptr != nullptr, "JailGetCurrent: Jail pointer is null");
#endif

  if (!ptr) return nullptr;

  return (struct JAIL*) ptr;
}
