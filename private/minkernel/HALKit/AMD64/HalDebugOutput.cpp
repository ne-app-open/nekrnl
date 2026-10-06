// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <ArchKit/ArchKit.h>
#include <KernelKit/DebugOutput.h>
#include <NeKit/New.h>
#include <NeKit/Utils.h>
#include <modules/CoreGfx/CoreGfx.h>
#include <modules/CoreGfx/TextGfx.h>

namespace Ne::Kernel {
enum CommStatus : UInt16 {
  kStateInvalid  = 0x64,
  kStateReady    = 0xCF,
  kStateTransmit = 0xFC,
  kStateCnt      = 3
};

namespace Detail {
  constexpr ATTRIBUTE(unused) const UInt16 kPort = 0x3F8;
  STATIC ATTRIBUTE(unused) UInt16 kState         = kStateInvalid;

  /// @brief Init COM1.
  /// @return
  template <UInt16 PORT>
  bool hal_serial_init() {
    if (kState == kStateReady || kState == kStateTransmit) return true;

#ifdef __DEBUG__
    HAL::rt_out8(PORT + 1, 0x00);  // Disable all interrupts
    HAL::rt_out8(PORT + 3, 0x80);  // Enable DLAB (set baud rate divisor)
    HAL::rt_out8(PORT + 0, 0x03);  // Set divisor to 3 (lo byte) 38400 baud
    HAL::rt_out8(PORT + 1, 0x00);  //                  (hi byte)
    HAL::rt_out8(PORT + 3, 0x03);  // 8 bits, no parity, one stop bit
    HAL::rt_out8(PORT + 2, 0xC7);  // Enable FIFO, clear them, with 14-byte threshold
    HAL::rt_out8(PORT + 4, 0x0B);  // IRQs enabled, RTS/DSR set
    HAL::rt_out8(PORT + 4, 0x1E);  // Set in loopback mode, test the serial chip
    HAL::rt_out8(PORT + 0, 0xAE);  // Test serial chip (send byte 0xAE and check if
                                   // serial returns same byte)

    // Check if serial is faulty (i.e: not same byte as sent)
    if (HAL::rt_in8(PORT) != 0xAE) {
      return false;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    HAL::rt_out8(PORT + 4, 0x0F);
#endif

    kState = kStateReady;

    return true;
  }
}  // namespace Detail

TerminalDevice::~TerminalDevice() = default;

#ifdef __DEBUG__
STATIC SizeT kX = kFontSizeX, kY = kFontSizeY + 70;
#endif  // __DEBUG__

EXTERN_C void ke_utf_io_write(IDevice<const Utf8Char*>* obj, const Utf8Char* bytes) {
  NE_UNUSED(bytes);
  NE_UNUSED(obj);

#ifdef __DEBUG__
  Detail::hal_serial_init<Detail::kPort>();

  if (!bytes || Detail::kState != kStateReady) return;

  if (*bytes == 0) return;

  Detail::kState = kStateTransmit;

  SizeT index = 0;
  SizeT len   = 0;

  index = 0;
  len   = urt_string_len(bytes);

  Char tmp_str[2];

  while (index < len) {
    if (bytes[index] == '\r') HAL::rt_out8(Detail::kPort, '\r');

    HAL::rt_out8(Detail::kPort, bytes[index] == '\r' ? '\n' : bytes[index]);

    tmp_str[0] = (bytes[index] > 127) ? '?' : bytes[index];
    tmp_str[1] = 0;

    fb_render_string(tmp_str, kY, kX, RGB(0xFF, 0xFF, 0xFF));

    if (bytes[index] == '\r') {
      kY += kFontSizeY;
      kX = kFontSizeX;
    }

    kX += kFontSizeX;

    if (kX > kHandoverHeader->f_GOP.f_Width) {
      kX = kFontSizeX;
    }

    if (kY > kHandoverHeader->f_GOP.f_Height) {
      kY = kFontSizeY;

      FBDrawInRegion(fb_get_clear_clr(), FB::CGAccessibilty::Height(), FB::CGAccessibilty::Width(),
                     0, 0);
    }

    ++index;
  }

  Detail::kState = kStateReady;
#endif  // __DEBUG__
}

EXTERN_C void ke_io_write(IDevice<const Char*>* obj, const Char* bytes) {
  NE_UNUSED(bytes);
  NE_UNUSED(obj);

#ifdef __DEBUG__
  Detail::hal_serial_init<Detail::kPort>();

  if (!bytes || Detail::kState != kStateReady) return;

  if (*bytes == 0) return;

  Detail::kState = kStateTransmit;

  SizeT index = 0;
  SizeT len   = 0;

  index = 0;
  len   = rt_string_len(bytes);

  Char tmp_str[2];

  while (index < len) {
    if (bytes[index] == '\r') HAL::rt_out8(Detail::kPort, '\r');

    HAL::rt_out8(Detail::kPort, bytes[index] == '\r' ? '\n' : bytes[index]);

    tmp_str[0] = bytes[index];
    tmp_str[1] = 0;

    fb_render_string(tmp_str, kY, kX, RGB(0xFF, 0xFF, 0xFF));

    if (bytes[index] == '\r') {
      kY += kFontSizeY;
      kX = kFontSizeX;
    }

    kX += kFontSizeX;

    if (kX > kHandoverHeader->f_GOP.f_Width) {
      kX = kFontSizeX;
    }

    if (kY > kHandoverHeader->f_GOP.f_Height) {
      kY = kFontSizeY + 70;

      FBDrawInRegion(fb_get_clear_clr(), FB::CGAccessibilty::Height(), FB::CGAccessibilty::Width(),
                     0, 0);
    }

    ++index;
  }

  Detail::kState = kStateReady;
#endif  // __DEBUG__
}

EXTERN_C void ke_io_read(IDevice<const Char*>*, const Char* bytes) {
  NE_UNUSED(bytes);

#ifdef __DEBUG__
  Detail::hal_serial_init<Detail::kPort>();

  if (!bytes || Detail::kState != kStateReady) return;

  Detail::kState = kStateTransmit;

  SizeT index = 0;

  Char* in  = (Char*) bytes;
  auto  in_ex  = HAL::rt_in8(Detail::kPort);
  in[index] = in_ex;

  Detail::kState = kStateReady;
#endif  // __DEBUG__
}

TerminalDevice TerminalDevice::The() {
  TerminalDevice out(Ne::Kernel::ke_io_write, Ne::Kernel::ke_io_read);
  return out;
}

Utf8TerminalDevice::~Utf8TerminalDevice() = default;

Utf8TerminalDevice Utf8TerminalDevice::The() {
  Utf8TerminalDevice out(Ne::Kernel::ke_utf_io_write,
                         [](IDevice<const Utf8Char*>*, const Utf8Char*) -> Void {});
  return out;
}

}  // namespace Ne::Kernel
