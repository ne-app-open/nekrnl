// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <HALKit/ARM64/ApplicationProcessor.h>
#include <HALKit/ARM64/Processor.h>
#include <KernelKit/DebugOutput.h>
#include <KernelKit/HardwareThreadScheduler.h>
#include <KernelKit/ProcessScheduler.h>
#include <KernelKit/Timer.h>

#define GICD_BASE 0x08000000
#define GICC_BASE 0x08010000

#define GICD_CTLR 0x000
#define GICD_ISENABLER 0x100
#define GICD_ICENABLER 0x180
#define GICD_ISPENDR 0x200
#define GICD_ICPENDR 0x280
#define GICD_IPRIORITYR 0x400
#define GICD_ITARGETSR 0x800
#define GICD_ICFGR 0xC00

#define GICC_CTLR 0x000
#define GICC_PMR 0x004
#define GICC_IAR 0x00C
#define GICC_EOIR 0x010

// ================================================================= //

namespace Ne::Kernel {
  
struct HAL_HARDWARE_THREAD final {
  HAL::StackFramePtr mFramePtr;
  ProcessID          mThreadID{0};
};

STATIC HAL_HARDWARE_THREAD kHWThread[kMaxAPInsideSched] = {{nullptr}};

namespace Detail {
  STATIC BOOL kGICEnabled = NO;

  /***********************************************************************************/
  /// @brief Enables the GIC with EL0 configuration.
  /// @internal
  /***********************************************************************************/
  STATIC Void mp_setup_gic_el0(Void) {
    ke_dma_write<UInt32>(GICD_BASE, GICD_CTLR, YES);

    UInt32 gicc_ctlr = ke_dma_read<UInt32>(GICC_BASE, GICC_CTLR);

    const UInt8 kEnableSignalInt = 0x1;

    gicc_ctlr |= kEnableSignalInt;
    gicc_ctlr |= (kEnableSignalInt << 0x1);

    ke_dma_write<UInt32>(GICC_BASE, GICC_CTLR, gicc_ctlr);

    ke_dma_write<UInt32>(GICC_BASE, GICC_PMR, 0xFF);

    UInt32 icfgr = ke_dma_read<UInt32>(GICD_BASE, GICD_ICFGR + (0x20 / 0x10) * 4);

    icfgr |= (0x2 << ((32 % 16) * 2));

    ke_dma_write<UInt32>(GICD_BASE, GICD_ICFGR + (0x20 / 0x10) * 4, icfgr);
    ke_dma_write<UInt32>(GICD_BASE, GICD_ITARGETSR + (0x20 / 0x04) * 4, 0x2 << ((32 % 4) * 8));
    ke_dma_write<UInt32>(GICD_BASE, GICD_IPRIORITYR + (0x20 / 0x04) * 4, 0xFF << ((32 % 4) * 8));
    ke_dma_write<UInt32>(GICD_BASE, GICD_ISENABLER + 4, 0x01);
  }

  EXTERN_C BOOL mp_handle_gic_interrupt_el0(Void) {
    UInt32 interrupt_id = ke_dma_read<UInt32>(GICC_BASE, GICC_IAR);

    if ((interrupt_id & 0x3FF) < 1020) {
      auto interrupt = interrupt_id & 0x3FF;

      const UInt16 kInterruptScheduler = 0x20;

      (Void)(kout << "SMP: AP: " << hex_number(interrupt) << kendl);

      switch (interrupt) {
        case kInterruptScheduler: {
          ke_dma_write<UInt32>(GICC_BASE, GICC_EOIR, interrupt_id);
          UserProcessHelper::StartScheduling();
          break;
        }
        default: {
          ke_dma_write<UInt32>(GICC_BASE, GICC_EOIR, interrupt_id);
          break;
        }
      }

      return YES;
    }

    return NO;
  }
}  // namespace Detail

/***********************************************************************************/
/// @brief Get current stack frame for a thread.
/// @param thrdid The thread ID.
/***********************************************************************************/

EXTERN_C HAL::StackFramePtr mp_get_current_task(ProcessID thrdid) {
  return kHWThread[thrdid].mFramePtr;
}

/***********************************************************************************/
/// @brief Register current stack frame for a thread.
/// @param stack_frame The current stack frame.
/// @param thrdid The thread ID.
/***********************************************************************************/

EXTERN_C Bool mp_register_task(HAL::StackFramePtr stack_frame, ProcessID thrdid) {
  MUST_PASS(Detail::kGICEnabled);

  if (!stack_frame) return NO;
  if (thrdid > kMaxAPInsideSched) return NO;

  const auto process_index = thrdid;

  kHWThread[process_index].mFramePtr = stack_frame;
  kHWThread[process_index].mThreadID = thrdid;

  return YES;
}

/***********************************************************************************/
/// @brief Initialize the Global Interrupt Controller.
/// @internal
/***********************************************************************************/
Void mp_init_cores(Void) {
  if (!Detail::kGICEnabled) {
    Detail::kGICEnabled = YES;
    Detail::mp_setup_gic_el0();
  }
}

}  // namespace Ne::Kernel