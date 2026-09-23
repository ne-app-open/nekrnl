// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifndef KERNELKIT_SEMAPHORE_H
#define KERNELKIT_SEMAPHORE_H

/// @author Amlal El Mahrouss
/// @file Semaphore.h
/// @brief Semaphore structure and functions for synchronization in the kernel.

#include <CompilerKit/CompilerKit.h>
#include <KernelKit/Timer.h>
#include <NeKit/Config.h>

#define kSemaphoreOwnerIndex (0U)
#define kSemaphoreCountIndex (1U)

#define kSemaphoreCount (2U)

#define kSemaphoreIncrementOwner(sem) (++sem[kSemaphoreOwnerIndex])
#define kSemaphoreDecrementOwner(sem) (--sem[kSemaphoreOwnerIndex])

namespace Ne::Kernel {

/// @brief Semaphore structure used for synchronization.
using SemaphoreArr = UInt64[kSemaphoreCount];

/// @brief Checks if the semaphore is valid.
inline Bool rtl_sem_is_valid(const SemaphoreArr& sem, const UInt64& owner = 0) {
  //if (!sem) return false;
  return sem[kSemaphoreOwnerIndex] == owner && sem[kSemaphoreCountIndex] > 0;
}

/// @brief Releases the semaphore, resetting its owner and count.
/// @param sem
/// @return
inline Bool rtl_sem_release(SemaphoreArr& sem) {
  //if (!sem) return false;

  sem[kSemaphoreOwnerIndex] = 0;
  sem[kSemaphoreCountIndex] = 0;

  return true;
}

/// @brief Initializes the semaphore with an owner and a count of zero.
/// @param sem the semaphore array to use.
/// @param owner the owner to set, could be anything identifitable.
/// @return
inline Bool rtl_sem_acquire(SemaphoreArr& sem, const UInt64& owner) {
  //if (!sem) return false;

  if (!owner) {
    err_global_get() = kErrorInvalidData;
    return false;  // Invalid owner, return false and set KPC.
  }

  sem[kSemaphoreOwnerIndex] = owner;
  sem[kSemaphoreCountIndex] = 0;

  return true;
}

/// @brief Waits for the semaphore to be available, blocking until it is.
/// @param sem
/// @param timeout
/// @param condition condition pointer.
/// @return
inline Bool rtl_sem_wait(SemaphoreArr& sem, const UInt64& owner, const UInt64& timeout,
                         Bool& condition) {
  //if (!sem) return false;

  if (!rtl_sem_is_valid(sem, owner)) {
    err_global_get() = kErrorInvalidData;
    return false;
  }

  if (timeout == 0) {
    err_global_get() = kErrorTimeout;
    return false;
  }

  if (!condition) {
    if (sem[kSemaphoreCountIndex] == 0) {
      err_global_get() = kErrorUnavailable;
      return false;
    }

    err_global_get() = kErrorSuccess;
    sem[kSemaphoreCountIndex]--;

    return true;
  }

#ifdef __NE_USE_HW_TIMER__
  HardwareTimer timer(timeout);
#else
  SoftwareTimer timer(timeout);
#endif

  Bool          ret = timer.Wait();

  if (ret) {
    if (!condition) {
      if (sem[kSemaphoreCountIndex] == 0) {
        err_global_get() = kErrorUnavailable;
        return false;
      }

      err_global_get() = kErrorSuccess;
      sem[kSemaphoreCountIndex]--;

      return true;
    }
  }

  err_global_get() = kErrorTimeout;

  return false;  // Failed to acquire semaphore
}

}  // namespace Ne::Kernel

#endif  // !KERNELKIT_SEMAPHORE_H
