// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifdef __NE_AMD64__
.code64
.intel_syntax noprefix
#endif

#define kTypeDriver 101
#define kArchAmd64 122
#define kHandoverMagic 0xBADAA

.section .ldr

.quad kHandoverMagic
.word kTypeDriver
.word 0
.word kArchAmd64

.section .text

.global ___chkstk_ms

___chkstk_ms:
    push rcx
    push rax

    cmp rax, 0x1000
    lea rcx, [rsp + 24]
    jb .last

.probe:
    sub rcx, 0x1000
    or rcx, 0

    sub rax, 0x1000
    cmp rax, 0x1000
    ja .probe

.last:
    sub rcx, rax
    or rcx, 0

    pop rax
    pop rcx

    ret
