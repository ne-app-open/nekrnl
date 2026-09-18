// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <ArchKit/ArchKit.h>
#include <KernelKit/ProcessScheduler.h>
#include <KernelKit/User.h>
#include <NeKit/KString.h>
#include <SignalKit/Signals.h>

EXTERN_C Ne::Kernel::Void int_handle_breakpoint(Ne::Kernel::UIntPtr rip);
EXTERN_C BOOL         mp_handle_gic_interrupt_el0(Void);

EXTERN_C BOOL  kEndOfInterrupt;
EXTERN_C UInt8 kEndOfInterruptVector;

STATIC BOOL kIsRunning = NO;

/// @note This is managed by the system software.
STATIC void hal_int_send_eoi(UInt8 vector) {
  kEndOfInterrupt       = YES;
  kEndOfInterruptVector = vector;
}

/// @brief Handle GPF fault.
/// @param rsp
EXTERN_C Ne::Kernel::Void int_handle_gpf(Ne::Kernel::UIntPtr rsp) {
  auto& process = Ne::Kernel::UserProcessScheduler::The().TheCurrentProcess();
  process.Crash();

  hal_int_send_eoi(13);

  process.Signal.SignalArg = rsp;
  process.Signal.SignalID  = SIGKILL;
  process.Signal.Status    = process.Status;
}

/// @brief Handle page fault.
/// @param rsp
EXTERN_C void int_handle_pf(Ne::Kernel::UIntPtr rsp) {
  auto& process = Ne::Kernel::UserProcessScheduler::The().TheCurrentProcess();
  process.Crash();

  hal_int_send_eoi(14);

  process.Signal.SignalArg = rsp;
  process.Signal.SignalID  = SIGKILL;
  process.Signal.Status    = process.Status;
}

/// @brief Handle scheduler interrupt.
EXTERN_C void int_handle_scheduler(Ne::Kernel::UIntPtr rsp) {
  NE_UNUSED(rsp);

  hal_int_send_eoi(32);

  while (kIsRunning);

  kIsRunning = YES;

  mp_handle_gic_interrupt_el0();

  kIsRunning = NO;
}

/// @brief Handle math fault.
/// @param rsp
EXTERN_C void int_handle_math(Ne::Kernel::UIntPtr rsp) {
  auto& process = Ne::Kernel::UserProcessScheduler::The().TheCurrentProcess();
  process.Crash();

  hal_int_send_eoi(8);

  process.Signal.SignalArg = rsp;
  process.Signal.SignalID  = SIGKILL;
  process.Signal.Status    = process.Status;
}

/// @brief Handle any generic fault.
/// @param rsp
EXTERN_C void int_handle_generic(Ne::Kernel::UIntPtr rsp) {
  auto& process = Ne::Kernel::UserProcessScheduler::The().TheCurrentProcess();
  process.Crash();

  hal_int_send_eoi(30);

  Ne::Kernel::kout << "Ne::Kernel: Generic Process Fault.\r";

  process.Signal.SignalArg = rsp;
  process.Signal.SignalID  = SIGKILL;
  process.Signal.Status    = process.Status;

  Ne::Kernel::kout << "Ne::Kernel: SIGKILL status.\r";
}

EXTERN_C Ne::Kernel::Void int_handle_breakpoint(Ne::Kernel::UIntPtr rip) {
  auto& process = Ne::Kernel::UserProcessScheduler::The().TheCurrentProcess();

  hal_int_send_eoi(3);

  process.Signal.SignalArg = rip;
  process.Signal.SignalID  = SIGTRAP;

  process.Signal.Status = process.Status;

  process.Status = Ne::Kernel::ProcessStatusKind::kFrozen;
}

/// @brief Handle #UD fault.
/// @param rsp
EXTERN_C void int_handle_ud(Ne::Kernel::UIntPtr rsp) {
  auto& process = Ne::Kernel::UserProcessScheduler::The().TheCurrentProcess();
  process.Crash();

  hal_int_send_eoi(6);

  process.Signal.SignalArg = rsp;
  process.Signal.SignalID  = SIGKILL;
  process.Signal.Status    = process.Status;
}

/// @brief Enter syscall from assembly (libSystem only)
/// @param stack the stack pushed from assembly routine.
/// @return nothing.
EXTERN_C Ne::Kernel::Void hal_system_call_enter(Ne::Kernel::UIntPtr rcx_hash,
                                                   Ne::Kernel::UIntPtr rdx_syscall_arg) {
  if (!Ne::Kernel::kCurrentUser) return;
  if (!rdx_syscall_arg || !rcx_hash) return;

  for (SizeT i{}; i < kMaxDispatchCallCount; ++i) {
    if (kSysCalls[i].fHooked && rcx_hash == kSysCalls[i].fHash) {
      if (!kSysCalls[i].fProc) break;

      (kSysCalls[i].fProc)((Ne::Kernel::VoidPtr) rdx_syscall_arg);
    }
  }

  return;
}

/// @brief Enter Ne::Kernel call from assembly (libDDK only).
/// @param stack the stack pushed from assembly routine.
/// @return nothing.
EXTERN_C Ne::Kernel::Void hal_kernel_call_enter(Ne::Kernel::UIntPtr rcx_hash, Ne::Kernel::SizeT cnt,
                                            Ne::Kernel::UIntPtr arg, Ne::Kernel::SizeT sz) {
  if (!Ne::Kernel::kRootUser) return;
  if (Ne::Kernel::kCurrentUser != Ne::Kernel::kRootUser) return;
  if (!Ne::Kernel::kCurrentUser->IsSuperUser()) return;
  if (!arg || !rcx_hash) return;

  for (SizeT i{}; i < kMaxDispatchCallCount; ++i) {
    if (kKernCalls[i].fHooked && rcx_hash == kKernCalls[i].fHash) {
      if (kKernCalls[i].fProc) {
        (kKernCalls[i].fProc)(cnt, (Ne::Kernel::VoidPtr) arg, sz);
      }
    }
  }
}
