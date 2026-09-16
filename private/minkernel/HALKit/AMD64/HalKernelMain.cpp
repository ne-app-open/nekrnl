// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <ArchKit/ArchKit.h>
#include <CFKit/Property.h>
#include <FirmwareKit/EFI/API.h>
#include <FirmwareKit/EFI/EFI.h>
#include <HALKit/Generic/PhysicalMemory.h>
#include <KernelKit/CodeMgr.h>
#include <KernelKit/HardwareThreadScheduler.h>
#include <KernelKit/PEFCodeMgr.h>
#include <KernelKit/ProcessScheduler.h>
#include <KernelKit/Timer.h>
#include <NetworkKit/IPC.h>
#include <StorageKit/AHCI.h>
#include <modules/ACPI/ACPIFactoryInterface.h>
#include <modules/BenchKit/HWChronometer.h>
#include <modules/CoreGfx/TextGfx.h>

#ifndef __NE_MODULAR_KERNEL_COMPONENTS__
STATIC Ne::Kernel::Void kei_init_drivers(Ne::Kernel::Void) {
  PE32Loader ldr("/system/drvhost.exe");

  if (ldr.IsLoaded() && rtl_create_user_process(
                            ldr, UserProcess::ExecutableKind::kExecutableKind) != kCPSInvalidPID) {
    (Void)(kout << "hal_real_init: Spawned the NeAnt Driver Host.\r");
  } else {
    (Void)(kout << "hal_real_init: warning: Driver host did not spawn.\r");
    ke_stop(RUNTIME_CHECK_BOOTSTRAP, "Bug-Check failed at Driver Host Init in HAL.");
  }

  PE32Loader ldr_shms("/system/shmshost.dll");

  if (ldr_shms.IsLoaded() &&
      rtl_create_user_process(ldr_shms, UserProcess::ExecutableKind::kExecutableDylibKind) !=
          kCPSInvalidPID) {
    (Void)(kout << "hal_real_init: Spawned the NeAnt Shims Host.\r");
  } else {
    (Void)(kout << "hal_real_init: warning: Shims host did not spawn.\r");
    ke_stop(RUNTIME_CHECK_BOOTSTRAP, "Bug-Check failed at Shims DLL Host Init in HAL.");
  }
}

EXTERN_C Ne::Kernel::VoidPtr kInterruptVectorTable[];

/// @brief Ne::Kernel init function.
/// @param handover_hdr Handover boot header.
EXTERN_C Ne::Kernel::Int32 hal_init_platform(Ne::Kernel::HEL::BootInfoHeader* handover_hdr) {
  using namespace Ne::Kernel;

  if (handover_hdr->f_Magic != kHandoverMagic || handover_hdr->f_Version != kHandoverVersion) {
    return kEfiFail;
  }

  HAL::rt_sti();

  ::fw_init_efi(static_cast<EfiSystemTable*>(
      handover_hdr->f_FirmwareCustomTables[Ne::Kernel::HEL::kHandoverTableST]));

  Boot::ExitBootServices(handover_hdr->f_HardwareTables.f_ImageKey,
                         handover_hdr->f_HardwareTables.f_ImageHandle);

  kHandoverHeader = handover_hdr;

  kKernelVM = kHandoverHeader->f_PageStart;

  if (!kKernelVM) {
    MUST_PASS(kKernelVM);
    return kEfiFail;
  }

  hal_write_cr3(kKernelVM);

  /************************************** */
  /*     INITIALIZE BIT MAP.              */
  /************************************** */

  auto region_base = reinterpret_cast<UIntPtr>(kHandoverHeader->f_BitMapStart);
  auto region_sz   = kHandoverHeader->f_BitMapSize;

  /// @note images map at kPefBaseOrigin over the identity map, shadowing whatever
  /// phys lives there. Keep the heap and the frames clear of that window.
  constexpr UIntPtr kImgWinStart = kPefBaseOrigin;
  constexpr UIntPtr kImgWinEnd   = kPefBaseOrigin + kPefMaxImageSz;

  if (region_base < kImgWinEnd && region_base + region_sz > kImgWinStart) {
    auto below = region_base < kImgWinStart ? kImgWinStart - region_base : (UIntPtr) 0UL;
    auto above =
        region_base + region_sz > kImgWinEnd ? region_base + region_sz - kImgWinEnd : (UIntPtr) 0UL;

    if (below >= above) {
      region_sz = below;
    } else {
      region_base = kImgWinEnd;
      region_sz   = above;
    }
  }

  auto usable_sz = region_sz / 2;

  kBitMapCursor     = 0UL;
  kKernelBitMpSize  = usable_sz;
  kKernelBitMpStart = reinterpret_cast<VoidPtr>(region_base);

  HAL::pmmi_init(region_base + usable_sz, region_sz - usable_sz);

  /************************************** */
  /*     ADOPT OUR OWN PAGE TABLES.       */
  /************************************** */

  constexpr UIntPtr kMinMapLimit = 0x100000000UL;
  constexpr UIntPtr kGiBMask     = 0x3FFFFFFFUL;

  auto ram_end =
      reinterpret_cast<UIntPtr>(kHandoverHeader->f_BitMapStart) + kHandoverHeader->f_BitMapSize;

  auto map_limit = ram_end > kMinMapLimit ? ((ram_end + kGiBMask) & ~kGiBMask) : kMinMapLimit;

  auto kernel_pml4 = HAL::mm_init_kernel_tables(map_limit);

  if (!kernel_pml4) {
    ke_stop(RUNTIME_CHECK_BOOTSTRAP, "Can't build the kernel page tables.");
  }

  kKernelVM = reinterpret_cast<VoidPtr>(kernel_pml4);

  hal_write_cr3(kKernelVM);

  /************************************** */
  /*     INITIALIZE GDT AND SEGMENTS. */
  /************************************** */

  STATIC CONST auto kGDTEntriesCount = 8;

  STATIC HAL::Detail::NE_TSS kKernelTSS{};

  if (!kHandoverHeader->f_StackTop) {
    ke_stop(RUNTIME_CHECK_BOOTSTRAP, "No ring 0 stack in handover.");
  }

  STATIC UInt8 ALIGN(0x10) kFaultStack[kib_cast(32)]{};

  kKernelTSS.fRsp0 = (UInt64) kHandoverHeader->f_StackTop;
  kKernelTSS.fIst1 = (UInt64) (kFaultStack + sizeof(kFaultStack)) & ~0xFUL;
  kKernelTSS.fIopb = sizeof(HAL::Detail::NE_TSS);

  /* The GDT, mostly descriptors for user and kernel segments. */
  STATIC HAL::Detail::NE_GDT_ENTRY ALIGN(0x08) kGDTArray[kGDTEntriesCount] = {
      {.fLimitLow   = 0,
       .fBaseLow    = 0,
       .fBaseMid    = 0,
       .fAccessByte = 0x00,
       .fFlags      = 0x00,
       .fBaseHigh   = 0},  // Null entry
      {.fLimitLow   = 0x0,
       .fBaseLow    = 0,
       .fBaseMid    = 0,
       .fAccessByte = 0x9A,
       .fFlags      = 0xAF,
       .fBaseHigh   = 0},  // Ne::Kernel code
      {.fLimitLow   = 0x0,
       .fBaseLow    = 0,
       .fBaseMid    = 0,
       .fAccessByte = 0x92,
       .fFlags      = 0xCF,
       .fBaseHigh   = 0},  // Ne::Kernel data
      {},                // TSS data low
      {},                // TSS data high
      {.fLimitLow   = 0x0,
       .fBaseLow    = 0,
       .fBaseMid    = 0,
       .fAccessByte = 0xFA,
       .fFlags      = 0xAF,
       .fBaseHigh   = 0},  // User code
      {.fLimitLow   = 0x0,
       .fBaseLow    = 0,
       .fBaseMid    = 0,
       .fAccessByte = 0xF2,
       .fFlags      = 0xCF,
       .fBaseHigh   = 0},  // User data
  };

  kGDTArray[3].fLimitLow   = sizeof(HAL::Detail::NE_TSS) - 1;
  kGDTArray[3].fBaseLow    = ((UIntPtr) &kKernelTSS) & 0xFFFF;
  kGDTArray[3].fBaseMid    = (((UIntPtr) &kKernelTSS) >> 16) & 0xFF;
  kGDTArray[3].fAccessByte = 0x89;  // Present, type 9 = 64-bit available TSS
  kGDTArray[3].fFlags      = 0x20 | ((((UIntPtr) &kKernelTSS) >> 24) & 0x0F);
  kGDTArray[3].fBaseHigh   = (((UIntPtr) &kKernelTSS) >> 24) & 0xFF;

  kGDTArray[4].fLimitLow   = ((UIntPtr) &kKernelTSS >> 32) & 0xFFFF;
  kGDTArray[4].fBaseLow    = 0;
  kGDTArray[4].fBaseMid    = 0;
  kGDTArray[4].fAccessByte = 0;
  kGDTArray[4].fFlags      = 0;
  kGDTArray[4].fBaseHigh   = 0;

  FB::cg_clear_video();

  // Load memory descriptors.
  HAL::Register64 gdt_reg;

  gdt_reg.Base  = reinterpret_cast<UIntPtr>(kGDTArray);
  gdt_reg.Limit = (sizeof(HAL::Detail::NE_GDT_ENTRY) * kGDTEntriesCount) - 1;

  //! GDT will load hal_read_init after it successfully loads the segments.
  HAL::GDTLoader gdt_loader;
  gdt_loader.Load(gdt_reg);

  return kEfiFail;
}

EXTERN_C BOOL rtl_init_nic_rtl8139();

#ifdef __DEBUG__
/// @brief Check the frame allocator's invariants on real memory.
STATIC Ne::Kernel::Void pmmi_self_test(Ne::Kernel::Void) {
  using namespace Ne::Kernel;

  auto a = HAL::pmmi_alloc_frame();
  auto b = HAL::pmmi_alloc_frame();

  if (!a || !b) {
    (Void)(kout << "pmm: FAIL out of memory\r");
    return;
  }

  if ((a & (kPageSize - 1)) || (b & (kPageSize - 1))) (Void)(kout << "pmm: FAIL alignment\r");
  if (a == b) (Void)(kout << "pmm: FAIL duplicate frame\r");

  for (SizeT i = 0UL; i < kPageSize; ++i) {
    if (reinterpret_cast<UInt8*>(a)[i] != 0) {
      (Void)(kout << "pmm: FAIL frame not zeroed\r");
      break;
    }
  }

  rt_set_memory(reinterpret_cast<VoidPtr>(b), 0xAB, kPageSize);
  HAL::pmmi_free_frame(b);

  auto c = HAL::pmmi_alloc_frame();

  if (c != b) (Void)(kout << "pmm: FAIL free list did not reuse\r");
  if (c && reinterpret_cast<UInt8*>(c)[8] != 0) (Void)(kout << "pmm: FAIL reuse not zeroed0\r");

  (Void)(kout << "pmm: self test done, free " << number(HAL::pmmi_free_frames()) << kendl);
}
#endif  // __DEBUG__

/// @brief Liveness probe, returns the handover magic to its caller.
STATIC Ne::Kernel::VoidPtr ke_ping(Ne::Kernel::VoidPtr arg) {
  NE_UNUSED(arg);
  return (Ne::Kernel::VoidPtr) kHandoverMagic;
}

EXTERN_C Ne::Kernel::Void hal_real_init(Ne::Kernel::Void) {
  HAL::mp_init_cores(kHandoverHeader->f_HardwareTables.f_VendorPtr);

  HAL::Register64 idt_reg;
  idt_reg.Base = reinterpret_cast<UIntPtr>(kInterruptVectorTable);

  HAL::IDTLoader idt_loader;
  idt_loader.Load(idt_reg);

#ifdef __DEBUG__
  pmmi_self_test();
#endif  // __DEBUG__

  user_init_std(kHandoverHeader->f_RecoverMode);

  ke_install_syscall("ke_ping", ke_ping);

  STATIC Char* kCanary = new Char[512];
  if (!kCanary) return;
  
  rt_set_memory(kCanary, 0xDE, 512); 

#ifdef __FSKIT_INCLUDES_OPENHEFS__
  OpenHeFS::fs_init_openhefs();
  HeFileSystemMgr::Mount(new HeFileSystemMgr());
#endif

#ifdef __FSKIT_INCLUDES_NEFS__
  NeFS::fs_init_nefs();
  NeFileSystemMgr::Mount(new NeFileSystemMgr());
#endif

  kei_init_drivers();

  UserProcessScheduler::The().SwitchTeam(kRTUserTeam);

  if (kHandoverHeader->f_HostImage && kHandoverHeader->f_HostImageSz) {
    PE32Loader ldr(kHandoverHeader->f_HostImage, kHandoverHeader->f_HostImageSz);

    if (ldr.IsLoaded() &&
        rtl_create_user_process(ldr, UserProcess::ExecutableKind::kExecutableKind) !=
            kCPSInvalidPID) {
      (Void)(kout << "hal_real_init: Spawned the NeAnt Launch Host.\r");

#ifdef __HALKIT_INCLUDES_BNID__
      rtl_init_nic_rtl8139();
#endif
    } else {
      (Void)(kout << "hal_real_init: warning: Launch host did not spawn.\r");
      ke_stop(RUNTIME_CHECK_BOOTSTRAP, "Bug-Check failed at Kernel Main in HAL.");
    }
  } else {
    (Void)(kout << "hal_real_init: warning: No launch host in handover.\r");
    ke_stop(RUNTIME_CHECK_BOOTSTRAP, "Bug-Check failed at Kernel Main in HAL.");
  }

  /// @note SwitchTeam overwrites the whole team, switching again here would
  /// discard the process we just spawned.
  
  ke_stop(RUNTIME_CHECK_BOOTSTRAP, "Bug-Check failed at Kernel Main in HAL.");
  while (YES) rt_cli();
}
#endif  // ifndef __NE_MODULAR_KERNEL_COMPONENTS__
