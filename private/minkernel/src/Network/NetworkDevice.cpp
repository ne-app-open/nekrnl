// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <NeKit/Utils.h>
#include <NetworkKit/NetworkDevice.h>

namespace Ne::Kernel {

/// \brief Getter for fNetworkName.
/// \return Network device name.
const Char* NetworkDevice::Name() const {
  return kDeviceMgrRootDirPath "net/nic{}";
}

/// \brief Setter for fNetworkName.
Boolean NetworkDevice::Name(const Char* name) {
  if (name == nullptr) return NO;

  if (*name == 0) return NO;

  if (rt_string_len(name) > rt_string_len(this->Name())) return NO;

  rt_copy_memory((VoidPtr) name, (VoidPtr) this->Name(), rt_string_len(this->Name()));

  return YES;
}

}  // namespace Ne::Kernel
