// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <ArchKit/ArchKit.h>
#include <HALKit/AMD64/Processor.h>
#include <KernelKit/BinaryMutex.h>
#include <KernelKit/HardwareThreadScheduler.h>
#include <KernelKit/ProcessScheduler.h>
#include <KernelKit/Timer.h>
#include <NeKit/KernelPanic.h>
#include <modules/ACPI/ACPIFactoryInterface.h>
#include <modules/CoreGfx/TextGfx.h>

#define APIC_MAG "APIC"

#define APIC_ICR_LOW 0x300
#define APIC_ICR_HIGH 0x310
#define APIC_SIPI_VEC 0x00500
#define APIC_EIPI_VEC 0x00400

#define LAPIC_REG_TIMER_LVT 0x320
#define LAPIC_REG_TIMER_INITCNT 0x380
#define LAPIC_REG_TIMER_CURRCNT 0x390
#define LAPIC_REG_TIMER_DIV 0x3E0
#define LAPIC_REG_ENABLE 0x80
#define LAPIC_REG_SPURIOUS 0xF0

#define APIC_BASE_MSR 0x1B
#define APIC_BASE_MSR_BSP 0x100
#define APIC_BASE_MSR_ENABLE 0x800

///////////////////////////////////////////////////////////////////////////////////////

/// @note: _hal_switch_context is internal.
/// @brief The **HAL** namespace.

///////////////////////////////////////////////////////////////////////////////////////

namespace Ne::Kernel::HAL {
struct HAL_APIC_MADT;
struct HAL_HARDWARE_THREAD;

struct HAL_HARDWARE_THREAD final {
  StackFramePtr mFramePtr;
  ProcessID     mThreadID{0};
};

EXTERN_C Void sched_jump_to_task(StackFramePtr stack_frame);

STATIC HAL_APIC_MADT* kSMPBlock = nullptr;
STATIC Bool           kSMPAware = false;
STATIC Int64          kSMPCount = 0;

EXTERN_C UIntPtr kApicBaseAddress;

STATIC Int32   kSMPInterrupt                   = 0;
STATIC UInt64  kAPICLocales[kMaxAPInsideSched] = {0};
STATIC VoidPtr kRawMADT                        = nullptr;

STATIC HAL_HARDWARE_THREAD kHWThread[kCPSProcessLimitPerTeam] = {{}};

/// @brief Multiple APIC Descriptor Table.
struct HAL_APIC_MADT final SDT_OBJECT {
  UInt32 Address;  // Madt address
  UInt32 Flags;    // Madt flags
  UInt8  List[1];  // Records List
};

/// @brief Local APIC Descriptor Table.
struct LAPIC final {
  UInt8  Type;
  UInt8  Length;
  UInt8  ProcessorID;
  UInt8  APICID;
  UInt32 Flags;
};

///////////////////////////////////////////////////////////////////////////////////////

/***********************************************************************************/
/// @brief Send end IPI for CPU.
/// @param apic_id
/// @param vector
/// @param target
/// @return
/***********************************************************************************/
EXTERN_C Void hal_send_ipi_msg(UInt32 target, UInt32 apic_id, UInt8 vector) {
  if (!apic_id || vector) return;
  if (!target) return;

  ke_dma_write<UInt32>(target, APIC_ICR_HIGH, apic_id << 24);
  
  const auto vec_val = 0x00000600 | 0x00004000 | 0x00000000 | vector;
  MUST_PASS(vec_val > 0);

  ke_dma_write<UInt32>(target, APIC_ICR_LOW, vec_val);

  while (Ne::Kernel::ke_dma_read<UInt32>(target, APIC_ICR_LOW) & 0x1000) {
    NE_UNUSED(0);
  }
}

/***********************************************************************************/
/// @brief Get current stack frame for a thread.
/// @param thrdid The thread ID.
/***********************************************************************************/

EXTERN_C HAL::StackFramePtr mp_get_current_task(ThreadID thrdid) {
  if (thrdid > kSMPCount) return nullptr;
  return kHWThread[thrdid].mFramePtr;
}

/***********************************************************************************/
/// @brief Register current stack frame for a thread.
/// @param stack_frame The current stack frame.
/// @param thrdid The thread ID.
/***********************************************************************************/

EXTERN_C BOOL mp_register_task(HAL::StackFramePtr stack_frame, ThreadID thrdid) {
  if (!stack_frame) return NO;

  if (!kSMPAware) {
    sched_jump_to_task(stack_frame);
    return YES;
  }

  if (thrdid > kSMPCount) return NO;

  kHWThread[thrdid].mThreadID = thrdid;
  kHWThread[thrdid].mFramePtr = stack_frame;

  hal_send_ipi_msg(thrdid, kAPICLocales[thrdid], kSMPInterrupt + 32);

  return YES;
}

/***********************************************************************************/
/// @brief Is the current config SMP aware?
/// @return True if YES, False if not.
/***********************************************************************************/

Bool mp_is_smp(Void) {
  return kSMPAware;
}

/***********************************************************************************/
/// @brief Fetch and enable SMP scheduler.
/// @param vendor_ptr SMP containing structure.
/***********************************************************************************/

Void mp_init_cores(VoidPtr vendor_ptr) {
  if (!vendor_ptr) return;

  PowerFactoryInterface hw_and_pow_int{vendor_ptr};

  auto pwr = hw_and_pow_int.Find(APIC_MAG);

  if (pwr.HasError()) {
    kSMPAware = NO;
    return;
  }

  kRawMADT  = pwr.Leak().Leak();
  kSMPBlock = reinterpret_cast<HAL_APIC_MADT*>(kRawMADT);
  kSMPAware = NO;

  if (kSMPBlock) {
    kSMPInterrupt = 0;
    kSMPCount     = 0;

    UInt32 lo = 0U, hi = 0U;

    hal_get_msr(APIC_BASE_MSR, &lo, &hi);

    UInt64 apic_base = ((UInt64) hi << 32) | lo;

    apic_base |= APIC_BASE_MSR_ENABLE;  // Enable APIC.

    lo = apic_base & 0xFFFFFFFF;
    hi = apic_base >> 32;

    hal_set_msr(APIC_BASE_MSR, lo, hi);

    kApicBaseAddress = apic_base & 0xFFFFF000;

    LAPICDmaWrapper controller{(VoidPtr) kApicBaseAddress};

    controller.Write(LAPIC_REG_ENABLE, 0);
    controller.Write(LAPIC_REG_SPURIOUS, 0x1FF);  // Enable bit, spurious interrupt vector register.
    controller.Write(LAPIC_REG_TIMER_DIV, 0b0011);
    controller.Write(LAPIC_REG_TIMER_LVT, 0x20 | (1 << 17));
    controller.Write(LAPIC_REG_TIMER_INITCNT, 1000000);

    volatile UInt8* entry_ptr = reinterpret_cast<volatile UInt8*>(kSMPBlock->List);
    volatile UInt8* end_ptr   = ((UInt8*) kSMPBlock) + kSMPBlock->Length;

    while (entry_ptr < end_ptr) {
      UInt8 type   = *entry_ptr;
      UInt8 length = *(entry_ptr + 1);

      if (type == 0 && length == sizeof(struct LAPIC)) {
        volatile LAPIC* entry_struct = (volatile LAPIC*) entry_ptr;

        if (entry_struct->Flags & 0x1) {
          kAPICLocales[kSMPCount]        = entry_struct->ProcessorID;
          kHWThread[kSMPCount].mThreadID = kAPICLocales[kSMPCount];

          ++kSMPCount;
        }
      }

      entry_ptr += length;
    }
  }
}
}  // namespace Ne::Kernel::HAL

///////////////////////////////////////////////////////////////////////////////////////

// EOF
