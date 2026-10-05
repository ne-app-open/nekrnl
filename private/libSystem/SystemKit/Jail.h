// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifndef SYSTEMKIT_JAIL_H
#define SYSTEMKIT_JAIL_H

#include <SystemKit/System.h>

/// @file Jail.h
/// @author Amlal El Mahrouss
/// @brief NeKernel Jail System, part of OpenEnclave.

struct JAIL_INFO;
struct JAIL;

/// @brief Jail information (client side struct)
struct JAIL_INFO _FINAL {
  SInt32 fParentID;
  SInt32 fJailHash;
  SInt64 fACL;
};

/// @brief Jail information (we grab a JAIL from JailGetCurrent())
struct JAIL _FINAL {
  struct JAIL_INFO* fServer;
  struct JAIL_INFO* fClient;
  SInt32            fJailHash;
  SInt32            fParentID;
  SInt64            fACL;
};

/// @brief Get the current jail
/// @return Pointer to the current jail structure, or NULL if not in a jail
IMPORT_C struct JAIL* JailGetCurrent(Void);
IMPORT_C Void JailSetCurrent(struct JAIL* ptr);

#endif
