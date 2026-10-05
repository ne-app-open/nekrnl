// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifndef KERNELKIT_LOCKDELEGATE_H
#define KERNELKIT_LOCKDELEGATE_H

#include <NeKit/Atom.h>
#include <NeKit/Config.h>

namespace Ne::Kernel {
enum : int {
  kLockInvalid  = 0,
  kLockDone     = 200,
  kLockTimedOut = 300,
  kLockCount    = 3,
};

/// @brief Lock condition pointer.
typedef Boolean* LockPtr;

/// @brief Locking delegate class, hangs until limit.
/// @tparam N the amount of cycles to wait.
template <SizeT N>
class LockDelegate final {
 public:
  LockDelegate() = delete;

 public:
  explicit LockDelegate(LockPtr expr) {
    auto spin = 0U;

    while (spin < N) {
      if (*expr) {
        fLockStatus | kLockDone;
        break;
      }

      ++spin;
    }

    if (spin > N) fLockStatus | kLockTimedOut;
  }

  ~LockDelegate() = default;

  LockDelegate& operator=(const LockDelegate&) = delete;
  LockDelegate(const LockDelegate&)            = delete;

  Bool Done() { return fLockStatus[kLockDone] == kLockDone; }

  Bool HasTimedOut() { return fLockStatus[kLockTimedOut] != kLockTimedOut; }

 private:
  Atom<UInt> fLockStatus;
};
}  // namespace Ne::Kernel

#endif
