#!/bin/sh

# LOG HISTORY:
# 03/25/25: Add 'disk' build step.
# 04/05/25: Improve and fix script.

cd private
cd libSystem
cd src
make nesys_asm_io_x64
cd ..
nebuild libSystem.json
cd ../libDDK
nebuild libDDK.json
cd ../mindetect
nebuild mindetect.json
cd ../rpchost
nebuild rpchost.json
cd ../basehost
nebuild basehost.json
cd ../hal
nebuild hal.dll.x64.json
cd ../minloader
make -f amd64-desktop.make  efi
make -f amd64-desktop.make  epm-img

# curl -fsSL https://retrage.github.io/edk2-nightly/bin/DEBUGX64_OVMF_VARS.fd -o private/minloader/OVMF.VARS.fd
