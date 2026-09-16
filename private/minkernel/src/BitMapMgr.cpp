// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifdef __NE_AMD64__
#include <HALKit/AMD64/Paging.h>
#elif defined(__NE_ARM64__)
#include <HALKit/ARM64/Paging.h>
#endif

#include <ArchKit/ArchKit.h>
#include <NeKit/KernelPanic.h>

#define kBitMapMagic (0x10210U)
#define kBitMapMaxSz gib_cast(4)
#define kBitMapMagIdx (0U)
#define kBitMapSizeIdx (1U)
#define kBitMapUsedIdx (2U)
#define kBitMapSz (3U)

///! @author Amlal El Mahrouss (amlal@nekernel.org)

namespace Ne::Kernel {
namespace HAL {
  namespace Detail {
    /***********************************************************************************/
    /// \brief Proxy Interface to manage a bitmap allocator.
    /***********************************************************************************/
    class IBitMapProxy final {
     public:
      explicit IBitMapProxy() = default;
      ~IBitMapProxy()         = default;

      NE_COPY_DELETE(IBitMapProxy)

      auto IsBitMap(VoidPtr page_ptr) -> Bool {
        if (!page_ptr) return No;

        UIntPtr* ptr_bit_set = reinterpret_cast<UIntPtr*>(page_ptr);

        if (ptr_bit_set[kBitMapMagIdx] != kBitMapMagic) return No;

        return Yes;
      }

      auto FreeBitMap(VoidPtr page_ptr) -> Bool {
        if (this->IsBitMap(page_ptr) == No) return No;

        volatile UIntPtr* ptr_bit_set = reinterpret_cast<volatile UIntPtr*>(page_ptr) - kBitMapSz;

        if (!ptr_bit_set) return No;
        if (ptr_bit_set[kBitMapMagIdx] != kBitMapMagic) return No;

        this->GetBitMapStatus(ptr_bit_set);

        if (kBitMapCursor >= ptr_bit_set[kBitMapSizeIdx])
          kBitMapCursor -= ptr_bit_set[kBitMapSizeIdx];

        /// @note magic and size have to survive, FindBitMap walks by them and
        /// would otherwise reclaim this hole at the wrong size.
        ptr_bit_set[kBitMapUsedIdx] = No;

        return Yes;
      }

      UInt32 MakeMMFlags(const Bool& wr, const Bool& user) {
        UInt32 flags = kMMFlagsPresent;

        if (wr) flags |= kMMFlagsWr;
        if (user) flags |= kMMFlagsUser;

        return flags;
      }

      /***********************************************************************************/
      /// @brief Iterate over availables bitmap, until we find a free entry.
      /// @param base_ptr base pointer to look on.
      /// @param size the size of the requested data structure.
      /// @param wr is writable flag?
      /// @param user is user flag?
      /// @param pad additional padding added to **size**
      /// @return The new free address, or nullptr.
      /***********************************************************************************/
      auto FindBitMap(VoidPtr base_ptr, SizeT size, Bool wr, Bool user, SizeT pad) -> VoidPtr {
        if (!size) return nullptr;
        if (size > kBitMapMaxSz) return nullptr;

        VoidPtr base = reinterpret_cast<VoidPtr>((UIntPtr)base_ptr + kBitMapCursor);

        STATIC SizeT biggest{0UL};

        while (YES) {
          volatile UIntPtr* ptr_bit_set = reinterpret_cast<volatile UIntPtr*>(base);

          if (ptr_bit_set[kBitMapMagIdx] == kBitMapMagic) {
            if (ptr_bit_set[kBitMapUsedIdx] == No) {
              this->GetBitMapStatus(ptr_bit_set);

              ptr_bit_set[kBitMapSizeIdx] = size + pad;
              ptr_bit_set[kBitMapUsedIdx] = Yes;

              NE_UNUSED(wr);
              NE_UNUSED(user);

              if (biggest < (size + pad)) biggest = size + pad;
              kBitMapCursor += size + pad + kAlign;

              return (VoidPtr) (ptr_bit_set + kBitMapSz);
            }
          } else if (ptr_bit_set[kBitMapMagIdx] != kBitMapMagic) {
            if (ptr_bit_set[kBitMapUsedIdx] == No) {
              ptr_bit_set[kBitMapMagIdx]  = kBitMapMagic;
              ptr_bit_set[kBitMapSizeIdx] = (size + pad);
              ptr_bit_set[kBitMapUsedIdx] = Yes;
              this->GetBitMapStatus(ptr_bit_set);

              NE_UNUSED(wr);
              NE_UNUSED(user);

              if (biggest < (size + pad)) biggest = (size + pad);
              kBitMapCursor += size + pad + kAlign;

              return (VoidPtr) (ptr_bit_set + kBitMapSz);
            }
          }

          UIntPtr raw_base = reinterpret_cast<UIntPtr>(base);

          UIntPtr offset = (ptr_bit_set[kBitMapMagIdx] != kBitMapMagic)
                               ? (size + pad)
                               : ptr_bit_set[kBitMapSizeIdx];

          base = reinterpret_cast<VoidPtr>(raw_base + offset + kAlign);
        }

        return nullptr;
      }

      /// @brief Print Bitmap status
      auto GetBitMapStatus(volatile UIntPtr* ptr_bit_set) -> Void {
        (Void)(kout << "Magic: " << hex_number(ptr_bit_set[kBitMapMagIdx]) << kendl);
        (Void)(kout << "Is Allocated? " << (ptr_bit_set[kBitMapUsedIdx] ? "YES" : "NO") << kendl);
        (Void)(kout << "Size of BitMap (B): " << number(ptr_bit_set[kBitMapSizeIdx]) << kendl);
        (Void)(kout << "Size of BitMap (KIB): " << number(KIB(ptr_bit_set[kBitMapSizeIdx]))
                    << kendl);
        (Void)(kout << "Size of BitMap (MIB): " << number(MIB(ptr_bit_set[kBitMapSizeIdx]))
                    << kendl);
        (Void)(kout << "Size of BitMap (GIB): " << number(GIB(ptr_bit_set[kBitMapSizeIdx]))
                    << kendl);
        (Void)(kout << "Size of BitMap (TIB): " << number(TIB(ptr_bit_set[kBitMapSizeIdx]))
                    << kendl);
        (Void)(kout << "BitMap Address: " << hex_number((UIntPtr) ptr_bit_set) << kendl);
      }
    };
  }  // namespace Detail

  STATIC Detail::IBitMapProxy kBitMapMgr;

  /***********************************************************************************/

  /***********************************************************************************/

  auto mm_is_bitmap(VoidPtr ptr) -> BOOL {
    return kBitMapMgr.IsBitMap(ptr);
  }

  /***********************************************************************************/
  /// @brief Allocate a new page to be used by the OS.
  /// @param wr read/write bit.
  /// @param user user bit.
  /// @return a new bitmap allocated pointer.
  /***********************************************************************************/
  auto mm_alloc_bitmap(Boolean wr, Boolean user, SizeT size, Bool is_page, SizeT pad) -> VoidPtr {
    VoidPtr ptr_new = nullptr;
    if (is_page) return ptr_new;

    ptr_new = kBitMapMgr.FindBitMap(kKernelBitMpStart, size, wr, user, pad);
    return ptr_new;
  }

  /***********************************************************************************/
  /// @brief Free Bitmap, and mark it as absent.
  /// @param ptr the pointer to free.
  /***********************************************************************************/
  auto mm_free_bitmap(VoidPtr ptr) -> Bool {
    if (!ptr) return No;

    Bool ret = kBitMapMgr.FreeBitMap(ptr);
    MUST_PASS(ret);

    return ret;
  }
}  // namespace HAL
}  // namespace Ne::Kernel
