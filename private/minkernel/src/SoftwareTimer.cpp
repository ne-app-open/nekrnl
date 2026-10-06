// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <KernelKit/Timer.h>

/// ================================================================================
/// @note The ITimer class is meant to be generic.
///! @author Amlal El Mahrouss (amlal@nekernel.org)
/// ================================================================================

using namespace Ne::Kernel;

SoftwareTimer::SoftwareTimer(UInt64 seconds) : fWaitFor(seconds) {
  fDigitalTimer = new UIntPtr();
  MUST_PASS(fDigitalTimer);
}

SoftwareTimer::~SoftwareTimer() {
  if (fDigitalTimer) delete fDigitalTimer;
  fDigitalTimer = nullptr;

  fWaitFor = 0;
}

BOOL SoftwareTimer::Wait() {
  if (fWaitFor < 1L) return NO;
  MUST_PASS(fDigitalTimer);

  while (*fDigitalTimer < (*fDigitalTimer + fWaitFor)) {
    ++(*fDigitalTimer);
  }

  return YES;
}
