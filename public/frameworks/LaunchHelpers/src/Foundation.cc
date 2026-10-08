// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <LaunchHelpers/headers/Foundation.h>
#include <SystemKit/Syscall.h>

/// @brief Get launch information.
/// @return the launch information structure.
CF::CFRef<LaunchHelpers::LHLaunchInfo> LaunchHelpers::LHGetLaunchInfo(Void) {
  return static_cast<LaunchHelpers::LHLaunchInfo*>(
      nesys_syscall_arg_1(nesys_hash_64("__LHGetLaunchInfoRef")));
}
