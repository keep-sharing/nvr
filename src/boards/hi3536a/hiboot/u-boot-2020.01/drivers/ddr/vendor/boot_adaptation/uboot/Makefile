##SPDX-License-Identifier: GPL-2.0
##Copyright (c) 2020 Shenshu Technologies CO., LIMITED.

ifeq ($(TOPDIR),)
TOPDIR   := $(realpath $(dir $(lastword $(MAKEFILE_LIST)))/../../../../..)
endif
DDR_TOP_DIR := $(TOPDIR)/drivers/ddr/vendor

EXTRA_CFLAGS += -I$(DDR_TOP_DIR)/boot_adaptation/uboot \
	-I$(DDR_TOP_DIR)/include

obj-y   := ddrtrn_uboot_cmd_loc.o
obj-y   += ddrtrn_uboot_cmd_ctl.o
obj-y   += ddrtrn_cmd_parsing.o
obj-y   += ddrtrn_boot.o
obj-y   += ../../training/ddrtrn_phy.o
obj-y   += ../../hal/phy/common/ddrtrn_hal_phy.o
obj-y   += ../../hal/phy/common/ddrtrn_hal_phy_common.o

ifeq ($(SOC), ss626v100)
EXTRA_CFLAGS += -DDDR_CHIP_SS626V100
obj-y   += ../../hal/chip/$(SOC)/ddrtrn_training_custom.o
obj-y   += ../../hal/phy/v0/ddrtrn_phy_s14.o
else
$(error "Not support soc!!!" $(SOC) "end")
endif

