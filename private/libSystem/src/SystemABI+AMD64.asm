;; // SPDX-License-Identifier: Apache-2.0
;; // Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
;; // Licensed under the Apache License, Version 2.0 (see LICENSE file)
;; // Official repository: https://github.com/ne-app-eu/krnl

[bits 64]

section .text

global nesys_syscall_arg_1
global nesys_syscall_arg_2
global nesys_syscall_arg_3
global nesys_syscall_arg_4

;; Win64 in: rcx = hash, rdx/r8/r9 = args.
;; Kernel expects: r8 = hash, r9/r10/r11 = args. Move high to low so nothing is clobbered.
;; Result comes back in rax.

nesys_syscall_arg_1:
    push rbp
    mov rbp, rsp

    mov r8, rcx

    int 0x32

    pop rbp

    ret

nesys_syscall_arg_2:
    push rbp
    mov rbp, rsp

    mov r9, rdx
    mov r8, rcx

    int 0x32

    pop rbp

    ret

nesys_syscall_arg_3:
    push rbp
    mov rbp, rsp

    mov r10, r8
    mov r9, rdx
    mov r8, rcx

    int 0x32

    pop rbp

    ret

nesys_syscall_arg_4:
    push rbp
    mov rbp, rsp

    mov r11, r9
    mov r10, r8
    mov r9, rdx
    mov r8, rcx

    int 0x32

    pop rbp

    ret
