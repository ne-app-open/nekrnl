<!-- Read Me of Ne.app Kernel -->

# Ne.app Kernel 🏗️

> [!NOTE]
> On Windows, you will need to use the setup link at:
> `curl -fsSL http://setup.nekernel.org`.
> Or `curl -fsSL http://install.nekernel.org` on POSIX platforms.

![License](https://img.shields.io/badge/LICENSE-Apache--2.0-blue.svg?style=for-the-badge)
![GitHub Repo Stars](https://img.shields.io/github/stars/ne-app-eu/krnl?style=for-the-badge)

<a href="https://github.com/ne-app-eu/krnl/actions/workflows/boot-ahci-dev.yml/badge.svg"><img src="https://github.com/ne-app-eu/krnl/actions/workflows/boot-ahci-dev.yml/badge.svg" alt="CI 1"></a>
<a href="https://github.com/ne-app-eu/krnl/actions/workflows/kernel-ahci-dev.yml/badge.svg"><img src="https://github.com/ne-app-eu/krnl/actions/workflows/kernel-ahci-dev.yml/badge.svg?style=for-the-badge" alt="CI 2"></a>
<a href="https://github.com/ne-app-eu/krnl/actions/workflows/libddk-build.yml/badge.svg"><img src="https://github.com/ne-app-eu/krnl/actions/workflows/libddk-build.yml/badge.svg?style=for-the-badge" alt="CI 3"></a>
<a href="https://github.com/ne-app-eu/krnl/actions/workflows/libsystem-build.yml/badge.svg"><img src="https://github.com/ne-app-eu/krnl/actions/workflows/libsystem-build.yml/badge.svg?style=for-the-badge" alt="CI 4"></a>

The Modular Operating System Kernel for NeAnt.

## Getting Started

### **Requirements**

- [MinGW](https://www.mingw-w64.org/)
- [Clang](https://clang.llvm.org/)
- [NASM](https://nasm.us/)
- [NeBuild](https://github.com/ne-app-open/bld)
- CoreUtils
- [Git](https://git-scm.com/)
- [NCC](https://github.com/ne-app-open/ncc)

### **Building & Running**

Please fork, and clone the repository. Then follow those steps:

```sh
git clone -j8 https://github.com/ne-app-open/krnl.git
cd krnl
./scripts/setup_x64_project.sh
./scripts/modules_ahci_x64.sh
./scripts/debug_ahci_x64.sh   # For debug generic AHCI target (QEMU, UDF)
```

---

## User Group

Join Ne.app's [User Group](https://discord.gg/uD76Qweght) to and chat with users, contributors, and staff.

## Documentation

The [Documentation](https://docs.src.nekernel.org/) is available, alongside with primers and wiki entries.

## Licensing

Ne.app Kernel is licensed under the [Apache-2.0 License](LICENSE.txt).

---

<div align="center">
  <sub>
    &copy; 2022-2026 Ne.app. Licensed under the Apache 2.0 license.
  </sub>
</div>
