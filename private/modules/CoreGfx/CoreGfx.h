// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifndef COREGFX_COREGFX_H
#define COREGFX_COREGFX_H

#include <NeKit/Config.h>

#define fb_init() Ne::Kernel::UInt32 kCGCursor = 0

#define fb_color(R, G, B) RGB(R, G, B)

#define fb_get_clear_clr() RGB(0, 0, 0)

#define fb_clear() kCGCursor = 0UL

#ifdef __NE_AMD64__
/// @brief Performs Alpha drawing on the framebuffer.
#define FBDrawBitMapInRegionA(reg_ptr, height, width, base_x, base_y)                   \
  for (Ne::Kernel::UInt32 i = base_x; i < (width + base_x); ++i) {                          \
    for (Ne::Kernel::UInt32 u = base_y; u < (height + base_y); ++u) {                       \
      *(((Ne::Kernel::UInt32*) (kHandoverHeader->f_GOP.f_The +                              \
                            4 * kHandoverHeader->f_GOP.f_PixelPerLine * i + 4 * u))) |= \
          (reg_ptr)[kCGCursor];                                                         \
                                                                                        \
      ++kCGCursor;                                                                      \
    }                                                                                   \
  }

/// @brief Performs drawing on the framebuffer.
#define FBDrawBitMapInRegion(reg_ptr, height, width, base_x, base_y)                   \
  for (Ne::Kernel::UInt32 i = base_x; i < (width + base_x); ++i) {                         \
    for (Ne::Kernel::UInt32 u = base_y; u < (height + base_y); ++u) {                      \
      *(((Ne::Kernel::UInt32*) (kHandoverHeader->f_GOP.f_The +                             \
                            4 * kHandoverHeader->f_GOP.f_PixelPerLine * i + 4 * u))) = \
          (reg_ptr)[kCGCursor];                                                        \
                                                                                       \
      ++kCGCursor;                                                                     \
    }                                                                                  \
  }

#define FBDrawBitMapInRegionToRgn(_Rgn, reg_ptr, height, width, base_x, base_y)               \
  for (Ne::Kernel::UInt32 i = base_x; i < (width + base_x); ++i) {                                \
    for (Ne::Kernel::UInt32 u = base_y; u < (height + base_y); ++u) {                             \
      *(((Ne::Kernel::UInt32*) (_Rgn + 4 * kHandoverHeader->f_GOP.f_PixelPerLine * i + 4 * u))) = \
          (reg_ptr)[kCGCursor];                                                               \
                                                                                              \
      ++kCGCursor;                                                                            \
    }                                                                                         \
  }

/// @brief Cleans a resource.
#define FBClearRegion(height, width, base_x, base_y)                                            \
  for (Ne::Kernel::UInt32 i = base_x; i < (width + base_x); ++i) {                                  \
    for (Ne::Kernel::UInt32 u = base_y; u < (height + base_y); ++u) {                               \
      *(((volatile Ne::Kernel::UInt32*) (kHandoverHeader->f_GOP.f_The +                             \
                                     4 * kHandoverHeader->f_GOP.f_PixelPerLine * i + 4 * u))) = \
          fb_get_clear_clr();                                                                   \
    }                                                                                           \
  }

/// @brief Draws inside a zone.
#define FBDrawInRegion(clr, height, width, base_x, base_y)                                \
  for (Ne::Kernel::UInt32 x_base = base_x; x_base < (width + base_x); ++x_base) {             \
    for (Ne::Kernel::UInt32 y_base = base_y; y_base < (height + base_y); ++y_base) {          \
      *(((volatile Ne::Kernel::UInt32*) (kHandoverHeader->f_GOP.f_The +                       \
                                     4 * kHandoverHeader->f_GOP.f_PixelPerLine * x_base + \
                                     4 * y_base))) = clr;                                 \
    }                                                                                     \
  }

/// @brief Draws inside a zone.
#define FBDrawInRegionToRgn(_Rgn, clr, height, width, base_x, base_y)                            \
  for (Ne::Kernel::UInt32 x_base = base_x; x_base < (width + base_x); ++x_base) {                    \
    for (Ne::Kernel::UInt32 y_base = base_y; y_base < (height + base_y); ++y_base) {                 \
      *(((volatile Ne::Kernel::UInt32*) (_Rgn + 4 * kHandoverHeader->f_GOP.f_PixelPerLine * x_base + \
                                     4 * y_base))) = clr[kCGCursor];                             \
      ++kCGCursor;                                                                               \
    }                                                                                            \
  }

#define FBDrawInRegionA(clr, height, width, base_x, base_y)                               \
  for (Ne::Kernel::UInt32 x_base = base_x; x_base < (width + base_x); ++x_base) {             \
    for (Ne::Kernel::UInt32 y_base = base_y; y_base < (height + base_y); ++y_base) {          \
      *(((volatile Ne::Kernel::UInt32*) (kHandoverHeader->f_GOP.f_The +                       \
                                     4 * kHandoverHeader->f_GOP.f_PixelPerLine * x_base + \
                                     4 * y_base))) |= clr;                                \
    }                                                                                     \
  }
#else
#define FBDrawBitMapInRegionA(reg_ptr, height, width, base_x, base_y)
#define FBDrawBitMapInRegion(reg_ptr, height, width, base_x, base_y)
#define FBDrawBitMapInRegionToRgn(_Rgn, reg_ptr, height, width, base_x, base_y)
#define FBClearRegion(height, width, base_x, base_y)
#define FBDrawInRegion(clr, height, width, base_x, base_y)
#define FBDrawInRegionToRgn(_Rgn, clr, height, width, base_x, base_y)
#define FBDrawInRegionA(clr, height, width, base_x, base_y)
#define FBDrawBitMapInRegionA(reg_ptr, height, width, base_x, base_y)
#define FBDrawBitMapInRegion(reg_ptr, height, width, base_x, base_y)
#define FBDrawBitMapInRegionToRgn(_Rgn, reg_ptr, height, width, base_x, base_y)
#define FBClearRegion(height, width, base_x, base_y)
#define FBDrawInRegion(clr, height, width, base_x, base_y)
#define FBDrawInRegionToRgn(_Rgn, clr, height, width, base_x, base_y)
#define FBDrawInRegionA(clr, height, width, base_x, base_y)
#endif  // __NE_AMD64__

#ifndef CORE_GFX_ACCESSIBILITY_H
#include <modules/CoreGfx/CoreAccess.h>
#endif  // ifndef CORE_GFX_ACCESSIBILITY_H

namespace FB {

inline Void fb_clear_video() {
  FBDrawInRegion(fb_get_clear_clr(), FB::CGAccessibilty::Height(), FB::CGAccessibilty::Width(), 0,
                 0);
}

}  // namespace FB

#endif
