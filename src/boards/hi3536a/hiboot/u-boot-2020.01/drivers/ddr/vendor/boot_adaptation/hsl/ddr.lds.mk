// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

CUR_PATH := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
DDR_TOP_DIR = $(realpath $(CUR_PATH)/../..)
include $(CUR_PATH)/ddr_cfg.mk

LOCAL_DDR_TEXT_LDS := $(wildcard $(DDR_TOP_DIR)/boot_adaptation/hsl/*.c)
LOCAL_DDR_TEXT_LDS += $(wildcard $(DDR_TOP_DIR)/boot_adaptation/common/*.c)
LOCAL_DDR_TEXT_LDS += $(wildcard $(DDR_TOP_DIR)/api/*.c)
LOCAL_DDR_TEXT_LDS += $(wildcard $(DDR_TOP_DIR)/training/*.c)
LOCAL_DDR_TEXT_LDS += $(wildcard $(DDR_TOP_DIR)/hal/*.c)
LOCAL_DDR_TEXT_LDS += $(wildcard $(DDR_TOP_DIR)/hal/phy/common/*.c)
LOCAL_DDR_TEXT_LDS += $(wildcard $(DDR_TOP_DIR)/hal/ddrc/common/*.c)
LOCAL_DDR_TEXT_LDS += $(wildcard $(DDR_TOP_DIR)/hal/ddrt/common/*.c)
LOCAL_DDR_TEXT_LDS += $(wildcard $(DDR_TOP_DIR)/hal/sysctrl/*.c)
LOCAL_DDR_TEXT_LDS += $(wildcard $(DDR_TOP_DIR)/hal/uart/*.c)

ifeq ($(CONFIG_SYS_CPU), ss626v100)
LOCAL_DDR_CHIP_TEXT_LDS += $(wildcard $(DDR_TOP_DIR)/hal/phy/v0/*.c)
LOCAL_DDR_CHIP_TEXT_LDS += $(wildcard $(DDR_TOP_DIR)/hal/chip/ss626v100/*.c)
LOCAL_DDR_CHIP_TEXT_LDS += $(wildcard $(DDR_TOP_DIR)/hal/ddrc/v0/*.c)
else
$(error "Not support cpu!!!" $(CONFIG_SYS_CPU))
endif

DDR_TEXT_LDS += $(patsubst $(DDR_TOP_DIR)/%, drv/ddr/%, $(LOCAL_DDR_TEXT_LDS))
DDR_CHIP_TEXT_LDS += $(patsubst $(DDR_TOP_DIR)/%, drv/ddr/%, $(LOCAL_DDR_CHIP_TEXT_LDS))

