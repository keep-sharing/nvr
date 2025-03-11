##
## Makefile.env
## Author:        zbing@milesight.cn
## Created at:    2015-05-27
## Modify at:     2016-03-25
##                
## Description:   Add env Makefile and compile update tools
##                1.fw_printenv
##                2.fw_setenv
##                
## Copyright (C)  milesight
##

HIBOOT_DIR		= $(FW_TOPDIR)/../hiboot
UBOOT_DIR 		= $(HIBOOT_DIR)/u-boot-2020.01
ENV_TOOLS_DIR		= $(UBOOT_DIR)/tools/env
ENV_SRC_DIR 		:= $(FW_TOPDIR)/env
ENV_SRCS		:= $(ENV_SRC_DIR)/fw_env.c $(ENV_SRC_DIR)/crc32.c
ENV_OBJS 		:= $(ENV_SRC_DIR)/fw_env.o $(ENV_SRC_DIR)/crc32.o
ALL_SRCS		:= $(ENV_SRC_DIR)/crc32.c  $(ENV_SRC_DIR)/fw_env.c $(ENV_SRC_DIR)/fw_env_main.c
UBOOT_VER_FILE	:= $(UBOOT_DIR)/include/configs/ms_nvrcfg.h
UBOOT_DEFCONFIG := ss626v100_emmc_defconfig
GSL_PLATFORM_FILE := $(HIBOOT_DIR)/gsl/include/platform.h
LOAD_KERNEL_ORI_ADDR := 0x44080000
LITE_OS_SIZE := 0x4000000#64MB

include $(UBOOT_DIR)/config.mk
ENVFLAGS 		:= -Wall $(PLATFORM_FLAGS) -I$(ENV_SRC_DIR) -I$(UBOOT_DIR)/include
ifeq ($(MTD_VERSION),old)
ENVFLAGS 		+= -DMTD_OLD
endif
HIBOOT_FLAGS 	:= $(MS_MAKE_JOBS) -C $(UBOOT_DIR) O="" ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE)
ifeq ($(DDR_SIZE),$(DDR_SIZE_2G))
UBOOT_DDR_SIZE	= (2*1024*1024*1024)
ARM_DDR_SIZE	= 1224
UBOOT_REG_CFG	:= ss626v100_reg_info_2048M.bin
else
UBOOT_DDR_SIZE	= (4*1024*1024*1024)
ARM_DDR_SIZE	= 2880
UBOOT_REG_CFG	:= ss626v100_reg_info_4096M.bin
endif
LOAD_KERNEL_ADDR = $(shell echo $$(($(UBOOT_DDR_SIZE)-$(ARM_DDR_SIZE)*1024*1024-$(LITE_OS_SIZE)+$(LOAD_KERNEL_ORI_ADDR))))
LOAD_KERNEL_ADDR_TO_HEX = $(shell printf '0x%08x' $(LOAD_KERNEL_ADDR))

UBOOT_CMDLINE	= mem=$(ARM_DDR_SIZE)M console=ttyAMA0,115200 clk_ignore_unused rw rootwait root=/dev/mmcblk0p6 rootfstype=ext4

hiboot-config:
	$(MS_MAKEFILE_V)$(MAKE) $(HIBOOT_FLAGS) distclean
	$(MS_MAKEFILE_V)cp $(UBOOT_DIR)/configs/$(UBOOT_DEFCONFIG) $(UBOOT_DIR)/.config
	#$(MS_MAKEFILE_V)cp $(HIBOOT_DIR)/mkboot.sh $(UBOOT_DIR)
	$(MS_MAKEFILE_V)$(MAKE) $(HIBOOT_FLAGS) menuconfig
	$(MS_MAKEFILE_V)cp $(UBOOT_DIR)/.config $(UBOOT_DIR)/configs/$(UBOOT_DEFCONFIG)

hiboot-prepare:
	$(MS_MAKEFILE_V)echo "Configure total memory=$(DDR_SIZE) version=$(HIBOOT_VERSION)"
	$(MS_MAKEFILE_V)echo "Configure cmdline '$(UBOOT_CMDLINE)'"
	@sed -i -e "s,\#define MS_UBOOT_VERSION.*,\#define MS_UBOOT_VERSION\t\"$(HIBOOT_VERSION)\"," $(UBOOT_VER_FILE)
	@sed -i -e "s,\#define MS_DDR_VERSION.*,\#define MS_DDR_VERSION\t$(VERSION_DDR)," $(UBOOT_VER_FILE)
	@sed -i -e "s%\#define CONFIG_BOOTARGS.*%\#define CONFIG_BOOTARGS\t\t\"$(UBOOT_CMDLINE)\"%" $(UBOOT_VER_FILE)
	@sed -i -e "s,\#define CFG_DDR_SIZE.*,\#define CFG_DDR_SIZE\t\t$(UBOOT_DDR_SIZE)," $(UBOOT_VER_FILE)
	@sed -i -e "s,\#define KERNEL_LOAD_ADDR.*,\#define KERNEL_LOAD_ADDR\t$(LOAD_KERNEL_ADDR_TO_HEX)," $(GSL_PLATFORM_FILE)
	@sed -i -e "s,CONFIG_KERNEL_LOAD_ADDR.*,CONFIG_KERNEL_LOAD_ADDR=$(LOAD_KERNEL_ADDR_TO_HEX)," $(UBOOT_DIR)/configs/$(UBOOT_DEFCONFIG)

hiboot: hiboot-prepare
	$(MS_MAKEFILE_V)$(MAKE) -C $(HIBOOT_DIR)/gsl CHIP=$(CHIP_MODEL)
	$(MS_MAKEFILE_V)cp $(HIBOOT_DIR)/gsl/pub/gsl.bin $(HIBOOT_DIR)/image_map/
	$(MS_MAKEFILE_V)cp $(HIBOOT_DIR)/$(UBOOT_REG_CFG) $(UBOOT_DIR)/.reg
	$(MS_MAKEFILE_V)$(MAKE) $(HIBOOT_FLAGS) $(UBOOT_DEFCONFIG)
	$(MS_MAKEFILE_V)$(MAKE) -C $(UBOOT_DIR) CROSS_COMPILE=$(CROSS_COMPILE)
	$(MS_MAKEFILE_V)$(MAKE) -C $(UBOOT_DIR) CROSS_COMPILE=$(CROSS_COMPILE) u-boot-z.bin
	$(MS_MAKEFILE_V)cp $(UBOOT_DIR)/u-boot-$(CHIP_MODEL).bin $(HIBOOT_DIR)/image_map/u-boot-original.bin
	$(MS_MAKEFILE_V)cp $(UBOOT_DIR)/u-boot-$(CHIP_MODEL).bin $(FIRMWARE_IMG_DIR)/fast_uboot #tanggp modify for creat fastboot
	$(MS_MAKEFILE_V)cp $(UBOOT_DIR)/.reg $(HIBOOT_DIR)/image_map/.reg
	$(MS_MAKEFILE_V)cd $(HIBOOT_DIR)/image_map/;python oem/oem_quick_build.py
	$(MS_MAKEFILE_V)cp $(HIBOOT_DIR)/image_map/image/oem/boot_image.bin $(FIRMWARE_IMG_DIR)/uboot_$(DDR_NAME)

hiboot-clean:
	-$(MS_MAKEFILE_V)rm -rf $(UBOOT_DIR)/mkboot.sh $(UBOOT_DIR)/.reg
	$(MS_MAKEFILE_V)$(MAKE) $(HIBOOT_FLAGS) distclean
	-$(MS_MAKEFILE_V)rm -rf $(UBOOT_DIR)/arch/arm/cpu/armv8/ss626v100/hw_compressed/u-boot-ss626v100.*
	-$(MS_MAKEFILE_V)rm -rf $(UBOOT_DIR)/arch/arm/cpu/armv8/ss626v100/hw_compressed/image_data.gzip
	$(MS_MAKEFILE_V)$(MAKE) -C $(HIBOOT_DIR)/gsl clean
	-$(MS_MAKEFILE_V)cd $(HIBOOT_DIR)/image_map/;python oem/ms_clean.py

envtool:
ifeq ($(wildcard $(PUBLIC_STATIC_LIB_DIR)/libarm-env.a),)
	$(MS_MAKEFILE_V)$(MAKE) $(HIBOOT_FLAGS) $(UBOOT_DEFCONFIG)
	$(MS_MAKEFILE_V)$(MAKE) $(HIBOOT_FLAGS) envtools
	$(MS_MAKEFILE_V)cp -dpRf $(ENV_TOOLS_DIR)/lib.a $(PUBLIC_STATIC_LIB_DIR)/libarm-env.a
	$(MS_MAKEFILE_V)cp -dpRf $(ENV_TOOLS_DIR)/fw_printenv $(PUBLIC_APP_DIR)/
	#$(MS_MAKEFILE_V)cd $(PUBLIC_APP_DIR) && ln -sf fw_printenv fw_setenv && cd -
	$(MS_MAKEFILE_V)$(MAKE) $(HIBOOT_FLAGS) env clean
endif

envtool-clean:
	#-$(MS_MAKEFILE_V)rm -rf $(PUBLIC_STATIC_LIB_DIR)/libarm-env.a // keep 
	-$(MS_MAKEFILE_V)rm -rf $(PUBLIC_APP_DIR)/fw_printenv $(PUBLIC_APP_DIR)/fw_setenv
