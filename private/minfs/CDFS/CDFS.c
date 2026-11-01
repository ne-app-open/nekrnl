// SPDX-License-Identifier: Apache-2.0
// Copyright 2026, Ne.app
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <modules/CDFS/CDFS.h>

/// @note Output file name is cdfs.exe

DDK_EXTERN void KDriverMain(void) {
  if (ke_call_dispatch("_HalIsCdfsDrvInstalled", 0, NULL, 0))
    return;

  ke_call_dispatch("_HalCdfsDrvInstall", 0, NULL, 0);
}

// END
