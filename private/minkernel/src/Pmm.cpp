// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <ArchKit/ArchKit.h>
#include <KernelKit/DebugOutput.h>
#include <NeKit/Pmm.h>

namespace Ne::Kernel {

  /***********************************************************************************/
/// @brief Pmm constructor.
/***********************************************************************************/
Pmm::Pmm() : fPageMgr() {
  kout << "[PMM] Allocate PageMemoryMgr.\r";
}

Pmm::~Pmm() = default;

/***********************************************************************************/
/// @param If this returns Null pointer, enter emergency mode.
/// @param user is this a user page?
/// @param readWrite is it r/w?
/***********************************************************************************/
Ref<PTEWrapper> Pmm::RequestPage(Boolean user, Boolean readWrite) {
  PTEWrapper pt = fPageMgr.Leak().Request(user, readWrite, false, kPageSize, 0);

  if (pt.fPresent) {
    kout << "[PMM]: Allocation failed.\r";
    return {pt};
  }

  return Ref<PTEWrapper>(pt);
}

Boolean Pmm::FreePage(Ref<PTEWrapper> PageRef) {
  if (!PageRef) return false;

  PageRef.Leak().fPresent = false;

  return true;
}

Boolean Pmm::TogglePresent(Ref<PTEWrapper> PageRef, Boolean Enable) {
  if (!PageRef) return false;

  PageRef.Leak().fPresent = Enable;

  return true;
}

Boolean Pmm::ToggleUser(Ref<PTEWrapper> PageRef, Boolean Enable) {
  if (!PageRef) return false;

  PageRef.Leak().fRw = Enable;

  return true;
}

Boolean Pmm::ToggleRw(Ref<PTEWrapper> PageRef, Boolean Enable) {
  if (!PageRef) return false;

  PageRef.Leak().fRw = Enable;

  return true;
}

Boolean Pmm::ToggleShare(Ref<PTEWrapper> PageRef, Boolean Enable) {
  if (!PageRef) return false;

  PageRef.Leak().fShareable = Enable;
  PageRef.Leak().fExecDisable  = (Enable) ? YES : NO;

  return true;
}

}  // namespace Ne::Kernel
