###############################################################
# (c) Amlal El Mahrouss and Ne.app, licensed under the Apache 2.0 license.
# This is the kernel makefile.
###############################################################

CC			= aarch64-w64-mingw32-g++
LD			= lld-link
CCFLAGS		= -fshort-wchar -I../../EXTERNAL_HEADERS -c -ffreestanding -MMD -mno-red-zone -D__NE_ARM64__ -fno-rtti -fno-exceptions -I./ \
				-std=c++20 -D__nekernel_max_cores=8 -D__nekernel_dma_best_align=8 -O3 -D__NEKERNEL__ -D__NEOSKRNL__  -D__NE_VEPM__ -D__NE_MINIMAL_OS__ -D__NE_NO_BUILTIN__ -D__HAVE_NE_API__ -D__NE__ -I../

ASM 		= clang++

DISKDRIVER = -D__USE_FLASH_MEM__

ifneq ($(DEBUG_SUPPORT), )
DEBUG =  -D__DEBUG__
endif

COPY		= cp

LDFLAGS		= -subsystem:efi_application -entry:hal_init_platform /nodefaultlib
LDOBJ		= obj/*.obj

# This file is the Kernel, responsible of task management and memory.
KERNEL_IMG		= vmoskrnl.exe

.PHONY: error
error:
	@echo "=== ERROR ==="
	@echo "=> Use a specific target."

MOVEALL=./move-all-aarch64.sh

.PHONY: nekernel-arm64-epm
nekernel-arm64-epm: clean
	$(CC) $(CCFLAGS) $(DISKDRIVER) $(DEBUG) $(wildcard src/*.cpp) \
	       $(wildcard src/FS/*.cpp) $(wildcard HALKit/Generic/*.cpp) $(wildcard HALKit/ARM64/Storage/*.cpp) \
			$(wildcard HALKit/ARM64/PCI/*.cpp) $(wildcard src/Network/*.cpp) $(wildcard src/Storage/*.cpp) \
			$(wildcard HALKit/ARM64/*.cpp) $(wildcard HALKit/ARM64/*.cpp) \
			$(wildcard HALKit/ARM64/*.s) $(wildcard HALKit/ARM64/APM/*.cpp)

	$(MOVEALL)

OBJCOPY=x86_64-w64-mingw32-objcopy

.PHONY: link-arm64-epm
link-arm64-epm:
	$(LD) $(LDFLAGS) $(LDOBJ) /out:$(KERNEL_IMG)

.PHONY: all
all: nekernel-arm64-epm link-arm64-epm
	@echo "Kernel => OK."

.PHONY: help
help:
	@echo "=== HELP ==="
	@echo "all: Build Kernel and link it."
	@echo "link-arm64-epm: Link Kernel for EPM based disks."
	@echo "nekernel-arm64-epm: Build Kernel for EPM based disks."

.PHONY: clean
clean:
	rm -f $(LDOBJ) $(wildcard *.o) $(KERNEL_IMG)
