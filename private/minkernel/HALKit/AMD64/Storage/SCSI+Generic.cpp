// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <modules/SCSI/SCSI.h>

using namespace Ne::Kernel;

///! @brief ATAPI SCSI packet.
const ATTRIBUTE(unused) scsi_packet_type_12 kCDRomPacketTemplate = {0x43, 0,  1,    0, 0, 0,
                                                                    0,    12, 0x40, 0, 0};
// EOF
