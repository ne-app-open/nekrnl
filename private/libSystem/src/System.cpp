// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <SystemKit/Err.h>
#include <SystemKit/Syscall.h>
#include <SystemKit/System.h>
#include <SystemKit/Verify.h>

using namespace System;

IMPORT_C Char* StrFmt(const Char* fmt, ...) {
  if (!fmt || *fmt == 0) return const_cast<Char*>("(null)");
  return const_cast<Char*>("(null)");
}

// memmove-style copy
IMPORT_C VoidPtr MmCopyMemory(_Input VoidPtr dest, _Input VoidPtr src, _Input UInt64 len) {
  // handles overlap, prefers 64-bit word copies when aligned
  if (!len || !dest || !src) return nullptr;

  auto s = static_cast<const UInt8*>(src);
  auto d = static_cast<UInt8*>(dest);

  if (d == s) return dest;

  // decide direction
  if (d > s && d < s + len) {
    const UInt8* rs = s + len;
    UInt8*       rd = d + len;

    // try 64-bit aligned backward copy
    if (len >= sizeof(UInt64) && (reinterpret_cast<UIntPtr>(rs) % sizeof(UInt64) == 0) &&
        (reinterpret_cast<UIntPtr>(rd) % sizeof(UInt64) == 0)) {
      auto rsw = reinterpret_cast<const UInt64*>(rs);
      auto rdw = reinterpret_cast<UInt64*>(rd);

      SizeT words = len / sizeof(UInt64);

      for (SizeT i = 0; i < words; ++i) {
        rdw[-1 - static_cast<SizeT>(i)] = rsw[-1 - static_cast<SizeT>(i)];
      }

      SizeT rem = len % sizeof(UInt64);
      for (SizeT i = 0; i < rem; ++i) {
        rd[-1 - i] = rs[-1 - i];
      }
    } else {
      // byte-wise backward
      for (SizeT i = 0; i < len; ++i) {
        rd[-1 - i] = rs[-1 - i];
      }
    }
  } else {
    // try 64-bit aligned forward copy
    if (len >= sizeof(UInt64) && (reinterpret_cast<UIntPtr>(s) % sizeof(UInt64) == 0) &&
        (reinterpret_cast<UIntPtr>(d) % sizeof(UInt64) == 0)) {
      auto  sw    = reinterpret_cast<const UInt64*>(s);
      auto  dw    = reinterpret_cast<UInt64*>(d);
      SizeT words = len / sizeof(UInt64);

      for (SizeT i = 0; i < words; ++i) {
        dw[i] = sw[i];
      }

      SizeT       rem    = len % sizeof(UInt64);
      const SizeT offset = words * sizeof(UInt64);
      for (SizeT i = 0; i < rem; ++i) {
        d[offset + i] = s[offset + i];
      }
    } else {
      for (SizeT i = 0; i < len; ++i) {
        d[i] = s[i];
      }
    }
  }

  return dest;
}

IMPORT_C SInt64 MmStrLen(const Char* in) {
  // strlen via pointer walk
  if (!in) return -kErrorInvalidData;

  const Char* p = in;
  while (*p) ++p;

  return static_cast<SInt64>(p - in);
}

IMPORT_C VoidPtr MmFillMemory(_Input VoidPtr dest, _Input UInt64 len, _Input UInt8 value) {
  if (!len || !dest) return nullptr;

  auto d = static_cast<UInt8*>(dest);

  if (len >= sizeof(UInt64) && (reinterpret_cast<UIntPtr>(d) % sizeof(UInt64)) == 0) {
    UInt64 pattern = static_cast<UInt64>(value);
    pattern |= (pattern << 8);
    pattern |= (pattern << 16);
    pattern |= (pattern << 32);

    auto  dw    = reinterpret_cast<UInt64*>(d);
    SizeT words = len / sizeof(UInt64);

    for (SizeT i = 0; i < words; ++i) {
      dw[i] = pattern;
    }

    SizeT       rem    = len % sizeof(UInt64);
    const SizeT offset = words * sizeof(UInt64);
    for (SizeT i = 0; i < rem; ++i) {
      d[offset + i] = value;
    }
  } else {
    for (SizeT i = 0; i < len; ++i) d[i] = value;
  }

  return dest;
}

IMPORT_C Ref IoOpenFile(_Input const Char* path, _Input const Char* drv_letter) {
  return static_cast<Ref>(nesys_syscall_arg_3(SYSCALL_HASH("IoOpenFile"),
                                               Verify::sys_safe_cast<Char, Void>(path),
                                               Verify::sys_safe_cast<Char, Void>(drv_letter)));
}

IMPORT_C Void IoCloseFile(_Input Ref desc) {
  nesys_syscall_arg_2(SYSCALL_HASH("IoCloseFile"), static_cast<VoidPtr>(desc));
}

IMPORT_C UInt64 IoSeekFile(_Input Ref desc, _Input UInt64 off) {
  auto ret_ptr = nesys_syscall_arg_3(SYSCALL_HASH("IoSeekFile"), static_cast<VoidPtr>(desc),
                                      reinterpret_cast<VoidPtr>(&off));

  if (!ret_ptr) return kNeNPos;

  auto ret = static_cast<volatile UInt64*>(ret_ptr);

  UInt64 result = *ret;
  MUST_PASS(result != kNeNPos);

  return result;
}

IMPORT_C UInt64 IoTellFile(_Input Ref desc) {
  auto ret_ptr = nesys_syscall_arg_2(SYSCALL_HASH("IoTellFile"), static_cast<VoidPtr>(desc));

  if (!ret_ptr) return kNeNPos;

  auto ret = static_cast<volatile UInt64*>(ret_ptr);
  return *ret;
}

IMPORT_C SInt32 PrintRelease(_Input IORef buf) {
  SInt32* ret = static_cast<SInt32*>(
      nesys_syscall_arg_2(SYSCALL_HASH("PrintRelease"), static_cast<VoidPtr>(buf)));

  if (!ret) return -kErrorInvalidData;

  return static_cast<SInt32>(*ret);
}

IMPORT_C IORef PrintCreate(Void) {
  return static_cast<IORef>(nesys_syscall_arg_1(SYSCALL_HASH("PrintCreate")));
}

IMPORT_C VoidPtr MmCreateHeap(UInt64 initial_size, UInt32 max_size) {
  return static_cast<VoidPtr>(nesys_syscall_arg_3(SYSCALL_HASH("MmCreateHeap"),
                                                   reinterpret_cast<VoidPtr>(&initial_size),
                                                   reinterpret_cast<VoidPtr>(&max_size)));
}

IMPORT_C SInt32 MmDestroyHeap(VoidPtr heap) {
  auto ret = nesys_syscall_arg_2(SYSCALL_HASH("MmDestroyHeap"), static_cast<VoidPtr>(heap));
  return *static_cast<SInt32*>(ret);
}

IMPORT_C SInt32 PrintIn(_Input IORef desc, const Char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  auto buf = StrFmt(fmt, args);

  va_end(args);

  // if truncated, `needed` >= kBufferSz; we still send truncated buffer
  auto ret_ptr = nesys_syscall_arg_3(SYSCALL_HASH("PrintIn"), static_cast<VoidPtr>(desc),
                                      Verify::sys_safe_cast<Char, Void>(buf));

  if (!ret_ptr) return -kErrorInvalidData;

  auto ret = static_cast<const volatile SInt32*>(ret_ptr);

  return *ret;
}

IMPORT_C IORef PrintGet(const Char* path) {
  return static_cast<IORef>(
      nesys_syscall_arg_2(SYSCALL_HASH("PrintGet"), Verify::sys_safe_cast<Char, Void>(path)));
}

IMPORT_C ErrRef ErrGetLastError(Void) {
  return *static_cast<ErrRef*>(nesys_syscall_arg_1(SYSCALL_HASH("ErrGetLastError")));
}

IMPORT_C SInt32 PrintOut(_Input IORef desc, const Char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  auto buf = StrFmt(fmt, args);

  va_end(args);

  // if truncated, `needed` >= kBufferSz; we still send truncated buffer
  auto ret_ptr = nesys_syscall_arg_3(SYSCALL_HASH("PrintOut"), static_cast<VoidPtr>(desc),
                                      Verify::sys_safe_cast<Char, Void>(buf));

  if (!ret_ptr) return -kErrorInvalidData;

  auto ret = static_cast<const volatile SInt32*>(ret_ptr);

  return *ret;
}

IMPORT_C UInt64 PrintSize(IORef ref) {
  return *static_cast<UInt64*>(nesys_syscall_arg_2(SYSCALL_HASH("PrintSize"), ref));
}

IMPORT_C SInt32 ThrJoinThread(ThreadRef ref) {
  if (auto ret = nesys_syscall_arg_2(nesys_hash_64("ThrJoinThread"), (VoidPtr) &ref); ret)
    return *(SInt32*) ret;

  return -kErrorInvalidData;
}

IMPORT_C ThreadRef ThrCurrentThread(Void) {
  if (auto ret = nesys_syscall_arg_1(nesys_hash_64("ThrCurrentThread")); ret)
    return (ThreadRef) ret;

  return nullptr;
}

IMPORT_C SInt32 ThrYieldThread(ThreadRef ref) {
  if (auto ret = nesys_syscall_arg_2(nesys_hash_64("ThrYieldThread"), (VoidPtr) &ref); ret)
    return *(SInt32*) ret;

  return -kErrorInvalidData;
}

IMPORT_C SInt32 ThrDetachThread(ThreadRef ref) {
  if (auto ret = nesys_syscall_arg_2(nesys_hash_64("ThrDetachThread"), (VoidPtr) &ref); ret)
    return *(SInt32*) ret;

  return -kErrorInvalidData;
}

IMPORT_C SInt32 ThrExitCurrentThread(SInt32 ex) {
  if (auto ret = nesys_syscall_arg_2(SYSCALL_HASH("ThrExitCurrentThread"), (VoidPtr) &ex); ret)
    return *(SInt32*) ret;

  return -kErrorInvalidData;
}

IMPORT_C SInt32 ThrExitMainThread(SInt32 exit_code) {
  nesys_syscall_arg_2(SYSCALL_HASH("ThrExitMainThread"),
                      reinterpret_cast<VoidPtr>(static_cast<UIntPtr>(exit_code)));

  while (YES) {
  }

  return exit_code;
}

IMPORT_C SemaphoreRef SemCreate(UInt32 initial_count, UInt32 max_count, const Char* name) {
  if (auto ret = nesys_syscall_arg_4(SYSCALL_HASH("SemCreate"), &initial_count, &max_count,
                                    (VoidPtr) name);
      ret)
    return (SemaphoreRef) ret;

  return nullptr;
}

IMPORT_C SInt32 SemClose(SemaphoreRef sem) {
  if (auto ret = nesys_syscall_arg_2(SYSCALL_HASH("SemClose"), sem); ret) return *(SInt32*) ret;

  return -kErrorInvalidData;
}

// END