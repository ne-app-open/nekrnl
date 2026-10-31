// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <HALKit/AMD64/Processor.h>
#include <modules/ACPI/ACPIFactoryInterface.h>

namespace Ne::Kernel::HAL {
/***********************************************************************************/
/// Constructors.
/***********************************************************************************/
LAPICDmaWrapper::LAPICDmaWrapper(VoidPtr base) : fApic(base) {}
LAPICDmaWrapper::~LAPICDmaWrapper() = default;

/***********************************************************************************/
/// @brief Read from APIC controller.
/// @param reg register.
/***********************************************************************************/
UInt32 LAPICDmaWrapper::Read(UInt16 reg) {
  MUST_PASS(this->fApic);

  UInt32 volatile* io_apic = (UInt32 volatile*) this->fApic;
  return io_apic[reg];
}

/***********************************************************************************/
/// @brief Write to APIC controller.
/// @param reg register.
/// @param value value.
/***********************************************************************************/
Void LAPICDmaWrapper::Write(UInt16 reg, UInt32 value) {
  MUST_PASS(this->fApic);

  UInt32 volatile* io_apic = (UInt32 volatile*) this->fApic;
  io_apic[reg]             = value;
}
}  // namespace Ne::Kernel::HAL

// EOF
