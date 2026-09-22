// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-foss/kernel

#include <HALKit/Generic/PhysicalMemory.h>
#include <KernelKit/FileMgr.h>
#include <KernelKit/HeapMgr.h>
#include <KernelKit/KPC.h>
#include <KernelKit/ThreadLocalStorage.h>
#include <KernelKit/User.h>
#include <NeKit/KString.h>
#include <NeKit/KernelPanic.h>
#include <NeKit/Utils.h>

/// @file User.cpp
/// @brief Multi-user support.

namespace Ne::Kernel {

namespace Detail {
  ////////////////////////////////////////////////////////////
  /// \brief Constructs a password by hashing the password.
  /// \param password password to hash.
  /// \return the hashed password
  ////////////////////////////////////////////////////////////
  STATIC UInt64 user_fnv_generator(const UserPublicKeyType* password, User* user) {
    kout << "user_fnv_generator: Try hashing user password...\r";

    if (!password || !user) return 0;
    if (*password == 0) return 0;

    const UInt64 kFnvOffsetBasis = 0xcbf29ce484222325ULL;
    const UInt64 kFnvPrime       = 0x100000001b3ULL;

    UInt64 hash = kFnvOffsetBasis;

    MUST_PASS(hash != 0);
    MUST_PASS(*password != 0);

    while (*password) {
      hash ^= (Char) (*password++);
      hash *= kFnvPrime;
    }

    kout << "user_fnv_generator: Hashed user password.\r";

    return hash;
  }
  /// @brief Copy a user name, truncating at kMaxUserNameLen.
  STATIC Void user_set_name(Char* dst, const Char* src) {
    auto len = rt_string_len(src);

    if (len >= kMaxUserNameLen) len = kMaxUserNameLen - 1;

    urt_copy_memory((VoidPtr) src, dst, len);
    dst[len] = 0;
  }
}  // namespace Detail

////////////////////////////////////////////////////////////
/// @brief User ring constructor.
////////////////////////////////////////////////////////////
User::User(const Int32& sel, const Char* user_name) : mUserRing((UserRingKind) sel) {
  MUST_PASS(sel >= 0);
  Detail::user_set_name(this->mUserName, user_name);
}

////////////////////////////////////////////////////////////
/// @brief User ring constructor.
////////////////////////////////////////////////////////////
User::User(const UserRingKind& ring_kind, const Char* user_name) : mUserRing(ring_kind) {
  MUST_PASS(ring_kind != UserRingKind::kRingInvalid);
  MUST_PASS(*user_name != 0);
  Detail::user_set_name(this->mUserName, user_name);
}

////////////////////////////////////////////////////////////
/// @brief User destructor class.
////////////////////////////////////////////////////////////
User::~User() = default;

////////////////////////////////////////////////////////////
/// @brief Is the user an adult?
////////////////////////////////////////////////////////////
Bool User::IsAdult() {
  return this->mUserIsAdult;
}

////////////////////////////////////////////////////////////
/// @brief Set whether the user is an adult.
////////////////////////////////////////////////////////////
Void User::Adult(const Bool& adult) {
  this->mUserIsAdult = adult;
}

Bool User::Save(const UserPublicKey password) {
  if (!password || *password == 0) return No;

  this->mUserFNV = Detail::user_fnv_generator(password, this);

  kout << "User::Save: Saved password successfully...\r";

  return Yes;
}

Bool User::Login(const UserPublicKey password) {
  if (!password || !*password) return No;

  auto ret = this->mUserFNV == Detail::user_fnv_generator(password, this);

  // now check if the password matches.
  kout << (ret ? "User::Login: Password matches.\r" : "User::Login: Password doesn't match.\r");
  return ret;
}

Bool User::operator==(const User& lhs) {
  return lhs.mUserRing == this->mUserRing;
}

Bool User::operator!=(const User& lhs) {
  return lhs.mUserRing != this->mUserRing;
}

////////////////////////////////////////////////////////////
/// @brief Returns the user's name.
////////////////////////////////////////////////////////////

Char* User::Name() {
  return this->mUserName;
}

////////////////////////////////////////////////////////////
/// @brief Returns the user's ring.
/// @return The king of ring the user is attached to.
////////////////////////////////////////////////////////////

const UserRingKind& User::Ring() {
  return this->mUserRing;
}

Bool User::IsStdUser() {
  return this->Ring() == UserRingKind::kRingStdUser;
}

Bool User::IsSuperUser() {
  return this->Ring() == UserRingKind::kRingSuperUser;
}

Bool User::IsGuestUser() {
  return this->Ring() == UserRingKind::kRingGuestUser;
}

////////////////////////////////////////////////////////////
/// @brief Binds kRootUser, kGuest and kCurrentUser.
/// @param recovery make a recovery user when asked to.
////////////////////////////////////////////////////////////

Void user_init_std(const Bool& recovery) {
  /// @note heap, not function local statics. Those need guard variables and an
  /// atexit thunk, and this kernel's implementations of both are homegrown.
  if (!kRootUser) kRootUser = new User(UserRingKind::kRingSuperUser, kRootUserName);
  if (!kGuest) kGuest = new User(UserRingKind::kRingGuestUser, kGuestUserName);

  if (!kRootUser || !kGuest) {
    ke_stop(RUNTIME_CHECK_UNEXCPECTED, "Ran out of memory for users.");
    return;
  }

  if (recovery)
    kCurrentUser = new User(UserRingKind::kRingRecoveryUser, kRecoveryUserName);
  else
    kCurrentUser = kGuest;

  kHostUser = nullptr;

  (Void)(kout << "user_init_std: " << kRootUser->Name() << kendl);
  (Void)(kout << "user_init_std: " << kCurrentUser->Name() << kendl);
}

}  // namespace Ne::Kernel
