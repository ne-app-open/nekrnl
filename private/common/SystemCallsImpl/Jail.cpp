// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-open/nekrnl

#include <SystemKit/Jail.h>
#include <SystemKit/Macros.h>

#ifndef kNeMaxJailListLen
#define kNeMaxJailListLen (128)
#endif

STATIC JAIL* kJailExecutables[kNeMaxListLen];
STATIC JAIL* kJailDLL[kNeMaxListLen];

STATIC JAIL* kJailHostsExecutables[kNeMaxListLen];
STATIC JAIL* kJailHostsDLL[kNeMaxListLen];

IMPORT_C JAIL* JailGetCurrent(Void) {
  return nullptr;
}
