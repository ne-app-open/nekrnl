// SPDX-License-Identifier: Apache-2.0
// Copyright 2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-foss/ne-kernel

#include <MsgKit/Server.h>
#include <SystemKit/Err.h>

static libmsg_func_type* kFuncPtr{nullptr};
static SizeT             kFuncCnt{0};
static SemaphoreRef      kSemaphore{nullptr};

IMPORT_C UInt32 libmsg_close_library(Void) {
  if (kSemaphore) return kErrorInvalidData;

  if (kFuncPtr) kFuncPtr   = nullptr;
  if (kFuncCnt) kFuncCnt = 0;

  return kErrorSuccess;
}

IMPORT_C UInt32 libmsg_eval_expr(struct LIBMSG_EXPR* head, VoidPtr arg, SizeT arg_size) {
  if (kSemaphore) return kErrorInvalidData;
  if (!head || !arg || !arg_size) return kErrorInvalidData;

  static auto kSemWaitTime = 1000;

  kSemaphore = ::SemCreate(0, kSemWaitTime, "libmsg_semaphore");

  if (!kSemaphore) return kErrorInvalidData;

  kFuncPtr[head->l_index](head, arg, arg_size);

  ::SemClose(kSemaphore);
  kSemaphore = nullptr;

  return kErrorSuccess;
}

IMPORT_C Void libmsg_init_library(libmsg_func_type* funcs, SizeT cnt) {
  if (!funcs || !cnt) return;
  if (!kFuncPtr || !kFuncCnt) return;

  kFuncPtr   = funcs;
  kFuncCnt = cnt;

  MUST_PASS(kFuncPtr != nullptr);
  MUST_PASS(kFuncCnt > 0);
}
