// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <NeKit/Utils.h>
#include <NetworkKit/IP.h>

namespace Ne::Kernel {

UInt8* RawIPAddress::Address() {
  MUST_PASS(*fAddr != 0);
  return fAddr;
}

RawIPAddress::RawIPAddress(UInt8 bytes[4]) {
  MUST_PASS(*bytes != 0);
  rt_copy_memory(bytes, fAddr, 4);
}

BOOL RawIPAddress::operator==(const RawIPAddress& ipv4) {
  for (SizeT index{}; index < 4; ++index) {
    if (ipv4.fAddr[index] != fAddr[index]) return false;
  }

  return true;
}

BOOL RawIPAddress::operator!=(const RawIPAddress& ipv4) {
  for (SizeT index{}; index < 4; ++index) {
    if (ipv4.fAddr[index] == fAddr[index]) return false;
  }

  return true;
}

UInt8& RawIPAddress::operator[](const Size& index) {
  kout << "[RawIPAddress::operator[]] Fetching Index...\r";

  static UInt8 kIPPlaceholder = '0';
  if (index >= 4) return kIPPlaceholder;

  return fAddr[index];
}

RawIPAddress6::RawIPAddress6(UInt8 bytes[16]) {
  rt_copy_memory(bytes, fAddr, 16);
}

UInt8* RawIPAddress6::Address() {
  return fAddr;
}

UInt8& RawIPAddress6::operator[](const Size& index) {
  kout << "[RawIPAddress6::operator[]] Fetching Index...\r";

  static UInt8 kIPPlaceholder = '0';
  if (index >= 16) return kIPPlaceholder;

  return fAddr[index];
}

bool RawIPAddress6::operator==(const RawIPAddress6& ipv6) {
  for (SizeT index{}; index < 16; ++index) {
    if (ipv6.fAddr[index] != fAddr[index]) return false;
  }

  return true;
}

bool RawIPAddress6::operator!=(const RawIPAddress6& ipv6) {
  for (SizeT index{}; index < 16; ++index) {
    if (ipv6.fAddr[index] == fAddr[index]) return false;
  }

  return true;
}

ErrorOr<KBasicString<UInt8>> IPFactory::ToKString(Ref<RawIPAddress6>& ipv6) {
  NE_UNUSED(ipv6);
  auto kipv6 = KStringBuilder::Construct(ipv6.Leak().Address());
  return kipv6;
}

ErrorOr<KBasicString<UInt8>> IPFactory::ToKString(Ref<RawIPAddress>& ipv4) {
  NE_UNUSED(ipv4);
  auto kipv4 = KStringBuilder::Construct(ipv4.Leak().Address());
  return kipv4;
}

bool IPFactory::IpCheckVersion4(const Char* ip) {
  if (!ip) return NO;

  Int32 cnter     = 0;
  Int32 dot_cnter = 0;

  constexpr const auto kIP4DotCharacter = '.';

  for (SizeT base = 0; base < rt_string_len(ip); ++base) {
    if (ip[base] == kIP4DotCharacter) {
      cnter = 0;
      ++dot_cnter;
    } else {
      if (ip[base] > '5' || ip[base] < '0') return NO;
      if (!rt_is_alnum(ip[base])) return NO;
      if (cnter > 3) return NO;

      ++cnter;
    }
  }

  if (dot_cnter != 3) return NO;

  return YES;
}

}  // namespace Ne::Kernel
