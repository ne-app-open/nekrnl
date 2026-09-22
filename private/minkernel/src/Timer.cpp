// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <KernelKit/Timer.h>

///! BUGS: 0
///! @file Timer.cc
///! @brief Software Timer implementation
///! @author Amlal El Mahrouss (amlal@nekernel.org)

namespace Ne::Kernel {

/// @brief Unimplemented as it is an interface.
BOOL ITimer::Wait() {
  return NO;
}

}  // namespace Ne::Kernel