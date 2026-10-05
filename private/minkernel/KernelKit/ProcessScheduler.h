// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifndef KERNELKIT_PROCESS_SCHEDULER_H
#define KERNELKIT_PROCESS_SCHEDULER_H

#include <KernelKit/KernelTask.h>
#include <KernelKit/UserProcessScheduler.h>

#ifdef __NEOSKRNL__

namespace Ne::Kernel {

/// @brief Local processes.
inline UserProcessTeam kLowUserTeam;
inline UserProcessTeam kHighUserTeam;
inline UserProcessTeam kMidUserTeam;
inline UserProcessTeam kRTUserTeam;

inline UserProcessTeam kDLLTeam;
inline UserProcessTeam kHostsTeam;

/// @brief Remote processes.
inline UserProcessTeam kServicesTeam;
inline UserProcessTeam kMsgBlkTeam;
inline UserProcessTeam kNetworkTeam;

}  // namespace Ne::Kernel

#endif

#endif
