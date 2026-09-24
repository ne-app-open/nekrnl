// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <KernelKit/KPC.h>
#include <KernelKit/ProcessScheduler.h>
#include <NetworkKit/IPC.h>

namespace Ne::Kernel {
  
/***********************************************************************************/
/// @internal internal use for IPC system only.
/// @brief The internal sanitize function.
/***********************************************************************************/
Bool ipc_int_sanitize_packet(IPC_MSG* pckt) {
  if (!pckt) return NO;

  auto endian = RTL_ENDIAN(pckt, ((Char*) pckt)[0]);

  switch (endian) {
    case Endian::kEndianBig: {
      if (pckt->IpcEndianess == kIPCLittleEndian) goto ipc_check_failed;

      break;
    }
    case Endian::kEndianLittle: {
      if (pckt->IpcEndianess == kIPCBigEndian) goto ipc_check_failed;

      break;
    }
    case Endian::kEndianMixed: {
      if (pckt->IpcEndianess == kIPCMixedEndian) goto ipc_check_failed;

      break;
    }
    default:
      goto ipc_check_failed;
  }

  if (pckt->IpcFrom == pckt->IpcTo || pckt->IpcPacketSize < 1) {
    goto ipc_check_failed;
  }

  return pckt->IpcPacketSize > 1 && pckt->IpcHeaderMagic == kIPCHeaderMagic;

ipc_check_failed:
  err_local_get() = kErrorIPC;
  return NO;
}

/***********************************************************************************/
/// @brief Sanitize packet function
/// @retval true packet is correct.
/// @retval false packet is incorrect and process has crashed.
/***********************************************************************************/
Bool ipc_sanitize_packet(IPC_MSG* pckt) {
  if (!pckt || !ipc_int_sanitize_packet(pckt)) {
    return NO;
  }

  return YES;
}

/***********************************************************************************/
/// @brief Construct packet function
/// @retval true packet is correct.
/// @retval false packet is incorrect and process has crashed.
/***********************************************************************************/
Bool ipc_construct_packet(_Output IPC_MSG** pckt_in) {
  // don't act if it's not even valid.
  if (!pckt_in) return false;

  if (!*pckt_in) *pckt_in = new IPC_MSG();

  MUST_PASS(*pckt_in);

  if (*pckt_in) {
    const auto endianess = RTL_ENDIAN((*pckt_in), ((Char*) (*pckt_in))[0]);

    (*pckt_in)->IpcHeaderMagic = kIPCHeaderMagic;

    (*pckt_in)->IpcEndianess  = static_cast<UInt8>(endianess);
    (*pckt_in)->IpcPacketSize = sizeof(IPC_MSG);

    (*pckt_in)->IpcTo.UserProcessID   = 0;
    (*pckt_in)->IpcTo.UserProcessTeam = 0;

    (*pckt_in)->IpcFrom.UserProcessID   = 0;
    (*pckt_in)->IpcFrom.UserProcessTeam = 0;

    (*pckt_in)->IpcLock = kIPCLockFree;

    return Yes;
  }

  return No;
}

/***********************************************************************************/
/// @brief Pass message from **src** to **target**
/// @param src Source message.
/// @param target Target message.
/***********************************************************************************/
Bool IPC_MSG::Pass(IPC_MSG* src, IPC_MSG* target) {
  if (src && target && (target != src)) {
    if (target->IpcSize > src->IpcSize) return No;

    auto timeout = 0U;

    constexpr auto kLimitTimeout = 1000000U;

    while ((target->IpcLock % kIPCLockUsed) != 0) {
      if (timeout > kLimitTimeout) {
        return No;
      }
    }

    ++target->IpcLock;

    rt_copy_memory_safe(src->IpcData, target->IpcData, src->IpcSize, target->IpcSize);

    --target->IpcLock;

    return Yes;
  }

  return No;
}

}  // namespace Ne::Kernel
