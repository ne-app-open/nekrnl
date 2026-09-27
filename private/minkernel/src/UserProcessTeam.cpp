// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-foss/kernel

#include <KernelKit/UserProcessScheduler.h>

namespace Ne::Kernel {

UserProcessTeam::UserProcessTeam() {
  for (SizeT i{}; i < kCPSProcessLimitPerTeam; ++i) {
    this->mProcessList[i]        = UserProcess{};
    this->mProcessList[i].PTime  = 0;
    this->mProcessList[i].RTime  = 0;
    this->mProcessList[i].UTime  = 0;
    this->mProcessList[i].STime  = 0;
    this->mProcessList[i].Status = ProcessStatusKind::kKilled;
  }

  // We dont know our team yet.
  this->mTeamId = 0UL;

  // We dont have any running processes.
  this->mProcessCur = 0UL;
}

/***********************************************************************************/
/// @brief Process list array getter.
/// @return The list of process to schedule.
/***********************************************************************************/

Array<UserProcess, kCPSProcessLimitPerTeam>& UserProcessTeam::AsArray() {
  return this->mProcessList;
}

/***********************************************************************************/
/// @brief Get team ID.
/// @return The team's ID.
/***********************************************************************************/

ProcessID& UserProcessTeam::Id() {
  MUST_PASS(mTeamId >= 0);
  return this->mTeamId;
}

/***********************************************************************************/
/// @brief Get current process getter as Ref.
/// @return The current process header.
/***********************************************************************************/

Ref<UserProcess>& UserProcessTeam::AsRef() {
  MUST_PASS(this->mCurrentProcess.Leak().GetParentTeam() != nullptr);
  return this->mCurrentProcess;
}

}  // namespace Ne::Kernel

// last rev 05-03-24
