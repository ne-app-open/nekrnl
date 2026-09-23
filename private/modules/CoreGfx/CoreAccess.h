/* ========================================

  Copyright Amlal El Mahrouss.

======================================== */

#ifndef CORE_GFX_ACCESSIBILITY_H
#define CORE_GFX_ACCESSIBILITY_H

#include <ArchKit/ArchKit.h>
#include <KernelKit/KPC.h>
#include <NeKit/NeKit.h>
#include <modules/CoreGfx/CoreGfx.h>
#include <modules/CoreGfx/MathGfx.h>

namespace FB {
using namespace Ne::Kernel;

template <SizeT N, typename NType>
struct CGVec;
class CGAccessibility;

/// @brief Video Accessbility Helper.
/// Use this when dealing with video diemensions.
class CGAccessibilty final {
  explicit CGAccessibilty() = default;
  ~CGAccessibilty()         = default;

 public:
  NE_COPY_DELETE(CGAccessibilty)

  static UInt64 Width() { return kHandoverHeader->f_GOP.f_Width; }

  static UInt64 Height() { return kHandoverHeader->f_GOP.f_Height; }
};

template <SizeT N, typename NType>
struct CGVec final {
  NType fVec[N]    = {};
  explicit CGVec() = default;
  ~CGVec()         = default;
  NE_COPY_DEFAULT(CGVec)

  NType& operator[](const SizeT& i) {
    MUST_PASS(i < N);
    return fVec[i];
  }
};

using CGVec3U64 = CGVec<3, UInt64>;
using CGVec2U64 = CGVec<2, UInt64>;
using CGVec4U64 = CGVec<4, UInt64>;
}  // namespace FB

#endif  // !CORE_GFX_ACCESSIBILITY_H_
