// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <KernelKit/CodeMgr.h>
#include <KernelKit/ProcessScheduler.h>
#include <NeKit/Utils.h>

namespace Ne::Kernel {

/// @brief Executes a new process from a function. Ne::Kernel code only.
/// @note This sets up a new stack, anything on the main function that calls the Ne::Kernel will not be
/// accessible.
/// @param main the start of the process.
/// @param kid the Ne::Kernel ID of the new task.
/// @return The process started or not.
BOOL rtl_create_kernel_task(KernelTask& task, const KID& kid) {
  if (!kid) return FALSE;
  if (!task.Kid) return FALSE;
  if (task.Name[0] == 0) return FALSE;
  if (task.StackSize == 0) return FALSE;

  return KernelTaskHelper::Start(task, kid);
}

/***********************************************************************************/
/// @brief Executes a new process from a function. Ne::Kernel code only.
/// @note This sets up a new stack, anything on the main function that calls the Ne::Kernel will not be
/// accessible.
/// @param main the start of the process.
/// @return if the process was started or not.
/***********************************************************************************/

ProcessID rtl_create_user_process(rtl_start_kind main, const Char* process_name) {
  if (!process_name || *process_name == 0) return kCPSInvalidPID;
  if (!main) return kCPSInvalidPID;
  
  return UserProcessScheduler::The().Spawn(process_name, reinterpret_cast<VoidPtr>(main), nullptr);
}

}  // namespace Ne::Kernel
