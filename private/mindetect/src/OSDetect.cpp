// SPDX-License-Identifier: Apache-2.0
// Copyright 2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <mindetect/DetectKit/DetectKit.h>

EXTERN_C Int32 OSDetectModuleMain(Ne::Kernel::HEL::BootInfoHeader* handover) {
  ::fw_init_efi(static_cast<EfiSystemTable*>(
      handover->f_FirmwareCustomTables[Ne::Kernel::HEL::kHandoverTableST]));

  if (handover->f_HardwareTables.f_MultiProcessingEnabled == NO) return kEfiFail;
  if (handover->f_NumberOfProcessors < 1ULL) return kEfiFail;
  if (handover->f_BitMapSize < 1LL) return kEfiFail;

  return kEfiOk;
}
