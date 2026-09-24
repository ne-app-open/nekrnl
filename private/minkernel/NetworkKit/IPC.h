// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifndef NETWORKKIT_IPC_H
#define NETWORKKIT_IPC_H

#include <NeKit/Config.h>
#include <NeKit/KString.h>
#include <hint/CompilerHint.h>

/// @file IPC.h
/// @brief IPC comm. protocol.

/// IA separator.
#define kIPCRemoteSeparator ":"

/// Interchange address, consists of ProcessID:TEAM.
#define kIPCRemoteInvalid "00:00"

#define kIPCHeaderMagic (0x4950434)

/// @brief Now the IPC I/O
#ifndef kIPCBusFileExt
#define kIPCBusFileExt ".swap"
#endif

namespace Ne::Kernel {
struct IPC_ADDR;
struct IPC_MSG;

/// @brief 128-bit IPC address.
struct PACKED IPC_ADDR final {
  UInt64 UserProcessID;
  UInt64 UserProcessTeam;

  ////////////////////////////////////
  // some operators.
  ////////////////////////////////////

  BOOL operator==(const IPC_ADDR& addr);
  BOOL operator==(IPC_ADDR& addr);
  BOOL operator!=(const IPC_ADDR& addr);
  BOOL operator!=(IPC_ADDR& addr);
};

typedef struct IPC_ADDR IPC_ADDR;

enum {
  kIPCNoEndian = 0,
  kIPCLittleEndian = 10,
  kIPCBigEndian    = 11,
  kIPCMixedEndian  = 12,
};

enum {
  kIPCLockInvalid = 0,
  kIPCLockFree    = 1,
  kIPCLockUsed    = 2,
};

typedef struct IPC_MSG final {
  UInt32   IpcHeaderMagic;  // cRemoteHeaderMagic
  UInt8    IpcEndianess;    // 0 : LE, 1 : BE
  SizeT    IpcPacketSize;
  IPC_ADDR IpcFrom;
  IPC_ADDR IpcTo;
  UInt32   IpcCRC32;
  UInt32   IpcMsg;
  UInt32   IpcMsgSz;
  UInt32   IpcLock;
  SizeT    IpcSize;
  UInt8*   IpcData;
  /// @brief Passes the message to target, could be anything, HTTP packet, JSON or whatever.
  static Bool Pass(IPC_MSG* self, IPC_MSG* target);
} PACKED ALIGN(8) IPC_MSG;

/// @brief Sanitize packet function
/// @retval true packet is correct.
/// @retval false packet is incorrect and process has crashed.
BOOL ipc_sanitize_packet(_Input IPC_MSG* pckt_in);

/// @brief Construct packet function
/// @retval true packet is correct.
/// @retval false packet is incorrect and process has crashed.
BOOL ipc_construct_packet(_Output _Input IPC_MSG** pckt_in);
}  // namespace Ne::Kernel

#endif  // NETWORKKIT_IPC_H
