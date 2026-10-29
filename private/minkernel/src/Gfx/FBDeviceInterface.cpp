// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-app-eu/krnl

#include <GfxKit/FB.h>

using namespace ::Ne::Kernel;

/// @brief Class constructor
/// @param Out Drive output
/// @param In  Drive input
/// @param Cleanup Drive cleanup.
FBDeviceInterface::FBDeviceInterface(void (*out)(IDevice* self, FBDevicePacket* outpacket),
                                     void (*in)(IDevice* self, FBDevicePacket* inpacket))
    : IDevice(out, in) {}

/// @brief Class desctructor
FBDeviceInterface::~FBDeviceInterface() = default;

/// @brief Output operator.
/// @param mnt the disk mountpoint.
/// @return the class itself after operation.
FBDeviceInterface& FBDeviceInterface::operator<<(FBDevicePacket* pckt) {
  if (!pckt) return *this;

  if (pckt->fHeight == 0 || pckt->fWidth == 0) return *this;

  if (this->fOut) this->fOut(this, pckt);

  return *this;
}

/// @brief Input operator.
/// @param mnt the disk mountpoint.
/// @return the class itself after operation.
FBDeviceInterface& FBDeviceInterface::operator>>(FBDevicePacket* pckt) {
  if (!pckt) return *this;

  if (pckt->fHeight == 0 || pckt->fWidth == 0) return *this;

  if (this->fIn) this->fIn(this, pckt);

  return *this;
}

/// @brief Returns the name of the device interface.
/// @return it's name as a string.
const Char* FBDeviceInterface::Name() const {
  return kDeviceMgrRootDirPath "fb{}";
}

// END
