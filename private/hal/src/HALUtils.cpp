// SPDX-License-Identifier: Apache-2.0
// Copyright 2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

/// For entries
#include <ArchKit/ArchKit.h>

// For common HAL routines
#include <hal/HAL/HAL.h>

/// AMLALE: Introduce the HAL calls here.

EXTERN_C const Char* hali_acpi_power_profile(Void);
EXTERN_C const Char* hali_set_acpi_power_profile(Void);
