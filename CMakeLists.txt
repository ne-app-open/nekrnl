# SPDX-License-Identifier: Apache-2.0
# Copyright 2024-2026, Ne.app (amlal@nekernel.org)
# Licensed under the Apache License, Version 2.0 (see LICENSE file)
# Official repository: https://github.com/ne-app-eu/krnl

if(DEFINED NEANT_RELEASE_PROFILE)

message("NeAnt: Running release AHCI build...")
execute_process(COMMAND ./scripts/release_ahci_x64.sh)

elseif(DEFINED NEANT_DEBUG_PROFILE)

message("NeAnt: Running debug AHCI build...")
execute_process(COMMAND ./scripts/debug_ahci_x64.sh)

endif()
