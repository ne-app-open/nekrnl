// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <BootKit/BootKit.h>
#include <BootKit/BootThread.h>
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

/** Ring 0 stack handed to the kernel's TSS. */

#define kBootStackSz (0x40000U)

/** Graphics related. */

STATIC EfiGraphicsOutputProtocol* kGop       = nullptr;
STATIC UInt16                     kGopStride = 0U;
STATIC EFI_GUID                   kGopGuid;

/** Related to jumping to the reset vector. */

EXTERN_C Void rt_reset_hardware();

EXTERN_C Ne::Kernel::VoidPtr boot_read_cr3();  // @brief Page directory inside cr3 register.

/**
  @brief Finds and stores the GOP object.
*/
STATIC Bool boot_init_fb() {
  kGopGuid = EFI_GUID(EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID);
  kGop     = nullptr;

  if (BS->LocateProtocol(&kGopGuid, nullptr, (VoidPtr*) &kGop) != kEfiOk) return No;

  // TODO: Ok that ain't great, open to fixes.
  kGopStride = 4;

  return Yes;
}

EFI_GUID kEfiGlobalNamespaceVarGUID = {
    0x8BE4DF61, 0x93CA, 0x11D2, {0xAA, 0x0D, 0x00, 0xE0, 0x98, 0x03, 0x2B, 0x8C}};

/// @brief Find the largest block of free memory and hand it to the kernel.
/// @note run this AFTER every image is placed, or the firmware still reports the
/// memory those images occupy as free and the kernel allocates over itself.
STATIC Void boot_scan_memory(HEL::BootInfoHeader* handover_hdr, UIntPtr* out_map_key) {
  Boot::BootTextWriter writer;

  UIntPtr              map_key         = 0;
  UIntPtr              size_struct_ptr = 0;
  UIntPtr              sz_desc         = sizeof(EfiMemoryDescriptor);
  UIntPtr              rev_desc        = 0;
  EfiMemoryDescriptor* struct_ptr      = nullptr;

  BS->GetMemoryMap(&size_struct_ptr, nullptr, &map_key, &sz_desc, &rev_desc);

  /// @note AllocatePool itself grows the map, so ask for room to spare or the
  /// second call fails and leaves the buffer untouched.
  size_struct_ptr += sz_desc * 8;

  if (BS->AllocatePool(EfiLoaderData, size_struct_ptr, (VoidPtr*) &struct_ptr) != kEfiOk ||
      !struct_ptr) {
    writer.Write("BootZ: Can't allocate the memory map, can't boot to NeKernel.\r");
    Boot::Stop();
  }

  if (BS->GetMemoryMap(&size_struct_ptr, struct_ptr, &map_key, &sz_desc, &rev_desc) != kEfiOk) {
    writer.Write("BootZ: Can't read the memory map, can't boot to NeKernel.\r");
    Boot::Stop();
  }

  UInt64  free_pages      = 0;
  VoidPtr first_free_page = nullptr;

  constexpr UInt64 kBootLowMemEnd = 0x100000;

  /// @note one region, not the sum of every region. Summing them yields a size the
  /// kernel does not own the moment the map is fragmented.
  for (UIntPtr i = 0; i < size_struct_ptr / sz_desc; ++i) {
    EfiMemoryDescriptor* desc = (EfiMemoryDescriptor*) ((UInt8*) struct_ptr + (i * sz_desc));

    if (desc->Kind != EfiConventionalMemory) continue;
    if (desc->PhysicalStart < kBootLowMemEnd) continue;

    if (desc->NumberOfPages > free_pages) {
      free_pages      = desc->NumberOfPages;
      first_free_page = (VoidPtr) desc->PhysicalStart;
    }
  }

  if (!first_free_page || free_pages < 1) {
    writer.Write("BootZ: No conventional memory, can't boot to NeKernel.\r");
    Boot::Stop();
  }

  handover_hdr->f_BitMapStart = (VoidPtr)((UIntPtr)first_free_page + mib_cast(8));
  handover_hdr->f_BitMapSize  = free_pages * 4096;

  if (out_map_key) *out_map_key = map_key;

  writer.Write("BootZ: Usable memory: ").Write(free_pages * 4096).Write("\r");
}

/// @brief BootloaderMain EFI entrypoint.
/// @param image_handle Handle of this image.
/// @param sys_table The system table of it.
/// @return nothing, never returns.
EFI_EXTERN_C EFI_API Int32 BootloaderMain(EfiHandlePtr image_handle, EfiSystemTable* sys_table) {
  if (!image_handle || !sys_table) return kEfiFail;

  fw_init_efi(sys_table);  ///! Init the EFI library.

  ST->ConOut->ClearScreen(sys_table->ConOut);
  ST->ConOut->SetAttribute(sys_table->ConOut, kEFIYellow);

  ST->BootServices->SetWatchdogTimer(0, 0, 0, nullptr);
  ST->ConOut->EnableCursor(ST->ConOut, false);

  HEL::BootInfoHeader* handover_hdr = new HEL::BootInfoHeader();

  UIntPtr map_key = 0;

  Boot::BootTextWriter writer;

  if (!boot_init_fb()) {
    writer.Write("BootZ: Invalid Framebuffer, can't boot to NeKernel.\r");
    Boot::Stop();
  }

  writer.Write("BootZ: The Ne.app NeKernel Loader. Copyright 2024-2026, Amlal El Mahrouss, Ne.app et al.\r");

  STATIC Bool kAcpiDetectedMandatory = FALSE;

  for (SizeT index_vt = 0; index_vt < sys_table->NumberOfTableEntries; ++index_vt) {
    Char* vendor_table =
        reinterpret_cast<Char*>(sys_table->ConfigurationTable[index_vt].VendorTable);

    // ACPI's 'RSD PTR', which contains the ACPI SDT (MADT, FACP...)
    if (vendor_table[0] == 'R' && vendor_table[1] == 'S' && vendor_table[2] == 'D' &&
        vendor_table[3] == ' ' && vendor_table[4] == 'P' && vendor_table[5] == 'T' &&
        vendor_table[6] == 'R' && vendor_table[7] == ' ') {
      handover_hdr->f_HardwareTables.f_VendorPtr = (VoidPtr) vendor_table;
      kAcpiDetectedMandatory                     = YES;
      break;
    }
  }

  if (!kAcpiDetectedMandatory) {
    writer.Write("BootZ: Starting from NeAnt v1.6+. ACPI is required to boot on UEFI.\r");
    Boot::Stop();
  }

  // ------------------------------------------ //
  // draw background color.
  // ------------------------------------------ //

  handover_hdr->f_GOP.f_The          = kGop->Mode->FrameBufferBase;
  handover_hdr->f_GOP.f_Width        = kGop->Mode->Info->VerticalResolution;
  handover_hdr->f_GOP.f_Height       = kGop->Mode->Info->HorizontalResolution;
  handover_hdr->f_GOP.f_PixelPerLine = kGop->Mode->Info->PixelsPerScanLine;
  handover_hdr->f_GOP.f_PixelFormat  = kGop->Mode->Info->PixelFormat;
  handover_hdr->f_GOP.f_Size         = kGop->Mode->FrameBufferSize;

  // ------------------------------------------- //
  // Grab MP services, extended to runtime.	   //
  // ------------------------------------------- //

  EFI_GUID               guid_mp = EFI_GUID(EFI_MP_SERVICES_PROTOCOL_GUID);
  EfiMpServicesProtocol* mp      = nullptr;

  BS->LocateProtocol(&guid_mp, nullptr, reinterpret_cast<VoidPtr*>(&mp));

  handover_hdr->f_HardwareTables.f_MpPtr = reinterpret_cast<VoidPtr>(mp);

  kHandoverHeader = handover_hdr;

  FB::cg_clear_video();

  UInt32 cnt_enabled  = 0;
  UInt32 cnt_disabled = 0;

  if (mp) {
    mp->GetNumberOfProcessors(mp, &cnt_disabled, &cnt_enabled);
    kHandoverHeader->f_NumberOfProcessors                   = cnt_enabled;
    handover_hdr->f_HardwareTables.f_MultiProcessingEnabled = YES;
  } else {
    handover_hdr->f_NumberOfProcessors                      = 1;
    handover_hdr->f_HardwareTables.f_MultiProcessingEnabled = NO;
  }

  // Fill handover header now.

  boot_scan_memory(handover_hdr, &map_key);

  handover_hdr->f_FirmwareCustomTables[Ne::Kernel::HEL::kHandoverTableBS] = (VoidPtr) BS;
  handover_hdr->f_FirmwareCustomTables[Ne::Kernel::HEL::kHandoverTableST] = (VoidPtr) ST;

  handover_hdr->f_Magic   = kHandoverMagic;
  handover_hdr->f_Version = kHandoverVersion;

  handover_hdr->f_HardwareTables.f_ImageKey    = map_key;
  handover_hdr->f_HardwareTables.f_ImageHandle = image_handle;

  UIntPtr sz_recover_mode = sizeof(Bool);
  Bool    recover_mode    = 0;

  ST->RuntimeServices->GetVariable(L"/props/recover_mode", kEfiGlobalNamespaceVarGUID, nullptr,
                                   &sz_recover_mode, &recover_mode);

  handover_hdr->f_RecoverMode = recover_mode;

  /* one shot flag, attributes 0 delete it. */
  ST->RuntimeServices->SetVariable(L"/props/recover_mode", kEfiGlobalNamespaceVarGUID, 0,
                                   &sz_recover_mode, &recover_mode);

  // Ring 0 stack, consumed by the kernel as TSS.RSP0. Stacks grow down, so hand over the top.

  VoidPtr stack_base = nullptr;

  if (BS->AllocatePool(EfiLoaderData, kBootStackSz, &stack_base) != kEfiOk || !stack_base) {
    writer.Write("BootZ: Can't allocate the kernel stack, can't boot to NeKernel.\r");
    Boot::Stop();
  }

  Boot::BSetMem((Char*) stack_base, 0, kBootStackSz);

  handover_hdr->f_StackSz  = kBootStackSz;
  handover_hdr->f_StackTop = (VoidPtr) (((UIntPtr) stack_base + kBootStackSz) & ~0xFUL);

  // ------------------------------------------ //
  // If we succeed in reading the blob, then execute it.
  // ------------------------------------------ //

  // Provide fimware vendor name.

  Boot::BCopyMem(handover_hdr->f_FirmwareVendorName, sys_table->FirmwareVendor,
                 handover_hdr->f_FirmwareVendorLen);

  handover_hdr->f_FirmwareVendorLen = Boot::BStrLen(sys_table->FirmwareVendor);
  // Assign to global 'kHandoverHeader'.

  Boot::BootFileReader reader_memtest(L"memtest.efi", image_handle);
  reader_memtest.ReadAll(0);

  if (reader_memtest.Blob()) {
    auto memtest_thread = Boot::BootThread(reader_memtest.Blob());
    memtest_thread.SetName("MemoryTest");
    memtest_thread.Start(handover_hdr, NO);
  }

  WideChar kernel_path[256U] = L"vmoskrnl.exe";
  UIntPtr  kernel_path_sz    = sizeof(kernel_path);

  /// access attributes (in order)
  /// EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS
  constexpr UInt32 kVarAttrs = 0x00000001 | 0x00000002 | 0x00000004;

  UIntPtr sz_ver = sizeof(UInt64);
  UInt64  ver    = KERNEL_VERSION_BCD;

  ST->RuntimeServices->GetVariable(L"/props/kern_ver", kEfiGlobalNamespaceVarGUID, nullptr, &sz_ver,
                                   &ver);

  UIntPtr sz_smp_max = sizeof(UInt64);
  UInt64  smp_max    = 0;

  ST->RuntimeServices->GetVariable(L"/props/smp_max", kEfiGlobalNamespaceVarGUID, nullptr,
                                   &sz_smp_max, &smp_max);

  /// This variable makes sure we enable the core we want to actually use.
  if (smp_max > 0 && smp_max < kHandoverHeader->f_NumberOfProcessors) {
    kHandoverHeader->f_NumberOfProcessors = smp_max;
  }

  if (ver < KERNEL_VERSION_BCD) {
    ver = KERNEL_VERSION_BCD;

    UIntPtr ver_sz = sizeof(UInt64);

    ST->RuntimeServices->SetVariable(L"/props/kern_ver", kEfiGlobalNamespaceVarGUID, kVarAttrs,
                                     &ver_sz, &ver);

    writer.Write("BootZ: Version has been updated: ").Write(ver).Write("\r");

    if (ST->RuntimeServices->GetVariable(L"/props/kernel_path", kEfiGlobalNamespaceVarGUID, nullptr,
                                         &kernel_path_sz, kernel_path) != kEfiOk) {
      UIntPtr len_t = (Boot::BStrLen(kernel_path) + 1) * sizeof(WideChar);
      ST->RuntimeServices->SetVariable(L"/props/kernel_path", kEfiGlobalNamespaceVarGUID, kVarAttrs,
                                       &len_t, kernel_path);
    }
  } else {
    writer.Write("BootZ: Version: ").Write(ver).Write("\r");
  }

  // boot to kernel, if not bootnet this.

  Boot::BootFileReader reader_libsys(L"libSystem.dll", image_handle);
  reader_libsys.ReadAll(0);

  if (reader_libsys.Blob()) {
    handover_hdr->f_SCIImage   = reader_libsys.Blob();
    handover_hdr->f_SCIImageSz = reader_libsys.Size();

    writer.Write("BootZ: Loaded libSystem.dll.\r");
  } else {
    handover_hdr->f_SCIImage   = nullptr;
    handover_hdr->f_SCIImageSz = 0UL;

    writer.Write("BootZ: No libSystem.dll, booting without a user process.\r");
  }

  Boot::BootFileReader reader_hal(L"hal.x64.dll", image_handle);
  reader_hal.ReadAll(0);

  if (reader_hal.Blob()) {
    handover_hdr->f_HALImage = reader_hal.Blob();
    handover_hdr->f_HALSz = reader_hal.Size();
  }

  Boot::BootFileReader reader_osdetect(L"mindetect.efi", image_handle);
  reader_osdetect.ReadAll(0);

  Boot::BootThread* osdetect_thread = nullptr;

  if (reader_osdetect.Blob()) {
    osdetect_thread = new Boot::BootThread(reader_osdetect.Blob());
    osdetect_thread->SetName("OS Detect");

    auto ret = osdetect_thread->Start(handover_hdr, NO);

    if (ret == kEfiFail) {
      UIntPtr sz_prob_cnt = sizeof(UInt64);
      UInt64  prob_cnt    = 0;

      ST->RuntimeServices->GetVariable(L"/props/problems_detected_cnt", kEfiGlobalNamespaceVarGUID,
                                       nullptr, &sz_prob_cnt, &prob_cnt);

      ++prob_cnt;

      ST->RuntimeServices->SetVariable(L"/props/problems_detected_cnt", kEfiGlobalNamespaceVarGUID,
                                       0, &sz_prob_cnt, &prob_cnt);
    }
  }

  Boot::BootFileReader reader_kernel(kernel_path, image_handle);
  reader_kernel.ReadAll(0);

  // ------------------------------------------ //
  // If we succeed in reading the blob, then execute it.
  // ------------------------------------------ //

  if (reader_kernel.Blob()) {
    handover_hdr->f_PageStart = boot_read_cr3();

    auto kernel_thread = Boot::BootThread(reader_kernel.Blob());

    kernel_thread.SetName("Ne.app NeKernel");

    handover_hdr->f_KernelImage = reader_kernel.Blob();
    handover_hdr->f_KernelSz    = reader_kernel.Size();

    boot_scan_memory(handover_hdr, &map_key);

    handover_hdr->f_HardwareTables.f_ImageKey = map_key;

    return kernel_thread.Start(handover_hdr, YES);
  }

  Boot::BootFileReader reader_net(L"net.efi", image_handle);
  reader_net.ReadAll(0);

  if (reader_net.Blob()) {
    auto net_thread = new Boot::BootThread(reader_net.Blob());

    if (net_thread) {
      net_thread->SetName("BootNet");
      net_thread->Start(handover_hdr, NO);
    }
  }

  return kEfiFail;
}
