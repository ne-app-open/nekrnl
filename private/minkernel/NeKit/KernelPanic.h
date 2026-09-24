// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifndef NEKIT_KERNELPANIC_H
#define NEKIT_KERNELPANIC_H

#include <NeKit/Config.h>

/// @brief Checks during compile time whether a condition passes.
#define STATIC_PASS(EXPR, MSG) static_assert(EXPR, MSG)

#ifdef TRY
#undef TRY
#endif

#define TRY(X)            \
  {                       \
    auto FN__ = X;        \
    if ((FN__()) == NO) { \
      MUST_PASS(NO);      \
    }                     \
  }

#ifdef __MUST_PASS
#undef __MUST_PASS
#endif

#define __MUST_PASS(EXPR, FILE, LINE) Ne::Kernel::ke_runtime_check(EXPR, FILE, STRINGIFY(LINE))

#ifdef __DEBUG__
#define MUST_PASS(EXPR) __MUST_PASS((EXPR), __FILE__, __LINE__)
#define assert(EXPR) MUST_PASS(EXPR)
#else
#define MUST_PASS(EXPR) __MUST_PASS(EXPR, __FILE__, __LINE__)
#define assert(EXPR) __MUST_PASS(EXPR, __FILE__, __LINE__)
#endif

enum RUNTIME_CHECK {
  RUNTIME_CHECK_FAILED = 1111,
  RUNTIME_CHECK_POINTER,
  RUNTIME_CHECK_EXPRESSION,
  RUNTIME_CHECK_FILE,
  RUNTIME_CHECK_IPC,
  RUNTIME_CHECK_TLS,
  RUNTIME_CHECK_HANDSHAKE,
  RUNTIME_CHECK_ACPI,
  RUNTIME_CHECK_INVALID_PRIVILEGE,
  RUNTIME_CHECK_PROCESS,
  RUNTIME_CHECK_BAD_BEHAVIOR,
  RUNTIME_CHECK_BOOTSTRAP,
  RUNTIME_CHECK_UNEXCPECTED,
  RUNTIME_CHECK_FILESYSTEM,
  RUNTIME_CHECK_VIRTUAL_OUT_OF_MEM,
  RUNTIME_CHECK_PAGE,
  RUNTIME_CHECK_INTERRUPT,
  RUNTIME_CHECK_NETWORK,
  RUNTIME_CHECK_INVALID,
  RUNTIME_CHECK_COUNT,
};

typedef enum RUNTIME_CHECK RTL_RUNTIME_CHECK;

namespace Ne::Kernel {

/// @brief Raises a runtime-check for the system, it failing, the system will raise a panic.
void ke_runtime_check(bool expr, const Char* file, const Char* line);

/// @brief Stops the system from running when unrecoverable.
void ke_stop(const Int32& id, const Char* message = nullptr);

}  // namespace Ne::Kernel

#endif
