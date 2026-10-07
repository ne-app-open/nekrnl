// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifndef COREGFX_MATHGFX_H
#define COREGFX_MATHGFX_H

/// @file MathGfx.h
/// @brief Math module implementation for CoreGfx.

namespace UI {

#ifdef NE_CORE_GFX_USE_DOUBLE
typedef double fb_real_t;
#else
typedef float fb_real_t;
#endif

/// @brief Linear interpolation equation solver.
/// @param from where to start
/// @param to to which value.
/// @param stat
/// @return Linear interop value.
inline fb_real_t fb_math_lerp(const fb_real_t& to, const fb_real_t& from, const fb_real_t& stat) {
  if (stat == 0) return {};
  return (from) + (to - from) * stat;
}

}  // namespace UI

#endif
