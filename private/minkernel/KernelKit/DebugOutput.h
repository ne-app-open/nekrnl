// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifndef KERNELKIT_DEBUGOUTPUT_H
#define KERNELKIT_DEBUGOUTPUT_H

#include <CompilerKit/CompilerKit.h>
#include <KernelKit/DeviceMgr.h>
#include <NeKit/OwnPtr.h>
#include <NeKit/Stream.h>
#include <NeKit/Utils.h>

namespace Ne::Kernel {

// Devices forward declarations.
class TerminalDevice;
class DTraceDevice;
class NeTraceDevice;
class Utf8TerminalDevice;

inline TerminalDevice end_line();
inline TerminalDevice number(const Long& x);
inline TerminalDevice hex_number(const Long& x);

// @brief Emulates a VT100 terminal.
class TerminalDevice final NE_DEVICE<const Char*> {
 public:
  TerminalDevice(void (*print)(IDevice*, const Char*), void (*gets)(IDevice*, const Char*))
      : IDevice<const Char*>(print, gets) {}

  ~TerminalDevice() override;

  /// @brief returns device name (terminal name)
  /// @return string type (const Char*)
  const Char* Name() const override { return ("TerminalDevice"); }

  NE_COPY_DEFAULT(TerminalDevice)

  STATIC TerminalDevice The();
};

class Utf8TerminalDevice final NE_DEVICE<const Utf8Char*> {
 public:
  Utf8TerminalDevice(void (*print)(IDevice*, const Utf8Char*),
                     void (*gets)(IDevice*, const Utf8Char*))
      : IDevice<const Utf8Char*>(print, gets) {}

  ~Utf8TerminalDevice() override;

  /// @brief returns device name (terminal name)
  /// @return string type (const Char*)
  const Char* Name() const override { return ("Utf8TerminalDevice"); }

  NE_COPY_DEFAULT(Utf8TerminalDevice)

  STATIC Utf8TerminalDevice The();
};

inline TerminalDevice end_line() {
  TerminalDevice self = TerminalDevice::The();

  self.operator<<("\r");
  return self;
}

inline Utf8TerminalDevice utf_end_line() {
  Utf8TerminalDevice self = Utf8TerminalDevice::The();

  self.operator<<(u8"\r");
  return self;
}

inline TerminalDevice carriage_return() {
  TerminalDevice self = TerminalDevice::The();

  self.operator<<("\r");
  return self;
}

inline TerminalDevice tabulate() {
  TerminalDevice self = TerminalDevice::The();

  self.operator<<("\t");
  return self;
}

/// @brief emulate a terminal bell, like the VT100 does.
inline TerminalDevice bell() {
  TerminalDevice self = TerminalDevice::The();

  self.operator<<("\a");
  return self;
}

namespace Detail {
  inline TerminalDevice _write_number(const Long& x, TerminalDevice& term) {
    UInt64 y = (x > 0 ? x : -x) / 10;
    UInt64 h = (x > 0 ? x : -x) % 10;

    if (y) _write_number(y, term);

    /* fail if the number is not base-10 */
    if (h > 10) {
      _write_number('?', term);
      return term;
    }

    if (y == ~0UL) y = -y;

    const Char kNumbers[11] = "0123456789";

    Char buf[2];
    buf[0] = kNumbers[h];
    buf[1] = 0;

    term.operator<<(buf);
    return term;
  }

  inline TerminalDevice _write_number_hex(const Long& x, TerminalDevice& term) {
    UInt64 y = (x > 0 ? x : -x) / 16;
    UInt64 h = (x > 0 ? x : -x) % 16;

    if (y) _write_number_hex(y, term);

    /* fail if the hex number is not base-16 */
    if (h > 16) {
      _write_number_hex('?', term);
      return term;
    }

    if (y == ~0UL) y = -y;

    const Char kNumbers[17] = "0123456789ABCDEF";

    Char buf[2];
    buf[0] = kNumbers[h];
    buf[1] = 0;

    term.operator<<(buf);
    return term;
  }
}  // namespace Detail

inline TerminalDevice hex_number(const Long& x) {
  TerminalDevice self = TerminalDevice::The();

  self << "0x";
  Detail::_write_number_hex(x, self);

  return self;
}

inline TerminalDevice number(const Long& x) {
  TerminalDevice self = TerminalDevice::The();

  Detail::_write_number(x, self);

  return self;
}

inline TerminalDevice get_console_in(Char* buf) {
  TerminalDevice self = TerminalDevice::The();

  self >> buf;
  return self;
}

inline constexpr auto kDebugPort    = 51820;
inline constexpr auto kDebugMagic   = "VMK1.0.0;";
inline constexpr auto kDebugVersion = 0x0100;

inline constexpr SizeT kDebugCmdLen = 256U;

typedef Char rt_debug_cmd[kDebugCmdLen];

inline TerminalDevice& operator<<(TerminalDevice& src, const Long& num) {
  src = number(num);
  return src;
}

}  // namespace Ne::Kernel

#ifdef kout
#undef kout
#endif  // ifdef kout

#define kout TerminalDevice::The() << "[DEBUG] " << __FILE__ << ": "

#ifdef kendl
#undef kendl
#endif  // ifdef kendl

#define kendl end_line()

#ifdef kout8
#undef kout8
#endif  // ifdef kout8

#define kout8 Utf8TerminalDevice::The() << "[DEBUG] " __FILE__ << ": "

#ifdef kendl8
#undef kendl8
#endif  // ifdef kendl8

#define kendl8 utf_end_line()

#endif
