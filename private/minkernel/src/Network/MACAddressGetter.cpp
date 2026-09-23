// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <NetworkKit/MAC.h>

namespace Ne::Kernel {

  Array<UInt8, kMACAddrLen>& MacAddressGetter::AsBytes() {
  return this->fMacAddress;
}

}  // namespace Ne::Kernel
