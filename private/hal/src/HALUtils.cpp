// SPDX-License-Identifier: Apache-2.0
// Copyright 2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <ArchKit/ArchKit.h>
#include <hal/HAL/HAL.h>

/// AMLALE: Introduce the HAL calls here.

#ifndef kHalBalancedStr
#define kHalBalancedStr "balanced"
#endif

#ifndef kHalLowPowStr
#define kHalLowPowStr "low-power"
#endif

#ifndef kHalPerformanceStr
#define kHalPerformanceStr "performance"
#endif

#ifndef kNameLen
#define kNameLen (128)
#endif

STATIC Char kPowerProfileStr[kNameLen] = {"invalid"};

EXTERN_C Void hal_stop_runtime(Void) { CANT_REACH(); }

EXTERN_C const Char* hali_acpi_power_profile(Void) { return kPowerProfileStr; }

Void ::Ne::Kernel::ke_runtime_check(BOOL expr, const Char*, const Char*) {
  if (!expr) {
    hal_stop_runtime();
  }
}

// EOF
