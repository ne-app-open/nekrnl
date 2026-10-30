// SPDX-License-Identifier: Apache-2.0
// Copyright 2026, Ne.app.
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-open/nekrnl

#include <NeKit/NeKit.h>
#include <KernelKit/UserProcessScheduler.h>

#ifndef IMPORT_C
#define IMPORT_C extern "C"
#endif

using namespace ::Ne::Kernel;

IMPORT_C Void _HalInitDriverRuntime(Void) {
    MUST_PASS(UserProcessScheduler::The().TheCurrentProcess().Image.HasCode());
    MUST_PASS(UserProcessScheduler::The().TheCurrentProcess().SubSystem == ProcessSubsystem::kProcessSubsystemDriver);

    UserProcessScheduler::The().TheCurrentProcess().Wake(YES);
}

IMPORT_C Void _HalFiniDriverRuntime(Void) {
    MUST_PASS(UserProcessScheduler::The().TheCurrentProcess().Image.HasCode());
    MUST_PASS(UserProcessScheduler::The().TheCurrentProcess().SubSystem == ProcessSubsystem::kProcessSubsystemDriver);

    UserProcessScheduler::The().TheCurrentProcess().Exit(kErrorSuccess);
    
    while (YES)
        MUST_PASS(NO);
}

// EOF
