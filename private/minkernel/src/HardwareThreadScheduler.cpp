// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <ArchKit/ArchKit.h>
#include <CFKit/Property.h>
#include <KernelKit/HardwareThreadScheduler.h>
#include <KernelKit/ProcessScheduler.h>

/***********************************************************************************/
///! @file HardwareThreadScheduler.cc
///! @brief This file handles multi processing in the Ne::Kernel.
///! @brief Multi processing is needed for multi-tasking operations.
/***********************************************************************************/

namespace Ne::Kernel {
/***********************************************************************************/
/// @note Those symbols are needed in order to switch and validate the stack.
/***********************************************************************************/

EXTERN_C Bool hal_check_task(HAL::StackFramePtr frame);
EXTERN_C Bool mp_register_task(HAL::StackFramePtr frame, ProcessID pid);

///! A HardwareThread class takes care of it's owned hardware thread.
///! It has a stack for it's core.

/***********************************************************************************/
///! @brief C++ constructor.
/***********************************************************************************/
HardwareThread::HardwareThread() = default;

/***********************************************************************************/
///! @brief C++ destructor.
/***********************************************************************************/
HardwareThread::~HardwareThread() = default;

/***********************************************************************************/
//! @brief returns the id of the thread.
/***********************************************************************************/
_Output const ThreadID& HardwareThread::ID() {
  return this->fID;
}

/***********************************************************************************/
//! @brief returns the kind of thread we have.
/***********************************************************************************/
_Output const ThreadKind& HardwareThread::Kind() {
  return this->fKind;
}

/***********************************************************************************/
//! @brief is the thread busy?
//! @return whether the thread is busy or not.
/***********************************************************************************/
Bool HardwareThread::IsBusy() {
  return this->fBusy;
}

/***********************************************************************************/
/// @brief Get processor stack frame.
/***********************************************************************************/

HAL::StackFramePtr HardwareThread::StackFrame() {
  MUST_PASS(this->fStack);
  return this->fStack;
}

Void HardwareThread::Busy(Bool busy) {
  this->fBusy = busy;
}

HardwareThread::operator bool() {
  return this->fStack && !this->fBusy;
}

/***********************************************************************************/
/// @brief Wakeup the processor.
/***********************************************************************************/

Void HardwareThread::Wake(const bool wakeup) {
  this->fWakeup = wakeup;
}

/***********************************************************************************/
/// @brief Switch to hardware thread.
/// @param stack the new hardware thread.
/// @retval true stack was changed, code is running.
/// @retval false stack is invalid, previous code is running.
/***********************************************************************************/
Bool HardwareThread::Switch(HAL::StackFramePtr frame) {
  if (!frame) {
    return NO;
  }

  if (!hal_check_task(frame)) {
    return NO;
  }

  if (frame->IP == 0 || frame->SP == 0) {
    return NO;
  }

  /// Use atomics before switching threads.
  static std::atomic_flag flg = ATOMIC_FLAG_INIT;

  while (!flg.test_and_set(std::memory_order_acquire));

  this->fStack = frame;
  auto ret = mp_register_task(this->fStack, this->fID);

  flg.clear(std::memory_order_release);

  return ret;
}

/***********************************************************************************/
///! @brief Tells if processor is waked up.
/***********************************************************************************/
bool HardwareThread::IsWakeup() {
  return this->fWakeup;
}

/***********************************************************************************/
///! @brief Constructor and destructors.
///! @brief Default constructor.
/***********************************************************************************/

HardwareThreadScheduler::HardwareThreadScheduler() = default;

/***********************************************************************************/
///! @brief Default destructor.
/***********************************************************************************/
HardwareThreadScheduler::~HardwareThreadScheduler() = default;

/***********************************************************************************/
/// @brief Shared singleton function
/***********************************************************************************/
HardwareThreadScheduler& HardwareThreadScheduler::The() {
  STATIC HardwareThreadScheduler kHardwareThreadScheduler;
  return kHardwareThreadScheduler;
}

/***********************************************************************************/
/// @brief Get Stack Frame of AP.
/***********************************************************************************/
HAL::StackFramePtr HardwareThreadScheduler::Leak() {
  return fThreadList[fCurrentThreadIdx].fStack;
}

/***********************************************************************************/
/**
 * Get Hardware thread at index.
 * @param idx the index
 * @return the reference to the hardware thread.
 */
/***********************************************************************************/
Ref<HardwareThread*> HardwareThreadScheduler::operator[](SizeT idx) {
  if (idx > kMaxAPInsideSched) {
    STATIC HardwareThread* kFakeThread = nullptr;
    return {kFakeThread};
  }

  this->fCurrentThreadIdx = idx;
  return &this->fThreadList[idx];
}

/***********************************************************************************/
/**
 * Check if thread pool isn't empty.
 * @return
 */
/***********************************************************************************/
HardwareThreadScheduler::operator bool() {
  return !this->fThreadList.Empty();
}

/***********************************************************************************/
/**
 * Reverse operator bool
 * @return
 */
/***********************************************************************************/
bool HardwareThreadScheduler::operator!() {
  return this->fThreadList.Empty();
}

/***********************************************************************************/
/// @brief Returns the amount of core present.
/// @return the number of APs.
/***********************************************************************************/
SizeT HardwareThreadScheduler::Capacity() {
  return this->fThreadList.Count();
}
}  // namespace Ne::Kernel
