// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <CFKit/GUIDWizard.h>
#include <NeKit/Ref.h>

// begin of ascii 'readable' characters. (A, C, C, 1, 2)
#define kGUIDAsciiBegin 47

// @brief Size of UUID.
#define kGUIDSize 37

namespace Ne::Kernel::CF::XRN::Version1 {
auto cf_make_sequence(const Array<UInt32, 10>& uuidSeq) -> Ref<GUIDSequence*> {
  GUIDSequence* seq = new GUIDSequence();
  MUST_PASS(seq);

  Ref<GUIDSequence*> seq_ref{seq};

  if (!seq) return seq_ref;

  seq_ref.Leak()->fUuid.fMs1    = uuidSeq[0];
  seq_ref.Leak()->fUuid.fMs2    = uuidSeq[1];
  seq_ref.Leak()->fUuid.fMs3    = uuidSeq[2];
  seq_ref.Leak()->fUuid.fMs4[0] = uuidSeq[3];
  seq_ref.Leak()->fUuid.fMs4[1] = uuidSeq[4];
  seq_ref.Leak()->fUuid.fMs4[2] = uuidSeq[5];
  seq_ref.Leak()->fUuid.fMs4[3] = uuidSeq[6];
  seq_ref.Leak()->fUuid.fMs4[4] = uuidSeq[7];
  seq_ref.Leak()->fUuid.fMs4[5] = uuidSeq[8];
  seq_ref.Leak()->fUuid.fMs4[6] = uuidSeq[9];
  seq_ref.Leak()->fUuid.fMs4[7] = uuidSeq[10];

  return seq_ref;
}

// @brief Tries to make a guid out of a string.
// This function is not complete for now
auto cf_try_guid_to_string(Ref<GUIDSequence*>& seq) -> ErrorOr<Ref<KString>> {
  if (!seq.Leak()) return {};
  
  Char buf[kGUIDSize] = {0};

  for (SizeT index = 0; index < 16; ++index) {
    buf[index] = seq.Leak()->fU8[index] + kGUIDAsciiBegin;
  }

  for (SizeT index = 16; index < 24; ++index) {
    buf[index] = seq.Leak()->fU16[index] + kGUIDAsciiBegin;
  }

  for (SizeT index = 24; index < 28; ++index) {
    buf[index] = seq.Leak()->fU32[index] + kGUIDAsciiBegin;
  }

  auto view = KStringBuilder::Construct(buf);

  if (view) return ErrorOr<Ref<KString>>{view.Leak()};

  return ErrorOr<Ref<KString>>{kErrorInvalidData};
}
}  // namespace Ne::Kernel::CF::XRN::Version1
