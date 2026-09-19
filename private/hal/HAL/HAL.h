// SPDX-License-Identifier: Apache-2.0
// Copyright 2026, Ne.app, all rights reserved.
// Official repository: https://github.com/ne-app-open/krnl

#ifndef HAL_HAL_H
#define HAL_HAL_H

#include <ArchKit/ArchKit.h>

#ifndef HAL_UNUSED
#define HAL_UNUSED(X) (Void) X
#endif

#ifndef HAL_FINAL
#define HAL_FINAL final
#endif

// @brief HAL architecture. This is used to determine the compatibility of the HAL with the kernel.

#ifdef __NE_AMD64__

#ifndef kHalCurrentArchitecture
#define kHalCurrentArchitecture (0x8664)
#endif

#ifndef kHalCurrentSubArchitecture
#define kHalCurrentSubArchitecture (0x0001)
#endif

#elif defined(__NE_ARM64__)

#ifndef kHalCurrentArchitecture
#define kHalCurrentArchitecture (0xA864)
#endif

#ifndef kHalCurrentSubArchitecture
#define kHalCurrentSubArchitecture (0x0001)
#endif

#else

#error !!! Unsupported architecture !!!

#endif

#ifdef __cplusplus
namespace Ne::Kernel::HAL {
#endif

struct HAL_CALL_ENTRY;

#ifdef __cplusplus
using hal_proc_type = Ne::Kernel::Void (*)(Ne::Kernel::VoidPtr);
#else
typedef VoidPtr hal_proc_type;
#endif

enum : SInt64 { kAuthLevelInvalid, kAuthLevelHigh = 100, kAuthLevelMid, kAuthLevelLow };

/// @brief Entry structure of the HAL dispatch table.
/// @authors Amlal El Mahrouss (amlal@nekernel.org, amlal@ne-app.eu)

struct PACKED ALIGN(8) HAL_CALL_ENTRY HAL_FINAL {
  UInt64        fHash{};
  Bool          fActive{};
  hal_proc_type fProc;
  SInt64        fAuthLevel{};

#ifdef __cplusplus
  BOOL     IsActive() { return fActive; }
  explicit operator bool() { return fActive; }
#endif
};

#ifdef __cplusplus
EXTERN_C SInt32 hali_install_entry(HAL::hal_proc_type, const UInt64, const UInt64 hash);
EXTERN_C Void hali_remove_entry(const UInt64 hash);
#else
EXTERN_C SInt32 hali_install_entry(HAL::hal_proc_type, const UInt64, const SInt64 hash);
EXTERN_C Void hali_remove_entry(const SInt64 hash);
#endif

EXTERN_C SSizeT hal_install_dispatch(const Char* name, rt_syscall_proc proc);
EXTERN_C UInt64 hali_hash_fnv64(const Char* path);

#ifdef __cplusplus
inline Array<HAL_CALL_ENTRY, kMaxDispatchCallCount> kRegisteredSystemCalls;
#endif

#ifdef __cplusplus
}  // namespace Ne::Kernel::HAL
#endif

#endif  // HAL_HAL_H