// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <BootKit/BootKit.h>
#include <BootKit/Platform.h>
#include <BootKit/Protocol.h>
#include <FirmwareKit/EFI/API.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/// BUGS: 0																							  ///
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
@brief puts wrapper over EFI ConOut.
*/
Boot::BootTextWriter& Boot::BootTextWriter::Write(const CharacterTypeUTF16* str) {
#ifdef __DEBUG__
  if (!str || *str == 0) return *this;

  CharacterTypeUTF16 strTmp[2];
  strTmp[1] = 0;

  for (size_t i = 0; str[i] != 0; i++) {
    if (str[i] == '\r') {
      strTmp[0] = str[i];
      ST->ConOut->OutputString(ST->ConOut, strTmp);

      strTmp[0] = '\n';
      ST->ConOut->OutputString(ST->ConOut, strTmp);
    } else {
      strTmp[0] = str[i];
      ST->ConOut->OutputString(ST->ConOut, strTmp);
    }
  }
#else
  NE_UNUSED(str);
#endif  // ifdef __DEBUG__

  return *this;
}

/// @brief UTF-8 equivalent of Write (UTF-16).
/// @param str the input string.
Boot::BootTextWriter& Boot::BootTextWriter::Write(const Char* str) {
#ifdef __DEBUG__
  if (!str || *str == 0) return *this;

  CharacterTypeUTF16 strTmp[2];
  strTmp[1] = 0;

  for (size_t i = 0; str[i] != 0; i++) {
    if (str[i] == '\r') {
      strTmp[0] = str[i];
      ST->ConOut->OutputString(ST->ConOut, strTmp);

      strTmp[0] = '\n';
      ST->ConOut->OutputString(ST->ConOut, strTmp);
    } else {
      strTmp[0] = str[i];
      ST->ConOut->OutputString(ST->ConOut, strTmp);
    }
  }
#else
  NE_UNUSED(str);
#endif  // ifdef __DEBUG__

  return *this;
}

Boot::BootTextWriter& Boot::BootTextWriter::Write(const UChar* str) {
#ifdef __DEBUG__
  if (!str || *str == 0) return *this;

  CharacterTypeUTF16 strTmp[2];
  strTmp[1] = 0;

  for (size_t i = 0; str[i] != 0; i++) {
    if (str[i] == '\r') {
      strTmp[0] = str[i];
      ST->ConOut->OutputString(ST->ConOut, strTmp);

      strTmp[0] = '\n';
      ST->ConOut->OutputString(ST->ConOut, strTmp);
    } else {
      strTmp[0] = str[i];
      ST->ConOut->OutputString(ST->ConOut, strTmp);
    }
  }
#else
  NE_UNUSED(str);
#endif  // ifdef __DEBUG__

  return *this;
}

/**
@brief putc wrapper over EFI ConOut.
*/
Boot::BootTextWriter& Boot::BootTextWriter::WriteCharacter(CharacterTypeUTF16 c) {
#ifdef __DEBUG__
  EfiCharType str[2];

  str[0] = c;
  str[1] = 0;
  ST->ConOut->OutputString(ST->ConOut, str);
#else
  NE_UNUSED(c);
#endif  // ifdef __DEBUG__

  return *this;
}

Boot::BootTextWriter& Boot::BootTextWriter::Write(const UInt64& x) {
#ifdef __DEBUG__
  this->Write("0x");
  this->_Write(x);
#else
  NE_UNUSED(x);
#endif  // ifdef __DEBUG__

  return *this;
}

Boot::BootTextWriter& Boot::BootTextWriter::_Write(const UInt64& x) {
#ifdef __DEBUG__
  UInt64 y = (x > 0 ? x : -x) / 16;
  UInt64 h = (x > 0 ? x : -x) % 16;

  if (y) this->_Write(y);

  /// @note Let 'X' be an invalid number of hexadecimal base.
  if (h > 16) {
    this->WriteCharacter('X');
    return *this;
  }

  if (y == ~0UL) y = -y;

  constexpr const char kNumberList[] = "0123456789ABCDEF";

  if (h > 16) return *this;

  this->WriteCharacter(kNumberList[h]);
#else
  NE_UNUSED(x);
#endif  // ifdef __DEBUG__

  return *this;
}

// EOF
