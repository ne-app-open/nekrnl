// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-open/nekrnl

#include <KernelKit/UserMgr.h>
#include <KernelKit/UserProcessScheduler.h>
#include <NeKit/NeKit.h>

#ifndef IMPORT_C
#define IMPORT_C extern "C"
#endif

using namespace ::Ne::Kernel;

///////////////////////////////////////////////////////////////////////////////////////////

STATIC ATTRIBUTE(unused)
UInt64 kDrvHostPid = 0;

STATIC ATTRIBUTE(unused)
UInt64 kBaseHostPid = 0;

STATIC ATTRIBUTE(unused)
UInt64 kRPCHostPid = 0;

STATIC ATTRIBUTE(unused)
User* kDrvHostUser = nullptr;

STATIC ATTRIBUTE(unused)
User* kBaseHostUser = nullptr;

STATIC ATTRIBUTE(unused)
User* kRPCHostUser = nullptr;

ATTRIBUTE(unused) BOOL kShutdownScheduled = NO;

///////////////////////////////////////////////////////////////////////////////////////////

IMPORT_C Void __ne_register_drv_host(Void) {
  if (kDrvHostPid == 0) kDrvHostPid = UserProcessScheduler::The().TheCurrentProcess().ProcessId;
}

IMPORT_C Void __ne_register_base_host(Void) {
  if (kBaseHostPid == 0) kBaseHostPid = UserProcessScheduler::The().TheCurrentProcess().ProcessId;
}

IMPORT_C Void __ne_register_rpc_service(Void) {
  if (kRPCHostPid == 0) kRPCHostPid = UserProcessScheduler::The().TheCurrentProcess().ProcessId;
}

IMPORT_C Void __ne_attach_drv_host(Void) {
  if (kDrvHostPid != 0) {
    kDrvHostUser = UserProcessScheduler::The().TheCurrentTeam().AsArray()[kDrvHostPid].Owner;
  }
}

IMPORT_C Void __ne_attach_base_host(Void) {
  if (kBaseHostPid != 0) {
    kBaseHostUser = UserProcessScheduler::The().TheCurrentTeam().AsArray()[kBaseHostPid].Owner;
  }
}

IMPORT_C Void __ne_attach_rpc_service(Void) {
  if (kRPCHostPid != 0) {
    kRPCHostUser = UserProcessScheduler::The().TheCurrentTeam().AsArray()[kRPCHostPid].Owner;
  }
}

IMPORT_C Void __ne_call_shutdown_service(Void) {
    if (kBaseHostUser == kCurrentUser) {
      if (!kShutdownScheduled)
        kShutdownScheduled = YES;
      
      kout << (kShutdownScheduled ? "Shutdown Called.\r" : "Nothing changed.\r");
    }
}
