// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <BootKit/BootKit.h>
#include <BootKit/BootThread.h>
#include <BootKit/HW/SATA.h>
#include <FirmwareKit/EFI.h>
#include <FirmwareKit/EFI/API.h>
#include <FirmwareKit/Handover.h>
#include <KernelKit/MSDOS.h>
#include <KernelKit/PE.h>
#include <KernelKit/PEF.h>
#include <NeKit/Macros.h>
#include <NeKit/Ref.h>
#include <modules/CoreGfx/CoreGfx.h>
#include <modules/CoreGfx/TextGfx.h>

// Makes the compiler shut up.
#ifndef kMachineModel
#define kMachineModel "OS"
#endif  // !kMachineModel

EXTERN_C Int32 SysChkModuleMain(Ne::Kernel::HEL::BootInfoHeader* handover) {
  ::fw_init_efi(static_cast<EfiSystemTable*>(handover->f_FirmwareCustomTables[Ne::Kernel::HEL::kHandoverTableST]));

#if defined(__ATA_PIO__)
  Boot::BDiskFormatFactory<Boot::BootDeviceATA> partition_factory;
#elif defined(__AHCI__)
  Boot::BDiskFormatFactory<Boot::BootDeviceSATA> partition_factory;
#elif defined(__NVME__)
  Boot::BDiskFormatFactory<Boot::BootDeviceNVME> partition_factory;
#endif

  if (partition_factory.IsPartitionValid()) return kEfiOk;

  return partition_factory.Format(kMachineModel) ? kEfiOk : kEfiFail;
}
