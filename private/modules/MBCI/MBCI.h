// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifndef _INC_MODULE_MBCI_H_
#define _INC_MODULE_MBCI_H_

#include <NeKit/Config.h>
#include <hint/CompilerHint.h>
#include <modules/ACPI/ACPI.h>

/// @file MBCI.h
/// @brief Mini Bus Controller Interface.

/**
- VCC (IN) (OUT for MCU)
- CLK (IN) (OUT for MCU)
- ACK (BI) (Contains an Acknowledge Packet Frame)
- D0- (IN) (Starts with the Host Interface Packet Frame)
- D1- (IN) (Starts with the Host Interface Packet Frame)
- D0+ (OUT) (Starts with the Host Interface Packet Frame)
- D1+ (OUT) (Starts with the Host Interface Packet Frame)
- GND (IN) (OUT for MCU)
 */

#define kMBCIZeroSz (8)
#define kMBCIESBSz (64)

namespace Ne::Kernel {
  
struct IMBCIHost;

enum {
  kMBCISpeedDeviceInvalid,
  kMBCILowSpeedDevice,
  kMBCIHighSpeedDevice,
  kMBCISpeedDeviceCount,
};

/// @brief MBCI Host header.
volatile struct PACKED IMBCIHost final {
  UInt32 Magic;
  UInt32 HostId;
  UInt16 VendorId;
  UInt16 DeviceId;
  UInt8  MemoryType;
  UInt16 HostType;
  UInt16 HostFlags;
  UInt8  Error;
  UInt32 MMIOTest;
  UInt16 State;
  UInt8  Status;
  UInt8  InterruptEnable;
  UInt64 BaseAddressRegister;
  UInt64 BaseAddressRegisterSize;
  UInt32 CommandIssue;
  UInt8  Esb[kMBCIESBSz];  // Extended Signature Block
  UInt8  Zero[kMBCIZeroSz];
};

/// @brief MBCI host flags.
enum MBCIHostFlags {
  kMBCIHostFlagsSupportsNothing,       // Invalid MBCI device.
  kMBCIHostFlagsSupportsAPM,           // FW's Advanced Power Management.
  kMBCIHostFlagsSupportsDaisyChain,    // Is daisy chained.
  kMBCIHostFlagsSupportsHWInterrupts,  // Has HW interrupts.
  kMBCIHostFlagsSupportsDMA,           // Has DMA.
  kMBCIHostFlagsExtended,              // Extended flags table.
};

/// @brief MBCI host kind.
enum MBCIHostKind {
  kMBCIHostKindHardDisk,
  kMBCIHostKindOpticalDisk,
  kMBCIHostKindKeyboardLow,
  kMBCIHostKindMouseLow,
  kMBCIHostKindMouseHigh,
  kMBCIHostKindKeyboardHigh,
  kMBCIHostKindNetworkInterface,
  kMBCIHostKindDaisyChain,
  kMBCIHostKindStartExtended,  // Extended vendor table limit.
};

enum MBCIHostState {
  kMBCIHostStateInvalid,
  kMBCIHostStateReset,
  kMBCIHostStateSuccess,
  kMBCIHostStateReady,
  kMBCIHostStateDmaStart,
  kMBCIHostStateDmaEnd,
  kMBCIHostStateFail,
  kMBCIHostStateCount,
};

/// @brief An AuthKey is a context used to tokenize data for an MBCI packet.
typedef UInt32 MBCIAuthKeyType;

/// @internal
inline BOOL busi_test_mmio(_Input struct IMBCIHost* host, _Input const UInt32 test) {
  host->MMIOTest = test;
  UInt16 timeout{0UL};

  while (host->MMIOTest == test) {
    ++timeout;

    if (timeout > 0x1000) return NO;
  }

  return host->MMIOTest == 0;
}

/// @brief Read Auth key for MBCI host.
/// @param host the mbci host to get the key on.
/// @return the 24-bit key.
inline MBCIAuthKeyType mbci_read_auth_key(_Input struct IMBCIHost* host) {
  auto const kChallengeMBCI = 0x1;  // MBCI Challenge test

  if (!busi_test_mmio(host, kChallengeMBCI)) return ~0;

  return (host->Esb[kMBCIESBSz - 1] << 16) | (host->Esb[kMBCIESBSz - 2] << 8) |
         (host->Esb[kMBCIESBSz - 3] & 0xFF);
}

}  // namespace Ne::Kernel

#endif  // ifndef _INC_MODULE_MBCI_H_

// END
