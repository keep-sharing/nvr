// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

CUR_PATH := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
DDR_TOP_DIR = $(realpath $(CUR_PATH)/../..)
include $(CUR_PATH)/ddr_cfg.mk

LCSRC := $(wildcard $(DDR_TOP_DIR)/boot_adaptation/hsl/*.c)
LCSRC += $(wildcard $(DDR_TOP_DIR)/boot_adaptation/common/*.c)
LCSRC += $(wildcard $(DDR_TOP_DIR)/api/*.c)
LCSRC += $(wildcard $(DDR_TOP_DIR)/training/*.c)
LCSRC += $(wildcard $(DDR_TOP_DIR)/hal/*.c)
LCSRC += $(wildcard $(DDR_TOP_DIR)/hal/phy/common/*.c)
LCSRC += $(wildcard $(DDR_TOP_DIR)/hal/ddrc/common/*.c)
LCSRC += $(wildcard $(DDR_TOP_DIR)/hal/ddrt/common/*.c)
LCSRC += $(wildcard $(DDR_TOP_DIR)/hal/sysctrl/*.c)
LCSRC += $(wildcard $(DDR_TOP_DIR)/hal/uart/*.c)

ifeq ($(CONFIG_SYS_CPU), ss626v100)
CFLAGSCA += -DDDR_CHIP_SS626V100
LCSRC += $(wildcard $(DDR_TOP_DIR)/hal/phy/v0/*.c)
LCSRC += $(wildcard $(DDR_TOP_DIR)/hal/chip/ss626v100/*.c)
LCSRC += $(wildcard $(DDR_TOP_DIR)/hal/ddrc/v0/*.c)
else
$(error "Not support cpu!!!" $(CONFIG_SYS_CPU))
endif

CSRC += $(patsubst $(DDR_TOP_DIR)/%, ddr/%, $(LCSRC))

CFLAGSCA += -I$(DDR_TOP_DIR)/boot_adaptation/hsl/
CFLAGSCA += -I$(DDR_TOP_DIR)/include/
