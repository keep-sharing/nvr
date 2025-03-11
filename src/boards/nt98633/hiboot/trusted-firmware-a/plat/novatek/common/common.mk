#
# Copyright (c) 2020, NovaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include lib/xlat_tables_v2/xlat_tables.mk
include $(NVT_PRJCFG_MODEL_CFG)

C_DEFINE_EXT = -D_BOARD_DRAM_ADDR_=$(BOARD_DRAM_ADDR) \
                                -D_BOARD_DRAM_SIZE_=$(BOARD_DRAM_SIZE) \
                                -D_BOARD_SHMEM_ADDR_=$(BOARD_SHMEM_ADDR) \
                                -D_BOARD_SHMEM_SIZE_=$(BOARD_SHMEM_SIZE) \
                                -D_BOARD_FDT_ADDR_=$(BOARD_FDT_ADDR) \
                                -D_BOARD_FDT_SIZE_=$(BOARD_FDT_SIZE) \
                                -D_BOARD_UBOOT_ADDR_=$(BOARD_UBOOT_ADDR) \
                                -D_BOARD_UBOOT_SIZE_=$(BOARD_UBOOT_SIZE) \
                                -D_BOARD_BL31_ADDR_=$(BOARD_ATF_ADDR) \
                                -D_BOARD_BL31_SIZE_=$(BOARD_ATF_SIZE) \
                                -D_BOARD_TEEOS_ADDR_=$(BOARD_TEEOS_ADDR) \
                                -D_BOARD_TEEOS_SIZE_=$(BOARD_TEEOS_SIZE) \
                                -D_$(NVT_FPGA_EMULATION)_

TF_CFLAGS += $(C_DEFINE_EXT)
ASFLAGS += $(C_DEFINE_EXT)

NVT_PLAT			:=	plat/novatek

PLAT_INCLUDES		:=	-Iinclude/plat/arm/common		\
				-Iinclude/plat/arm/common/aarch64	\
				-I${NVT_PLAT}/common/include		\
				-I${NVT_PLAT}/${PLAT}/include

include lib/libfdt/libfdt.mk

PLAT_BL_COMMON_SOURCES	:=	drivers/ti/uart/${ARCH}/16550_console.S	\
				${XLAT_TABLES_LIB_SRCS}			\
				${NVT_PLAT}/common/plat_helpers.S

BL31_SOURCES		+=	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v2/gicv2_helpers.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				drivers/delay_timer/delay_timer.c       \
				drivers/delay_timer/generic_delay_timer.c \
				plat/common/plat_gicv2.c		\
				plat/common/plat_psci_common.c          \
				${NVT_PLAT}/common/bl31_setup.c	\
				${NVT_PLAT}/common/common.c	\
				${NVT_PLAT}/common/topology.c	\
				${NVT_PLAT}/common/pm.c	\
				${NVT_PLAT}/common/nvt_headinfo.c	\
				${NVT_PLAT}/${PLAT}/setup.c

ifeq ($(SDK_CODENAME), na51102)
BL31_SOURCES		+=	lib/cpus/${ARCH}/cortex_a53.S
BL31_SOURCES		+=	${NVT_PLAT}/common/nvt_tzasc_na51102.c 
BL31_SOURCES		+=	${NVT_PLAT}/common/nvt_at_pattern.S
else
ifeq ($(SDK_CODENAME), ns02201)
BL31_SOURCES		+=	lib/cpus/${ARCH}/cortex_a53.S
BL31_SOURCES		+=	lib/cpus/${ARCH}/cortex_a73.S
BL31_SOURCES		+=	${NVT_PLAT}/common/nvt_tzasc_ns02201.c
else
BL31_SOURCES		+=	lib/cpus/${ARCH}/cortex_a53.S
BL31_SOURCES		+=	${NVT_PLAT}/common/nvt_tzasc.c
endif
endif

ifeq ($(USE_OPTEE), 1)
BL31_SOURCES		+=	services/spd/opteed/opteed_common.c     \
				services/spd/opteed/opteed_helpers.S    \
				services/spd/opteed/opteed_main.c       \
				services/spd/opteed/opteed_pm.c
endif

# The bootloader is guaranteed to run all CPU.
COLD_BOOT_SINGLE_CPU		:=	0

# Do not enable SPE (not supported on ARM v8.0).
ENABLE_SPE_FOR_LOWER_ELS	:=	0

# Do not enable SVE (not supported on ARM v8.0).
ENABLE_SVE_FOR_NS		:=	0

# Enable workarounds for Cortex-A53 errata. Allwinner uses at least r0p4.
ERRATA_A53_835769		:=	1
ERRATA_A53_843419		:=	1
ERRATA_A53_855873		:=	1

# The reset vector can be changed for each CPU.
PROGRAMMABLE_RESET_ADDRESS	:=	1

# Allow mapping read-only data as execute-never.
SEPARATE_CODE_AND_RODATA	:=	1

# BL31 gets loaded alongside BL33 (U-Boot) by U-Boot's SPL
RESET_TO_BL31			:=	1


USE_COHERENT_MEM		:=	0

# required so that optee code can control access to the timer registers
NS_TIMER_SWITCH         :=      1
