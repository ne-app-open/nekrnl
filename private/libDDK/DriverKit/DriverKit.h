// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifndef DRIVERKIT_DDK_H
#define DRIVERKIT_DDK_H

#include <DriverKit/Defines.h>

struct DDK_STATUS_STRUCT;
struct DDK_OBJECT_MANIFEST;

typedef void* ptr_t;

typedef ptr_t addr_t;

typedef ptr_t vaddr_t;
typedef ptr_t paddr_t;

/// \brief Object handle manifest.
struct DDK_OBJECT_MANIFEST DDK_FINAL {
  char*   p_name;
  int32_t p_kind;
  ptr_t   p_object;
};

/// \brief DDK status ping structure.
struct DDK_STATUS_STRUCT DDK_FINAL {
  int32_t                     s_action_id;
  int32_t                     s_issuer_id;
  int32_t                     s_group_id;
  struct DDK_OBJECT_MANIFEST* s_object;
};

/// @brief Call Ne::Kernel procedure.
/// @param name the procedure name.
/// @param cnt number of elements in **dat**
/// @param dat data argument pointer.
/// @param sz sz of whole data argument pointer.
/// @return result of call
DDK_EXTERN void* ke_call_dispatch(const char* name, int32_t cnt, void* dat, size_t sz);

/// @brief add a system call.
/// @param slot system call slot id.
/// @param slotFn, syscall slot.
DDK_EXTERN void ke_set_syscall(const int32_t slot, void (*slotFn)(void* a0));

/// @brief Allocates an heap ptr.
/// @param sz size of the allocated struct/type.
/// @return the pointer allocated or **nil**.
DDK_EXTERN void* kalloc(size_t sz);

/// @brief Frees an heap ptr.
/// @param pointer kernel pointer to free.
DDK_EXTERN void kfree(void* the_ptr);

/// @brief Gets a Ne::Kernel object.
/// @param slot object id (can be 0)
/// @param name the property's name.
/// @return DDK_OBJECT_MANIFEST.
DDK_EXTERN struct DDK_OBJECT_MANIFEST* ke_get_obj(const int slot, const char* name);

/// @brief Set a Ne::Kernel object.
/// @param slot object id (can be 0)
/// @param name the property's name.
/// @param ddk_pr pointer to a object's DDK_OBJECT_MANIFEST.
/// @return returned object.
DDK_EXTERN void* ke_set_obj(const int32_t slot, const struct DDK_OBJECT_MANIFEST* ddk_pr);

#endif
