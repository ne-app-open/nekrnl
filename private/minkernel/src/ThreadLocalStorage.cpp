// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <CFKit/Property.h>
#include <KernelKit/ProcessScheduler.h>
#include <KernelKit/ThreadLocalStorage.h>
#include <NeKit/KString.h>

/***********************************************************************************/
/// @bugs: 0
/// @file ThreadLocalStorage.cc
/// @brief NeKernel Thread Local Storage.
///! @author Amlal El Mahrouss (amlal@nekernel.org)
/***********************************************************************************/

namespace Ne::Kernel {

/**
 * @brief Checks for cookie inside the TIB.
 * @param tib_ptr the TIB to check.
 * @return if the cookie is enabled, true; false otherwise
 */

Boolean tlsi_check_tib(THREAD_INFORMATION_BLOCK* tib_ptr) {
  MUST_PASS(tib_ptr);
  if (!tib_ptr) return false;

  return tib_ptr->Cookie[kCookieMag0Idx] == kCookieMag0 &&
         tib_ptr->Cookie[kCookieMag1Idx] == kCookieMag1 &&
         tib_ptr->Cookie[kCookieMag2Idx] == kCookieMag2;
}

}  // namespace Ne::Kernel

/**
 * @brief Implementation of the TLS check.
 * @param tib_ptr The TIB record.
 * @return if the TIB record is valid or not.
 */
EXTERN_C Bool tls_check_tib(Ne::Kernel::VoidPtr tib_ptr) {
  MUST_PASS(tib_ptr);
  if (!tib_ptr) {
    kout << "TLS: Failed because of an invalid TIB...\r";
    return NO;
  }

  THREAD_INFORMATION_BLOCK* tib = static_cast<THREAD_INFORMATION_BLOCK*>(tib_ptr);
  if (!tib) return NO;

  return Ne::Kernel::tlsi_check_tib(tib);
}
