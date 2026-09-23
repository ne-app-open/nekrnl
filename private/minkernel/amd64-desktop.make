###############################################################
# (c) Amlal El Mahrouss, licensed under the Apache 2.0 license.
# This is the NeKernel's makefile.
###############################################################

CXX			= x86_64-w64-mingw32-g++
LD			= x86_64-w64-mingw32-ld
CCFLAGS		= -fshort-wchar -D__nekernel_dma_pool_start=0x1000000 -D__HALKIT_INCLUDES_BNID__=1 -D__nekernel_dma_pool_size=0x1000000 \
				-D__nekernel_halkit_include_processor="<HALKit/AMD64/Processor.h>" -D__nekernel_max_cores=8 -c -D__NE_AMD64__ -D__NEOSKRNL__ -D__NE_VEPM__ -Wall -Wc++11-extensions -Wpedantic -Wextra -mno-red-zone -fno-rtti -fno-exceptions -std=c++20 -D__nekernel_dma_best_align=8 -D__FSKIT_INCLUDES_OPENHEFS__ -D__NE_SUPPORT_NX__ -O0 -I../vendor -D__NEKERNEL__ -D__HAVE_NE_API__ -D__FREESTANDING__ -D__NE_VIRTUAL_MEMORY_SUPPORT__ -D__NE_AUTO_FORMAT__ -D__NE__ -I./ -I../ -I../minloader

ASM 		= nasm

DISK_DRV  =

ifneq ($(ATA_PIO_SUPPORT), )
DISK_DRV =  -D__ATA_PIO__
endif

ifneq ($(ATA_DMA_SUPPORT), )
DISK_DRV =  -D__ATA_DMA__
endif

ifneq ($(AHCI_SUPPORT), )
DISK_DRV =  -D__AHCI__
endif

ifneq ($(DEBUG_SUPPORT), )
DEBUG_MACRO = -D__DEBUG__
endif

COPY		= cp

# Add assembler, linker, and object files variables.
ASMFLAGS	= -f win64

# Kernel subsystem is 17 and entrypoint is hal_init_platform
LDFLAGS		= -e hal_init_platform --subsystem=17 --image-base 0x10000000
LDOBJ		= obj/*.obj

# This file is the Kernel, responsible of task, memory, driver, sci, disk and device management.
KERNEL_IMG	= vmoskrnl.exe

.PHONY: error
error:
	@echo "=== ERROR ==="
	@echo "=> Use a specific target."

MOVEALL=./move-all-x64.sh
WINDRES=x86_64-w64-mingw32-windres

.PHONY: nekernel-amd64-epm
nekernel-amd64-epm: clean
	$(WINDRES) kernel_rsrc.rsrc -O coff -o kernel_rsrc.obj
	$(CXX) $(CCFLAGS) $(DISK_DRV) $(DEBUG_MACRO) $(wildcard src/*.cpp) \
	$(wildcard ../common/*.cpp) $(wildcard ../common/SystemCallsImpl/*.cpp) $(wildcard ../common/KernelCallsImpl/*.cpp) $(wildcard src/Gfx/*.cpp) $(wildcard HALKit/AMD64/Network/*.cpp) $(wildcard HALKit/AMD64/PCI/*.cpp) $(wildcard src/Network/*.cpp) $(wildcard src/Storage/*.cpp) $(wildcard src/FS/*.cpp) $(wildcard HALKit/AMD64/Storage/*.cpp) $(wildcard HALKit/AMD64/*.cpp) $(wildcard HALKit/Generic/*.cpp) $(wildcard src/Swap/*.cpp) $(wildcard HALKit/AMD64/*.s)
	$(ASM) $(ASMFLAGS) HALKit/AMD64/HalInterruptAPI.asm
	$(ASM) $(ASMFLAGS) HALKit/AMD64/HalCommonAPI.asm
	$(ASM) $(ASMFLAGS) HALKit/AMD64/HalHandoverStub.asm
	$(ASM) $(ASMFLAGS) HALKit/AMD64/HalUtilsAPI.asm
	$(MOVEALL)

OBJCOPY=x86_64-w64-mingw32-objcopy

.PHONY: link-amd64-epm
link-amd64-epm:
	$(LD) $(LDFLAGS) $(LDOBJ) -o $(KERNEL_IMG)

.PHONY: all
all: nekernel-amd64-epm link-amd64-epm
	@echo "Kernel => OK."

.PHONY: help
help:
	@echo "=== HELP ==="
	@echo "all: Build Kernel and link it."
	@echo "link-amd64-epm: Link Kernel for EPM based disks."
	@echo "nekernel-amd64-epm: Build Kernel for EPM based disks."

.PHONY: clean
clean:
	rm -f $(LDOBJ) $(wildcard *.o) $(KERNEL_IMG)
