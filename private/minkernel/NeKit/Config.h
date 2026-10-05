// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#ifndef NEKIT_CONFIG_H
#define NEKIT_CONFIG_H

#include <NeKit/Macros.h>

#define NEKIT_VERSION "0.0.4"
#define NEKIT_VERSION_BCD 0x0004

#if __cplusplus < 202002L
#error !!! Ne.app NeKernel compiles with C++20 as of December 4, 2025 !!!
#endif

#define NE_ICODEC \
  final:          \
  public          \
  ::Ne::Kernel::ICodec

/// @brief The **NeKernel** namespace.
namespace Ne::Kernel {

using voidPtr = void*;
using VoidPtr = void*;
using nullPtr = decltype(nullptr);
using NullPtr = decltype(nullptr);

using Int     = int;
using Int32   = __INT32_TYPE__;
using UShort  = __UINT16_TYPE__;
using UInt16  = __UINT16_TYPE__;
using Short   = short;
using Int16   = __INT16_TYPE__;
using UInt    = __UINT32_TYPE__;
using UInt32  = __UINT32_TYPE__;
using Long    = __INT64_TYPE__;
using Int64   = __INT64_TYPE__;
using ULong   = __UINT64_TYPE__;
using UInt64  = __UINT64_TYPE__;
using Boolean = bool;
using Bool    = bool;
using Char    = char;
using Int8    = __INT8_TYPE__;
using Char8   = char8_t;
using UChar   = __UINT8_TYPE__;
using UInt8   = __UINT8_TYPE__;

using SSize     = long;
using SSizeT    = long;
using Size      = __SIZE_TYPE__;
using SizeT     = __SIZE_TYPE__;
using IntPtr    = __INTPTR_TYPE__;
using UIntPtr   = __UINTPTR_TYPE__;
using IntFast   = __INT_FAST32_TYPE__;
using IntFast64 = __INT_FAST64_TYPE__;
using PtrDiff   = __PTRDIFF_TYPE__;

using SInt16 = Int16;
using SInt32 = Int32;
using SInt64 = Int64;

typedef UIntPtr* Ptr64;
typedef UInt32*  Ptr32;
typedef UInt8*   Ptr8;

using Utf8Char  = char8_t;
using Utf16Char = char16_t;
using WideChar  = wchar_t;
using Utf32Char = char32_t;

using LongDouble = long double;
using Double     = double;
using Float      = float;

typedef UInt32  PhysicalAddressKind;
typedef UIntPtr VirtualAddressKind;

using Void = void;
using Any  = void*;

using Lba = UInt64;

using Char16 = char16_t;
using Char8 = char8_t;

enum struct Endian : UInt8 {
  kEndianInvalid,
  kEndianBig,
  kEndianLittle,
  kEndianMixed,
  kEndianCount
};

/// @brief Forward object.
/// @tparam Args the object type.
/// @param arg the object.
/// @return object's rvalue
template <typename Args>
inline Args&& forward(Args& arg) {
  return static_cast<Args&&>(arg);
}

/// @brief Move object.
/// @tparam Args the object type.
/// @param arg the object.
/// @return object's rvalue
template <typename Args>
inline Args&& move(Args&& arg) {
  return static_cast<Args&&>(arg);
}

template <class Type>
concept IsSerializable = requires() {
  { Type::kValue };
};

template <class Type>
concept IsNotSerializable = requires() {
  { !Type::kValue };
};

/// @brief Encoding interface, used as a proxy to convert T to Char*
/// Used to cast A to B or B to A.
class ICodec {
 public:
  explicit ICodec() = default;
  virtual ~ICodec() = default;

  ICodec& operator=(const ICodec&) = default;
  ICodec(const ICodec&)            = default;

 public:
  /// @brief Convert type to bytes.
  /// @tparam T the type.
  /// @param type (a1) the data.
  /// @return a1 as Char*
  template <typename T>
  const Char* AsBytes(T type) {
    NE_UNUSED(type);
    return nullptr;
  }

  /// @brief Construct from type to class.
  /// @tparam T the type to convert.
  /// @param type (a1) the data.
  /// @return a1 as Char*
  template <typename OutputClass, typename FactoryClass>
  OutputClass* Construct(Char* type) {
    return FactoryClass{}.template From<OutputClass>(type);
  }

  /// @brief Convert T to a Y type.
  /// @note The class must be serializable, otherwise this will fail.
  /// @tparam Concept the class type of type.
  /// @tparam RetType the result class.
  /// @param type the class to cast.
  /// @return the class as Y.
  template <IsSerializable Concept, typename RetType>
  RetType As(Concept& type) {
    return type.template As<RetType>();
  }

  template <IsNotSerializable Concept>
  Void As(Concept& type) {
    static_assert(type, "Concept is not serializable.");
  }
};

/// \brief Scheduler interface, represents a scheduler object.
/// @note This is used to schedule tasks, such as threads, drivers, user threads, etc.
class ISchedulable {
 public:
  explicit ISchedulable() = default;
  virtual ~ISchedulable() = default;

  ISchedulable& operator=(const ISchedulable&) = default;
  ISchedulable(const ISchedulable&)            = default;

  /// @brief Is this object only accepting user tasks?
  virtual Bool IsUser() { return NO; }

  /// @brief Is this object only accepting kernel tasks?
  virtual Bool IsKernel() { return NO; }

  /// @brief Is this object offloading to another CPU?
  virtual Bool HasMP() { return NO; }
  
};

template <class Type>
struct FalseResult final {
  using ResultType    = Type;
  using ResultTypeRef = ResultType&;
  using TypeRef       = ResultTypeRef;
  using ConstType     = const Type&;
  using TypePtr       = Type*;

  static constexpr bool kValue = NO;
};

template <class Type>
struct TrueResult final {
  using ResultType    = Type;
  using ResultTypeRef = ResultType&;
  using TypeRef       = ResultTypeRef;
  using ConstType     = const Type&;
  using TypePtr       = Type*;

  static constexpr bool kValue = YES;
};

template <class Type>
struct PropertyResult final {
  using ResultType    = Type;
  using ResultTypeRef = ResultType&;
  using TypeRef       = ResultTypeRef;
  using ConstType     = const Type&;
  using TypePtr       = Type*;

  static constexpr bool kValue = Type::kValue;
};

}  // namespace Ne::Kernel

#endif
