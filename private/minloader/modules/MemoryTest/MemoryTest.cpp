// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <BootKit/BootKit.h>
#include <BootKit/BootThread.h>
#include <FirmwareKit/EFI/API.h>

#ifndef kMemoryTestVal
#define kMemoryTestVal (42)
#endif

EXTERN_C Int32 MemoryTestModuleMain(Ne::Kernel::HEL::BootInfoHeader* handover) {
  ::fw_init_efi(static_cast<EfiSystemTable*>(handover->f_FirmwareCustomTables[Ne::Kernel::HEL::kHandoverTableST]));

  Boot::BootTextWriter writer;
  writer.Write("MemoryTest: Testing Memory...\r");

  UIntPtr kTestValue = 0;

  volatile UInt64* mem = (volatile UInt64*) kTestValue;

  auto prev = *mem;
  *mem      = kMemoryTestVal;
  
  if (*mem != kMemoryTestVal) {
    return kEfiFail;
  }

  *mem = prev;

  return kEfiOk;
}
