// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <ArchKit/ArchKit.h>
#include <KernelKit/ProcessScheduler.h>
#include <KernelKit/User.h>
#include <NeKit/Atom.h>
#include <NeKit/KString.h>
#include <SignalKit/Signals.h>

EXTERN_C Ne::Kernel::Void idt_handle_breakpoint(Ne::Kernel::UIntPtr rip);
EXTERN_C Ne::Kernel::UIntPtr kApicBaseAddress;

STATIC BOOL kIsRunning{NO};

/// @brief Notify APIC and PIC that we're done with the interrupt.
/// @note
static void hal_idt_send_eoi(UInt8 vector) {
  ((volatile UInt32*) kApicBaseAddress)[0xB0 / 4] = 0;

  if (vector >= kPICCommand && vector <= 0x2F) {
    if (vector >= 0x28) {
      Ne::Kernel::HAL::rt_out8(kPIC2Command, kPICCommand);
    }
    Ne::Kernel::HAL::rt_out8(kPICCommand, kPICCommand);
  }
}

/// @brief Handle GPF fault.
/// @param rsp
EXTERN_C Ne::Kernel::Void idt_handle_gpf(Ne::Kernel::UIntPtr rsp) {
  auto& process = Ne::Kernel::UserProcessScheduler::The().TheCurrentProcess();

  if (process) process.Crash();

  hal_idt_send_eoi(13);

  if (process) {
    process.Signal.SignalArg = rsp;
    process.Signal.SignalID  = SIGKILL;
    process.Signal.Status    = process.Status;
  }
}

/// @brief Handle page fault.
/// @param rsp
EXTERN_C void idt_handle_pf(Ne::Kernel::UIntPtr rsp) {
  auto& process = Ne::Kernel::UserProcessScheduler::The().TheCurrentProcess();

  if (process) process.Crash();

  hal_idt_send_eoi(14);

  if (process) {
    process.Signal.SignalArg = rsp;
    process.Signal.SignalID  = SIGKILL;
    process.Signal.Status    = process.Status;
  }
}

/// @brief Handle scheduler interrupt.
EXTERN_C void idt_handle_scheduler(Ne::Kernel::UIntPtr rsp) {
  NE_UNUSED(rsp);

  hal_idt_send_eoi(32);

  while (kIsRunning);

  kIsRunning = YES;

  Ne::Kernel::UserProcessHelper::StartScheduling();

  kIsRunning = NO;
}

/// @brief Handle math fault.
/// @param rsp
EXTERN_C void idt_handle_math(Ne::Kernel::UIntPtr rsp) {
  auto& process = Ne::Kernel::UserProcessScheduler::The().TheCurrentProcess();

  if (process) process.Crash();

  hal_idt_send_eoi(8);

  if (process) {
    process.Signal.SignalArg = rsp;
    process.Signal.SignalID  = sig_generate_unique<SIGKILL>();
    process.Signal.Status    = process.Status;
  }
}

/// @brief Handle any generic fault.
/// @param rsp
EXTERN_C void idt_handle_generic(Ne::Kernel::UIntPtr rsp) {
  auto& process = Ne::Kernel::UserProcessScheduler::The().TheCurrentProcess();

  if (process) process.Crash();

  hal_idt_send_eoi(30);

  Ne::Kernel::kout << "Ne::Kernel: Generic Process Fault.\r";

  process.Signal.SignalArg = rsp;
  process.Signal.SignalID  = sig_generate_unique<SIGSEG>();
  ;
  process.Signal.Status = process.Status;

  Ne::Kernel::kout << "Ne::Kernel: SIGKILL status.\r";
}

EXTERN_C Ne::Kernel::Void idt_handle_breakpoint(Ne::Kernel::UIntPtr rip) {
  auto& process = Ne::Kernel::UserProcessScheduler::The().TheCurrentProcess();

  if (process) process.Crash();

  hal_idt_send_eoi(3);

  process.Signal.SignalArg = rip;
  process.Signal.SignalID  = sig_generate_unique<SIGTRAP>();

  process.Signal.Status = process.Status;

  process.Status = Ne::Kernel::ProcessStatusKind::kFrozen;
}

/// @brief Handle #UD fault.
/// @param rsp
EXTERN_C void idt_handle_ud(Ne::Kernel::UIntPtr rsp) {
  auto& process = Ne::Kernel::UserProcessScheduler::The().TheCurrentProcess();

  if (process) process.Crash();

  hal_idt_send_eoi(6);

  process.Signal.SignalArg = rsp;
  process.Signal.SignalID  = sig_generate_unique<SIGKILL>();
  process.Signal.Status    = process.Status;
}

/// @brief Enter syscall from assembly (libSystem only)
/// @param stack the stack pushed from assembly routine.
/// @return nothing.
EXTERN_C Ne::Kernel::VoidPtr hal_system_call_enter(Ne::Kernel::UIntPtr rcx_hash,
                                                   Ne::Kernel::UIntPtr rdx_syscall_arg) {
  hal_idt_send_eoi(50);

  if (!Ne::Kernel::kCurrentUser) return nullptr;
  if (!rdx_syscall_arg || !rcx_hash) return nullptr;

  for (SizeT i = 0UL; i < kMaxDispatchCallCount; ++i) {
    if (kSysCalls[i].fHooked && rcx_hash == kSysCalls[i].fHash) {
      if (!kSysCalls[i].fProc) break;

      return (kSysCalls[i].fProc)((Ne::Kernel::VoidPtr) rdx_syscall_arg);
    }
  }

  return nullptr;
}

/// @brief Enter Ne::Kernel call from assembly (libDDK only).
/// @param stack the stack pushed from assembly routine.
/// @return nothing.
EXTERN_C Ne::Kernel::Void hal_kernel_call_enter(Ne::Kernel::UIntPtr rcx_hash, Ne::Kernel::SizeT cnt,
                                            Ne::Kernel::UIntPtr arg, Ne::Kernel::SizeT sz) {
  hal_idt_send_eoi(51);

  if (!Ne::Kernel::kRootUser) return;
  if (Ne::Kernel::kCurrentUser != Ne::Kernel::kRootUser) return;
  if (!Ne::Kernel::kCurrentUser->IsSuperUser()) return;
  if (!arg || !rcx_hash) return;

  for (SizeT i = 0UL; i < kMaxDispatchCallCount; ++i) {
    if (kKernCalls[i].fHooked && rcx_hash == kKernCalls[i].fHash) {
      if (kKernCalls[i].fProc) {
        return (kKernCalls[i].fProc)(cnt, (Ne::Kernel::VoidPtr) arg, sz);
      }
    }
  }
}
