// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifndef KERNELKIT_HARDWARETHREADSCHEDULER_H
#define KERNELKIT_HARDWARETHREADSCHEDULER_H

#include <ArchKit/ArchKit.h>
#include <CompilerKit/CompilerKit.h>
#include <NeKit/Ref.h>

/// @note Last Rev Sun 28 Jul CET 2024
/// @note Last Rev Thu, Aug  1, 2024  9:07:38 AM
/// @note Last Rev Sun, March  22, 2026  5:16 PM

#if defined(__nekernel_max_cores)
/// \note This can be edited at compile-time to specify how many cores can be used by NeKernel.
#define kMaxAPInsideSched (__nekernel_max_cores)
#endif

#if defined(__nekernel_boot_core_index)
#define kBootAPIndex (__nekernel_boot_core_index)
#endif

namespace Ne::Kernel {

enum struct ThreadKind {
  kAPInvalid        = 0,
  kAPSystemReserved = 100,  // System reserved thread, well user can't use it
  kAPStandard,              // user thread, cannot be used by Ne::Kernel
  kAPRealTime,              // fallback thread, cannot be used by user if not clear or
                            // used by Ne::Kernel.
  kAPBoot,                  // The core we booted from, the mama.
  kAPCount = kAPBoot - kAPSystemReserved + 1,
};

/// \brief Alias for thread ID.
using ThreadID = UInt32;

/***********************************************************************************/
///
/// \name HardwareThread
/// \brief Abstraction over the CPU's core, used to run processes or threads.
///
/***********************************************************************************/

class HardwareThread final {
 public:
  explicit HardwareThread();
  ~HardwareThread();

 public:
  NE_COPY_DEFAULT(HardwareThread)

 public:
  explicit operator bool();

 public:
  Void Wake(const BOOL& wakeup = false);
  Void Busy(const BOOL& busy = false);

 public:
  BOOL Switch(HAL::StackFramePtr frame);
  BOOL IsWakeup();

 public:
  HAL::StackFramePtr        StackFrame();
  _Output const ThreadKind& Kind();
  BOOL                      IsBusy();
  _Output const ThreadID&   ID();

 private:
  HAL::StackFramePtr fStack{};
  ThreadKind         fKind{ThreadKind::kAPStandard};
  ThreadID           fID{};
  Bool               fWakeup{NO};
  Bool               fBusy{NO};
  UInt64             fPTime{};

 private:
  friend class HardwareThreadScheduler;
  friend class UserProcessHelper;
};

///
/// \name HardwareThreadScheduler
/// \brief Class to manage the thread scheduling.
///

class HardwareThreadScheduler final : public ISchedulable {
 private:
  friend class UserProcessHelper;

 public:
  explicit HardwareThreadScheduler();
  ~HardwareThreadScheduler();
  NE_COPY_DEFAULT(HardwareThreadScheduler)

 public:
  HAL::StackFramePtr Leak();

 public:
  Ref<HardwareThread*> operator[](SizeT idx);
  bool                 operator!();
  operator bool();

  Bool IsUser() override { return Yes; }

  Bool IsKernel() override { return No; }

  Bool HasMP() override { return kHandoverHeader->f_HardwareTables.f_MultiProcessingEnabled; }

 public:
  /// @brief Shared instance of the MP Mgr.
  /// @return the reference to the mp manager class.
  STATIC HardwareThreadScheduler& The();

 public:
  /// @brief Returns the amount of threads present in the system.
  /// @returns SizeT the amount of cores present.
  SizeT Capacity();

 private:
  Array<HardwareThread, kMaxAPInsideSched> fThreadList;
  ThreadID                                 fCurrentThreadIdx{0};
};

/// @brief wakes up thread.
/// wakes up thread from hang.
Void mp_wakeup_thread(HAL::StackFramePtr stack);

/// @brief makes thread sleep.
/// hooks and hangs thread to prevent code from executing.
Void mp_hang_thread(HAL::StackFramePtr stack);

}  // namespace Ne::Kernel

#endif
