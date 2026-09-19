// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifndef DMAKIT_DMAPOOL_H
#define DMAKIT_DMAPOOL_H

#ifndef __nekernel_halkit_include_processor
#define __nekernel_halkit_include_processor <HALKit/AMD64/Processor.h>
#endif

#include <KernelKit/DebugOutput.h>
#include __nekernel_halkit_include_processor

#define kNeDMAPoolStart (__nekernel_dma_pool_start)
#define kNeDMAPoolSize (__nekernel_dma_pool_size)
#define kNeDMABestAlign (__nekernel_dma_best_align)

namespace Ne::Kernel {

/// @brief DMA pool base pointer, here we're sure that AHCI or whatever tricky standard sees it.
inline UInt8* kDmaPoolPtr = (UInt8*) kNeDMAPoolStart;

/// @brief DMA pool end pointer.
inline const UInt8* kDmaPoolEnd = (UInt8*) (kNeDMAPoolStart + kNeDMAPoolSize);

/***********************************************************************************/
/// @brief allocate from the rtl_dma_alloc system.
/// @param size the size of the chunk to allocate.
/// @param align alignement of pointer.
/***********************************************************************************/
inline VoidPtr rtl_dma_alloc(SizeT size, SizeT align) {
  if (!size) {
    return nullptr;
  }

  /// Check alignement according to architecture.
  if ((align % kNeDMABestAlign) != 0) {
    return nullptr;
  }

  UIntPtr addr = (UIntPtr) kDmaPoolPtr;

  /// here we just align the address according to a `align` variable, i'd rather be a power of two
  /// really.
  addr = (addr + (align - 1)) & ~(align - 1);

  if ((addr + size) > reinterpret_cast<UIntPtr>(kDmaPoolEnd)) {
    err_global_get() = kErrorDmaExhausted;
    return nullptr;
  }

  kDmaPoolPtr = (UInt8*) (addr + size);

  HAL::mm_memory_fence((VoidPtr) addr);

  return (VoidPtr) addr;
}

/***********************************************************************************/
/// @brief Free DMA pointer.
/***********************************************************************************/
inline Void rtl_dma_free(SizeT size) {
  if (!size) return;

  auto ptr = (kDmaPoolPtr - size);

  if (!ptr || ptr < (UInt8*) kNeDMAPoolStart) {
    err_global_get() = kErrorDmaExhausted;
    return;
  }

  kDmaPoolPtr = ptr;

  HAL::mm_memory_fence(ptr);
}

/***********************************************************************************/
/// @brief Flush DMA pointer.
/***********************************************************************************/
inline Void rtl_dma_flush(VoidPtr ptr, SizeT size_buffer) {
  if (ptr > kDmaPoolEnd) {
    return;
  }

  if (!ptr || ptr < (UInt8*) kNeDMAPoolStart) {
    err_global_get() = kErrorDmaExhausted;
    return;
  }

  for (SizeT buf_idx = 0UL; buf_idx < size_buffer; ++buf_idx) {
    HAL::mm_memory_fence((VoidPtr) ((UInt8*) ptr + buf_idx));
  }
}

}  // namespace Ne::Kernel

#endif
