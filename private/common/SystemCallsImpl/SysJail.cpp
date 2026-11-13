// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-open/nekrnl

#include <NeKit/NeKit.h>

#ifndef kNeMaxJailListLen
#define kNeMaxJailListLen (128)
#endif

#ifndef IMPORT_C
#define IMPORT_C extern "C"
#endif

using namespace ::Ne::Kernel;

/////////////////////////////////////////////////////////////////////////////////////

struct JAIL_INFO;
struct JAIL;

/// @brief Jail information (client side struct)
struct JAIL_INFO final {
  Int32 fParentID;
  Int32 fJailHash;
  Int64 fACL;
};

/// @brief Jail information (we grab a JAIL from JailGetCurrent())
struct JAIL final {
  struct JAIL_INFO* fServer;
  struct JAIL_INFO* fClient;
  Int32            fJailHash;
  Int32            fParentID;
  Int64            fACL;
};

/////////////////////////////////////////////////////////////////////////////////////

STATIC ATTRIBUTE(unused)
JAIL* kJailExecutables[kNeMaxJailListLen];

STATIC ATTRIBUTE(unused)
JAIL*  kJailDLL[kNeMaxJailListLen];

STATIC       ATTRIBUTE(unused)
JAIL*        kJailHostsExecutables[kNeMaxJailListLen];

STATIC       ATTRIBUTE(unused)
JAIL*        kJailHostsDLL[kNeMaxJailListLen];

STATIC       ATTRIBUTE(unused)
JAIL*  kJailCurrent{nullptr};

IMPORT_C JAIL* _JailGetCurrent(Void) {
  if (!kJailCurrent) {
    MUST_PASS(kJailCurrent); // Every process shall have a jail, fail here.
    return nullptr;
  }

  return kJailCurrent;
}

IMPORT_C Void _JailSetCurrentExe(JAIL* jail) {
  MUST_PASS(jail);
  MUST_PASS(jail->fJailHash != 0);
  MUST_PASS(kJailExecutables[jail->fJailHash] == jail);

  if (jail) kJailCurrent = jail;
}

IMPORT_C Void _JailSetCurrentDLL(JAIL* jail) {
  MUST_PASS(jail);
  MUST_PASS(jail->fJailHash != 0);
  MUST_PASS(kJailDLL[jail->fJailHash] == jail);

  if (jail) kJailCurrent = jail;
}

/// @brief Hosts jail shall be exposed by another private API.
