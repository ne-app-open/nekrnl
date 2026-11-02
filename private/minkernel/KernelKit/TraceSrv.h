// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifndef KERNELKIT_TRACESRV_H
#define KERNELKIT_TRACESRV_H

#include <CompilerKit/CompilerKit.h>
#include <NeKit/NeKit.h>

namespace Ne::Kernel {

namespace TraceSrv {

  inline constexpr auto kDebugCmdLen  = 256U;
  inline constexpr auto kDebugPort    = 51820;
  inline constexpr auto kDebugMagic   = "NE1.0.1;";
  inline constexpr auto kDebugVersion = 0x0101;
  inline constexpr auto kDebugDelim   = ';';
  inline constexpr auto kDebugEnd     = '\r';

  enum class TRACE_SOURCE_OPCODE {
    kStop,
    kContinue,
    kStepIn,
    kStepOver,
    kStepOut,
    kSetBreakpoint,
    kRemoveBreakpoint,
    kGetStackTrace,
    kGetSourceMessage,
    kGetVariables,
    kEvaluateExpression,
    kUnknown
  };

  inline constexpr auto kDebugSymbolLen = 0x0100;

  struct TRACE_SOURCE_MESSAGE final {
    Int32  fType;
    Int32  fChksum;
    Char   fSymbol[kDebugSymbolLen];
    UInt32 fOffset;
    UInt32 fOpcode;
  };

  struct TRACE_STRING_MESSAGE final {
    Int32 fType;
    Int32 fChksum;
    Int32 fLevel;
    Int32 fLength;
    Char  fMessage[kDebugCmdLen];
  };

}  // namespace TraceSrv

}  // namespace Ne::Kernel

#endif  // !KERNELKIT_TRACESRV_H
