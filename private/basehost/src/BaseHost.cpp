// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-foss/ne-kernel

#include <basehost/HostKit/Foundation.h>
#include <SystemKit/Err.h>
#include <SystemKit/Syscall.h>

/// @note This called from _NeMain for its own runtime.
IMPORT_C SInt32 launch_startup_fn(Void) {
  /// start the LaunchHelpers.fwrk service, and make the launcher scheduable too (via mgmt.launch)
  UInt32* ret = static_cast<UInt32*>(::nesys_syscall_arg_1(
      ::nesys_hash_64("__ne_register_base_host")));  // Register service based on program data.

  if (ret) {
    switch (*ret) {
      case kErrorSuccess: {
        ret = static_cast<UInt32*>(::nesys_syscall_arg_1(
          ::nesys_hash_64(
            "__ne_attach_base_host")));  // Attach this program as the service process.
        return *ret;
      }
      default:
        break;
    }
  }

  /// The Shutdown Service (neshtdown.exe) is called when the OS goes into a unrecoverable state.
  ::nesys_syscall_arg_1(::nesys_hash_64("__ne_call_shutdown_service"));
  
  return kErrorExecutable;
}
