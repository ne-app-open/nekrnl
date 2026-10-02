// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifndef NEKIT_JSON_H
#define NEKIT_JSON_H

/// @brief NeAnt JSON API for the configuration files.

#include <CompilerKit/CompilerKit.h>
#include <NeKit/Config.h>
#include <NeKit/KString.h>
#include <NeKit/Stream.h>
#include <NeKit/Utils.h>

#define kNeJsonMaxLen (8196)
#define kNeJsonLen (256)
#define kNeJsonNullArr "[]"
#define kNeJsonNullObj "{}"
#define kNeJsonNullKey "null"
#define kNeJsonNullValue kNeJsonNullKey

namespace Ne::Kernel {
/// ================================================================================
/// @brief JSON Object type.
/// ================================================================================
template <typename CharKind = Char>
class JsonObject final {
 public:
  JsonObject() : fUndefined(YES) {
    KBasicString<CharKind> key = KString(kNeJsonMaxLen);
    key += kNeJsonNullValue;

    this->AsKey()   = key;
    this->AsValue() = key;
  }

  JsonObject(SizeT lhsLen, SizeT rhsLen) : fUndefined(NO), fKey(lhsLen), fValue(rhsLen) {
    KBasicString<CharKind> key = KString(lhsLen);
    this->AsKey()              = key;

    KBasicString<CharKind> value = KString(rhsLen);
    this->AsValue()              = value;
  }

  ~JsonObject() = default;

  NE_COPY_DEFAULT(JsonObject)
  NE_MOVE_DEFAULT(JsonObject)

  Bool& IsUndefined() { return fUndefined; }

 private:
  Bool                   fUndefined{YES};  // is this instance undefined?
  KBasicString<CharKind> fKey;
  KBasicString<CharKind> fValue;

 public:
  /// @brief returns the key of the json
  /// @return the key as string view.
  KBasicString<CharKind>& AsKey() { return fKey; }

  /// @brief returns the value of the json.
  /// @return the key as string view.
  KBasicString<CharKind>& AsValue() { return fValue; }

  STATIC JsonObject<CharKind> kNull;
};

/// ================================================================================
/// @brief JsonObject stream reader helper for ASCII.
/// ================================================================================
template <typename CharType>
struct BasicJsonStreamReader final {
  STATIC JsonObject<CharType> In(const CharType* full_array) {
    auto    start_val   = '{';
    auto    end_val     = '}';
    Boolean probe_value = false;

    if (full_array[0] != start_val) {
      if (full_array[0] != '[') return JsonObject<CharType>{0, 0};

      start_val = '[';
      end_val   = ']';

      probe_value = true;
    }

    SizeT len = rt_string_len(full_array);

    SizeT key_len   = 0;
    SizeT value_len = 0;

    JsonObject<CharType> type(kNeJsonMaxLen, kNeJsonMaxLen);

    for (SizeT i = 1; i < len; ++i) {
      if (full_array[i] == '\r' || full_array[i] == '\n') continue;

      if (probe_value) {
        if (full_array[i] == end_val || full_array[i] == ',') {
          probe_value = false;

          ++value_len;
        } else {
          if (full_array[i] == '\'') {
            type.AsValue().Data()[value_len] = 0;
            break;
          }

          type.AsValue().Data()[value_len] = full_array[i];

          ++value_len;
        }
      } else {
        if (start_val == '[') continue;

        if (full_array[i] == ':') {
          type.AsKey().Data()[key_len] = 0;
          ++key_len;

          ++i;

          while (full_array[i] == ' ' || full_array[i] == '\t') ++i;

          probe_value = true;
        } else {
          type.AsKey().Data()[key_len] = full_array[i];

          ++key_len;
        }
      }
    }

    type.AsValue().Data()[value_len] = 0;

    return type;
  }
};

/// ================================================================================
/// @brief AsciiJsonStream and WideJsonStream type definitions.
/// ================================================================================
using AsciiJsonStream = Stream<BasicJsonStreamReader<Char>, JsonObject<Char>>;
using WideJsonStream = Stream<BasicJsonStreamReader<WideChar>, JsonObject<WideChar>>;

/// ================================================================================
/// @brief UTF*JsonStream type definitions.
/// ================================================================================
using Utf8JsonStream = Stream<BasicJsonStreamReader<UInt8>, JsonObject<UInt8>>;
using Utf16JsonStream = Stream<BasicJsonStreamReader<UInt16>, JsonObject<UInt16>>;
}  // namespace Ne::Kernel

#endif
