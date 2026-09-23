#!/bin/sh

cd private/minloader
make -f arm64-desktop.make  efi
make -f arm64-desktop.make  epm-img

# curl -fsSL https://retrage.github.io/edk2-nightly/bin/DEBUGAARCH64_QEMU_VARS.fd -o private/minloader/OVMF.VARS.fd
