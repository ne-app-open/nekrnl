// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <ArchKit/ArchKit.h>
#include <KernelKit/DebugOutput.h>
#include <KernelKit/HeapMgr.h>
#include <KernelKit/KPC.h>
#include <KernelKit/LockDelegate.h>
#include <NeKit/Crc32.h>
#include <NeKit/PageMgr.h>
#include <NeKit/Utils.h>

/* ========================================

 Revision History:
  10/8/24: FIX: Fix useless long name, alongside a new WR (WriteRead) field.
  20/10/24: FIX: Fix mm_new_ and mm_delete_ APIs inside HeapMgr.h header. (amlal)
  27/01/25: REFACTOR: Reworked code as the memory manager.
  25/03/25: REFACTOR: Refactor HeapMgr code and log freed address location.

 ======================================== */

//! @file HeapMgr.cpp
//! @brief Heap system that serves as the main memory manager.

#define kHeapMgrMagic (0xD4D75)
#define kHeapMgrAlignSz (4U)

#define kHeapMgrSpinMax (255'000)

namespace Ne::Kernel {

/// @brief Implementation details.
namespace Detail {
  struct PACKED MM_INFORMATION_BLOCK;

  /// @brief Ne::Kernel heap information block.
  /// Located before the address bytes.
  /// | HIB |  CLASS/STRUCT/DATA TYPES... |
  struct PACKED MM_INFORMATION_BLOCK final {
    ///! @brief 32-bit value which contains the magic number of the heap.
    UInt32 fMagic : 24;
    ///! @brief Is the heap present?
    UInt8 fPresent : 1;
    /// @brief Is this value writable?
    UInt8 fWriteRead : 1;
    /// @brief Is this value owned by the user?
    UInt8 fUser : 1;
    /// @brief Is this a page pointer?
    UInt8 fPage : 1;
    /// @brief 32-bit CRC checksum.
    UInt32 fCRC32;
    /// @brief 64-bit Allocation flags.
    UInt16 fFlags;
    /// @brief 64-bit pointer size.
    SizeT fSize;
    /// @brief 64-bit target offset pointer.
    UIntPtr fOffset;
    /// @brief Padding count.
    UInt32 fPad;
    /// @brief Padding bytes for header.
    UInt8 fPadding[kHeapMgrAlignSz];
  };

  typedef MM_INFORMATION_BLOCK* MM_INFORMATION_BLOCK_PTR;

  /// @brief Distance between a bitmap block base and the pointer we hand out.
  /// @note the first header steps over the bitmap's own magic/size/used words.
  inline constexpr auto kHeapMgrHdrSz = sizeof(MM_INFORMATION_BLOCK) * 2;

  static_assert(sizeof(MM_INFORMATION_BLOCK) > sizeof(UIntPtr) * 3,
                "heap header would overwrite the bitmap header");

  /// @brief Check that a heap pointer still refers to a live allocation.
  /// @return Bool if the pointer is valid or not.
  /// @param heap_ptr The address_ptr to check.
  _Output auto mm_check_ptr_address(VoidPtr heap_ptr) -> Bool {
    if (!heap_ptr) return No;

    /// Reject anything too low to carry our headers, that would underflow. ///
    if ((UIntPtr) heap_ptr <= kHeapMgrHdrSz) return No;

    if (!HAL::mm_is_bitmap((VoidPtr) ((UIntPtr) heap_ptr - kHeapMgrHdrSz))) return No;

    auto heap_info_ptr = reinterpret_cast<MM_INFORMATION_BLOCK_PTR>((UIntPtr) heap_ptr -
                                                                    sizeof(MM_INFORMATION_BLOCK));

    return heap_info_ptr->fMagic == kHeapMgrMagic && heap_info_ptr->fPresent;
  }
}  // namespace Detail

STATIC PageMgr kPageMgr;

/// @brief Allocate chunk of memory.
/// @param sz Size of pointer
/// @param wr Read Write bit.
/// @param user User enable bit.
/// @return The newly allocated pointer.
_Output VoidPtr mm_alloc_ptr(SizeT sz, Bool wr, Bool user, SizeT pad_amount) {
  STATIC Bool       kAllocationLocked = false;
  while (kAllocationLocked);
  
  auto sz_fix = sz;

  if (sz_fix == 0) return nullptr;

  kAllocationLocked = true;

  /// @note the block has to cover both headers, not just ours.
  sz_fix += Detail::kHeapMgrHdrSz;

  PTEWrapper wrapper = kPageMgr.Request(wr, user, 
                  No, sz_fix, pad_amount);

  Detail::MM_INFORMATION_BLOCK_PTR heap_info_ptr =
      reinterpret_cast<Detail::MM_INFORMATION_BLOCK_PTR>(wrapper.VirtualAddress() +
                                                         sizeof(Detail::MM_INFORMATION_BLOCK));

  heap_info_ptr->fSize  = sz_fix;
  heap_info_ptr->fMagic = kHeapMgrMagic;
  heap_info_ptr->fCRC32 = 0U;  // dont fill it for now.
  heap_info_ptr->fOffset =
      reinterpret_cast<UIntPtr>(heap_info_ptr) + sizeof(Detail::MM_INFORMATION_BLOCK);
  heap_info_ptr->fPage      = No;
  heap_info_ptr->fWriteRead = wr;
  heap_info_ptr->fUser      = user;
  heap_info_ptr->fPresent   = Yes;
  heap_info_ptr->fPad       = pad_amount;

  rt_set_memory(heap_info_ptr->fPadding, 0, kHeapMgrAlignSz);

  auto result = reinterpret_cast<VoidPtr>(heap_info_ptr->fOffset);

  if (result)
    (Void)(kout << "HeapMgr: Registered heap address: "
                << hex_number(reinterpret_cast<UIntPtr>(heap_info_ptr)) << kendl);

  kAllocationLocked = false;

  return result;
}

/// @brief Controls the page's heap.
/// @param heap_ptr the pointer to make a page heap.
/// @return kErrorSuccess if successful, otherwise an error code.
_Output Int32 mm_ctl_page(VoidPtr heap_ptr, const Bool enable) {
  if (Detail::mm_check_ptr_address(heap_ptr) == No) return kErrorHeapNotPresent;

  Detail::MM_INFORMATION_BLOCK_PTR heap_info_ptr =
      reinterpret_cast<Detail::MM_INFORMATION_BLOCK_PTR>((UIntPtr) heap_ptr -
                                                         sizeof(Detail::MM_INFORMATION_BLOCK));

  if (!heap_info_ptr) return kErrorHeapNotPresent;

  heap_info_ptr->fPage = enable;
  return kErrorSuccess;
}

/// @brief Overwrites and set the flags of a heap header.
/// @param heap_ptr the pointer to update.
/// @param flags the flags to set.
_Output Int32 mm_set_ptr_flags(VoidPtr heap_ptr, UInt64 flags) {
  if (Detail::mm_check_ptr_address(heap_ptr) == No) return kErrorHeapNotPresent;

  Detail::MM_INFORMATION_BLOCK_PTR heap_info_ptr =
      reinterpret_cast<Detail::MM_INFORMATION_BLOCK_PTR>((UIntPtr) heap_ptr -
                                                         sizeof(Detail::MM_INFORMATION_BLOCK));

  if (!heap_info_ptr) return kErrorHeapNotPresent;

  heap_info_ptr->fFlags = flags;

  return kErrorSuccess;
}

/// @brief Gets the flags of a heap header.
/// @param heap_ptr the pointer to get.
_Output UInt64 mm_get_ptr_flags(VoidPtr heap_ptr) {
  Detail::MM_INFORMATION_BLOCK_PTR heap_info_ptr =
      reinterpret_cast<Detail::MM_INFORMATION_BLOCK_PTR>((UIntPtr) heap_ptr -
                                                         sizeof(Detail::MM_INFORMATION_BLOCK));

  if (!heap_info_ptr) return kErrorHeapNotPresent;

  return heap_info_ptr->fFlags;
}

/// @brief Declare pointer as free.
/// @param heap_ptr the pointer.
/// @return
_Output Int32 mm_free_ptr(VoidPtr heap_ptr) {
  if (!heap_ptr) return kErrorHeapNotPresent;

  if ((UIntPtr) heap_ptr <= Detail::kHeapMgrHdrSz) return kErrorHeapNotPresent;

  VoidPtr base_ptr = (VoidPtr) ((UIntPtr) heap_ptr - Detail::kHeapMgrHdrSz);

  if (!HAL::mm_is_bitmap(base_ptr)) return kErrorHeapNotPresent;

  Detail::MM_INFORMATION_BLOCK_PTR heap_info_ptr =
      reinterpret_cast<Detail::MM_INFORMATION_BLOCK_PTR>((UIntPtr) (heap_ptr) -
                                                         sizeof(Detail::MM_INFORMATION_BLOCK));

  if (heap_info_ptr->fMagic != kHeapMgrMagic) return kErrorHeapNotPresent;

  /// @note the magic survives a free, so a second one lands here.
  if (!heap_info_ptr->fPresent) {
    ke_stop(RUNTIME_CHECK_TLS, "Double-Free Detected on HeapMgr, aborting.");
  }

  heap_info_ptr->fSize      = 0UL;
  heap_info_ptr->fPresent   = No;
  heap_info_ptr->fOffset    = 0;
  heap_info_ptr->fCRC32     = 0;
  heap_info_ptr->fWriteRead = No;
  heap_info_ptr->fUser      = No;
  heap_info_ptr->fPad       = 0;

  (Void)(kout << "HeapMgr: Freed heap address: "
              << hex_number(reinterpret_cast<UIntPtr>(heap_info_ptr)) << kendl);

  PTEWrapper page_wrapper(No, No, No, reinterpret_cast<UIntPtr>(base_ptr));

  Ref<PTEWrapper> pte_address{page_wrapper};

  kPageMgr.Free(pte_address);

  return kErrorSuccess;
}

/// @brief Check if pointer is a valid Ne::Kernel pointer.
/// @param heap_ptr the pointer
/// @return if it exists.
_Output Boolean mm_is_valid_ptr(VoidPtr heap_ptr) {
  return Detail::mm_check_ptr_address(heap_ptr);
}

/// @brief Protect the heap with a CRC value.
/// @param heap_ptr HIB pointer.
/// @return if it valid: point has crc now., otherwise fail.
_Output Boolean mm_protect_ptr(VoidPtr heap_ptr) {
  if (heap_ptr) {
    Detail::MM_INFORMATION_BLOCK_PTR heap_info_ptr =
        reinterpret_cast<Detail::MM_INFORMATION_BLOCK_PTR>((UIntPtr) heap_ptr -
                                                           sizeof(Detail::MM_INFORMATION_BLOCK));

    if (heap_info_ptr && heap_info_ptr->fPresent && kHeapMgrMagic == heap_info_ptr->fMagic) {
      heap_info_ptr->fCRC32 =
          ke_calculate_crc32((Char*) heap_info_ptr, sizeof(Detail::MM_INFORMATION_BLOCK));

      return Yes;
    }
  }

  return No;
}

}  // namespace Ne::Kernel

// END
