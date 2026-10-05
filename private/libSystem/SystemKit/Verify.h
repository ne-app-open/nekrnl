// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifndef SYSTEMKIT_VERIFY_H
#define SYSTEMKIT_VERIFY_H

#include <SystemKit/System.h>

#ifndef voidptr_cast
#define voidptr_cast(x) static_cast<VoidPtr>(x)
#endif

namespace System::Verify {

/// @author 0xf00sec, and Amlal El Mahrouss
/// @brief safe cast operator.
template <class T, class R>
inline R* sys_safe_cast(const T* ptr) {
  ::_rtl_assert(ptr, "safe cast failed!");
  return static_cast<R*>(const_cast<T*>(ptr));
}

template <class T, class U>
struct is_castable _FINAL {
  using return_type = U;

  constexpr static BOOL value = false;
};

template <class T>
struct is_castable<T, T> _FINAL {
  using return_type = T;

  constexpr static BOOL value = true;
};

/// @author Amlal El Mahrouss
/// @brief Safe constexpr cast.
template <typename T, typename R>
constexpr R* sys_constexpr_cast(T* ptr) {
  static_assert(is_castable<T, R>::value, "types cannot be casted.");
  return static_cast<R*>(ptr);
}

}  // namespace System::Verify

#endif
