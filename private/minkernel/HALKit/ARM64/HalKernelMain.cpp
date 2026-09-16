// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <ArchKit/ArchKit.h>
#include <CFKit/Property.h>
#include <FirmwareKit/Handover.h>
#include <HALKit/ARM64/ApplicationProcessor.h>
#include <HALKit/ARM64/Processor.h>
#include <KernelKit/CodeMgr.h>
#include <KernelKit/FileMgr.h>
#include <KernelKit/HardwareThreadScheduler.h>
#include <KernelKit/HeapMgr.h>
#include <KernelKit/PEFCodeMgr.h>
#include <KernelKit/ProcessScheduler.h>
#include <NeKit/Json.h>
#include <NetworkKit/IPC.h>
#include <modules/ACPI/ACPIFactoryInterface.h>
#include <modules/CoreGfx/CoreGfx.h>

#ifndef __NE_MODULAR_KERNEL_COMPONENTS__
STATIC Ne::Kernel::Void kei_init_drivers(Ne::Kernel::Void) {
  PE32Loader ldr("/system/drvhost.exe");

  if (ldr.IsLoaded() && rtl_create_user_process(
                            ldr, UserProcess::ExecutableKind::kExecutableKind) != kCPSInvalidPID) {
    (Void)(kout << "hal_real_init: Spawned the NeAnt Driver Host.\r");
  } else {
    (Void)(kout << "hal_real_init: warning: Driver host did not spawn.\r");
    ke_stop(RUNTIME_CHECK_BOOTSTRAP, "Bug-Check failed at Kernel Driver Init in HAL.");
  }

  /// Implement additional driver runtime code code here ///

  /// Implement additional driver runtime code code here ///
}
EXTERN_C void hal_init_platform(Ne::Kernel::HEL::BootInfoHeader* handover_hdr) {
  using namespace Ne::Kernel;

  /************************************************** */
  /*     INITIALIZE AND VALIDATE HEADER.              */
  /************************************************** */

  if (handover_hdr->f_Magic != kHandoverMagic || handover_hdr->f_Version != kHandoverVersion) {
    return;
  }

  kHandoverHeader = handover_hdr;

#ifdef __NE_ARM64_EFI__
  ::fw_init_efi(static_cast<EfiSystemTable*>(
      handover_hdr->f_FirmwareCustomTables[Ne::Kernel::HEL::kHandoverTableST]));

  Boot::ExitBootServices(handover_hdr->f_HardwareTables.f_ImageKey,
                         handover_hdr->f_HardwareTables.f_ImageHandle);
#endif

  kei_init_drivers();

  FB::cg_clear_video();

  /************************************** */
  /*     INITIALIZE BIT MAP.              */
  /************************************** */

  kBitMapCursor     = 0UL;
  kKernelBitMpSize  = kHandoverHeader->f_BitMapSize;
  kKernelBitMpStart = reinterpret_cast<Ne::Kernel::VoidPtr>(
      reinterpret_cast<Ne::Kernel::UIntPtr>(kHandoverHeader->f_BitMapStart));

  /// @note do initialize the interrupts after it.

  Ne::Kernel::mp_init_cores();

  Ne::Kernel::user_init_std(kHandoverHeader->f_RecoverMode);

  while (YES);
}
#endif
