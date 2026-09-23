// SPDX-License-Identifier: Apache-2.0
// Copyright 2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifndef LIBPOSIX_POSIXKIT_POSIX_H
#define LIBPOSIX_POSIXKIT_POSIX_H

#include <SystemKit/System.h>

/// @file POSIX.h
/// @brief POSIX definitions header for the NeKernel.

/// @brief Please use these macros to specify whether your function is thread safe or not.
#define PTHREAD_UNSAFE
#define PTHREAD_SAFE

#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE __POSIX_SOURCE__
#endif

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE __XOPEN_SOURCE__
#endif

PTHREAD_UNSAFE IMPORT_C SInt64 _write(SizeT count, SInt32 fd, Void* data, SizeT sz);
PTHREAD_UNSAFE IMPORT_C SInt64 _read(SizeT count, SInt32 fd, Void* data, SizeT sz);

#ifndef read
#define read _read
#endif

#ifndef write
#define write _write
#endif

#endif  // LIBPOSIX_POSIXKIT_POSIX_H
