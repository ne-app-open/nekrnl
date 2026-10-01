// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

.text

.extern launch_startup_fn
.extern ThrExitMainThread
.globl _NeMain

/** 	This should not be touched unless there's a **really** valid reason to it.
	\note This helps start the ne_launch program.
*/
_NeMain:
  push %rbp
  movq %rsp, %rbp

  callq launch_startup_fn

  movq %rcx, 0
  callq ThrExitMainThread
  popq %rbp

.data
.global _ProgramData
_ProgramData:
  //;; ident
  .word 0xeecc
  //;; version
  .word 0x0110
  //;; kind: 0x0 = LaunchSystem
  .word 0x0
  //;;  flags
  .word 0x0
  //;; Launcher name
  .asciz "Driver Host"


