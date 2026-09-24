// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <KernelKit/FileMgr.h>
#include <KernelKit/KPC.h>
#include <KernelKit/ProcessScheduler.h>
#include <NetworkKit/IPC.h>
#include <atomic>

namespace Ne::Kernel {

bool IPC_ADDR::operator==(const IPC_ADDR& addr) {
  return addr.UserProcessID == this->UserProcessID && addr.UserProcessTeam == this->UserProcessTeam;
}

bool IPC_ADDR::operator==(IPC_ADDR& addr) {
  return addr.UserProcessID == this->UserProcessID && addr.UserProcessTeam == this->UserProcessTeam;
}

bool IPC_ADDR::operator!=(const IPC_ADDR& addr) {
  return addr.UserProcessID != this->UserProcessID || addr.UserProcessTeam != this->UserProcessTeam;
}

bool IPC_ADDR::operator!=(IPC_ADDR& addr) {
  return addr.UserProcessID != this->UserProcessID || addr.UserProcessTeam != this->UserProcessTeam;
}

/// @brief The IPC I/O funct.

#ifndef kIPCTagName
#define kIPCTagName "IpcKrnlTxRxFn_"
#endif

static const constexpr auto kMIBMaxIpcMsg = mib_cast(512);

Ref<Int64> ipc_write_to_file(FileStreamDefault& fs, ErrorOrAny dat, SizeT& len) {
  std::atomic_flag flg = ATOMIC_FLAG_INIT;

  while (!flg.test_and_set(std::memory_order_acquire));

  if (fs.Leak() && dat && len) {
    MUST_PASS(len < kMIBMaxIpcMsg);

    flg.clear(std::memory_order_release);
    return fs.Write(kIPCTagName "__impOutboundMsg", dat.Leak().Leak(), len);
  }

  flg.clear(std::memory_order_release);
  /// @brief Check if canary has errors. In other words a bug check.
  MUST_PASS(fs.Read(kIPCTagName "__impCanary", 1).HasError() != YES);

  return Ref<Int64>(0);
}

ErrorOrAny ipc_read_from_file(FileStreamDefault& fs, SizeT& len) {
  std::atomic_flag flg = ATOMIC_FLAG_INIT;

  while (!flg.test_and_set(std::memory_order_acquire));

  if (fs.Leak() && len) {
    MUST_PASS(len < kMIBMaxIpcMsg);

    flg.clear(std::memory_order_release);
    return fs.Read(kIPCTagName "__impInboundMsg", len);
  }

  flg.clear(std::memory_order_release);
  /// @brief Check if canary has errors. In other words a bug check.
  MUST_PASS(fs.Read(kIPCTagName "__impCanary", 1).HasError() != YES);

  return ErrorOrAny(kErrorInvalidData);
}

}  // namespace Ne::Kernel
