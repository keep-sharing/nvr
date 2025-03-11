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
UBOOT_DIR 		= $(HIBOOT_DIR)/uboot
ENV_TOOLS_DIR		= $(UBOOT_DIR)/tools/env
ENV_SRC_DIR 		:= $(FW_TOPDIR)/env
ENV_SRCS		:= $(ENV_SRC_DIR)/fw_env.c $(ENV_SRC_DIR)/crc32.c
ENV_OBJS 		:= $(ENV_SRC_DIR)/fw_env.o $(ENV_SRC_DIR)/crc32.o
ALL_SRCS		:= $(ENV_SRC_DIR)/crc32.c  $(ENV_SRC_DIR)/fw_env.c $(ENV_SRC_DIR)/fw_env_main.c
UBOOT_VER_FILE	:= $(UBOOT_DIR)/include/configs/ms_nvrcfg.h
UBOOT_REG_CFG	:= $(HIBOOT_DIR)/cfg/ModelConfig.mk
UBOOT_DEFCONFIG := nvt-na51068_evb_nornand_defconfig#nvt-na51068_evb_nand_defconfig
UBOOT_DTS := $(HIBOOT_DIR)/cfg/nvt-na51068-$(DDR_SIZE)-evb

include $(UBOOT_DIR)/config.mk
ENVFLAGS 		:= -Wall $(PLATFORM_FLAGS) -I$(ENV_SRC_DIR) -I$(UBOOT_DIR)/include
ifeq ($(MTD_VERSION),old)
ENVFLAGS 		+= -DMTD_OLD
endif
HIBOOT_FLAGS 	:= $(MS_MAKE_JOBS) -C $(UBOOT_DIR) O="" ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE)  NVT_PRJCFG_MODEL_CFG=$(UBOOT_REG_CFG)

ifeq ($(DDR_SIZE),$(DDR_SIZE_512M))
UBOOT_DDR_SIZE	= (512*1024*1024)
else
UBOOT_DDR_SIZE	= (1024*1024*1024)
endif
UBOOT_CMDLINE	= earlyprintk console=ttyS0,115200 rootwait nprofile_irq_duration=on ubi.mtd=fs1 root=ubi0:rootfs rootfstype=ubifs rw init=/linuxrc

DTC_VER=$(shell ver=`dtc -v | awk -F' ' '{print $$NF}' | awk -F'-' '{print $$(NF-1)}'`; echo "$$ver" | sed 's/[^a-zA-Z0-9]//g')
DTC_CFLAGS=$(shell echo "-W no-unit_address_format -W no-unit_address_vs_reg")

define gen_dtb_from_dts
	which dtc &> /dev/null || sudo apt-get install device-tree-compiler -y; \
	cpp -nostdinc -D__DTS__ -I$(HIBOOT_DIR) -undef -x assembler-with-cpp $(UBOOT_DTS).dts > $(UBOOT_DTS).tmp.dts; \
	if [ "$(DTC_VER)" -le "141" ]; then \
		dtc -O dtb -b 0 -o $(UBOOT_DTS).dtb $(UBOOT_DTS).tmp.dts; \
	else \
		dtc $(DTC_CFLAGS) -O dtb -b 0 -o $(UBOOT_DTS).dtb $(UBOOT_DTS).tmp.dts; \
	fi;

endef

define gen_modelcfg_from_dtb
        if [ -f $(UBOOT_DTS).dtb ]; then \
		$(HIBOOT_DIR)/nvt-tools/nvt-ld-op --modelcfg-dtb=$(UBOOT_DTS).dtb --modelcfg-dst=$(UBOOT_REG_CFG); \
	else \
		echo "Can't find $(UBOOT_DTS).dtb"; \
	fi;
endef

define clean_dtb
	rm -rf $(UBOOT_DTS).tmp.dts $(UBOOT_DTS).dtb;
endef

hiboot_config:
	$(MS_MAKEFILE_V)echo "##### u-boot Menuconfig #####"
	$(MS_MAKEFILE_V)cp $(UBOOT_DIR)/configs/$(UBOOT_DEFCONFIG) $(UBOOT_DIR)/.config
	$(MS_MAKEFILE_V)$(MAKE) $(HIBOOT_FLAGS) menuconfig
	$(MS_MAKEFILE_V)cp $(UBOOT_DIR)/.config $(UBOOT_DIR)/configs/$(UBOOT_DEFCONFIG)

hiboot-prepare:
	$(MS_MAKEFILE_V)echo "Configure total memory=$(UBOOT_DDR_SIZE) version=$(HIBOOT_VERSION)"
	$(MS_MAKEFILE_V)echo "Configure cmdline '$(UBOOT_CMDLINE)'"
	@sed -i -e "s,\#define MS_UBOOT_VERSION.*,\#define MS_UBOOT_VERSION\t\"$(HIBOOT_VERSION)\"," $(UBOOT_VER_FILE)
	@sed -i -e "s,\#define MS_DDR_VERSION.*,\#define MS_DDR_VERSION\t$(VERSION_DDR)," $(UBOOT_VER_FILE)
	@sed -i -e "s%\#define CONFIG_BOOTARGS.*%\#define CONFIG_BOOTARGS\t\t\"$(UBOOT_CMDLINE)\"%" $(UBOOT_VER_FILE)
	@sed -i -e "s,\#define CFG_DDR_SIZE.*,\#define CFG_DDR_SIZE\t\t$(UBOOT_DDR_SIZE)," $(UBOOT_VER_FILE)
	$(call gen_dtb_from_dts)
	$(call gen_modelcfg_from_dtb)

hiboot: hiboot-prepare
	$(MS_MAKEFILE_V)$(MAKE) $(HIBOOT_FLAGS) distclean
	$(MS_MAKEFILE_V)$(MAKE) $(HIBOOT_FLAGS) $(UBOOT_DEFCONFIG)
	$(MS_MAKEFILE_V)$(MAKE) $(HIBOOT_FLAGS) all
	#$(MS_MAKEFILE_V)$(MAKE) $(HIBOOT_FLAGS) env
	#$(MS_MAKEFILE_V)$(MAKE) $(HIBOOT_FLAGS) examples
	@$(HIBOOT_DIR)/nvt-tools/encrypt_bin SUM $(UBOOT_DIR)/u-boot.bin 0x350 ub51068
	@$(HIBOOT_DIR)/nvt-tools/bfc c lz $(UBOOT_DIR)/u-boot.bin $(UBOOT_DIR)/u-boot.lz.bin 0 0
	@$(HIBOOT_DIR)/nvt-tools/mkboot.sh $(HIBOOT_DIR)/cfg/LD98323A.bin $(UBOOT_DTS).dtb $(UBOOT_DIR)/u-boot.lz.bin $(FIRMWARE_IMG_DIR)/uboot_$(DDR_NAME)

hiboot-clean:
	-$(MS_MAKEFILE_V)rm -rf $(UBOOT_DIR)/u-boot.bin $(UBOOT_DIR)/u-boot.lz.bin #$(FIRMWARE_IMG_DIR)/uboot_$(DDR_NAME)
	$(MS_MAKEFILE_V)$(MAKE) $(HIBOOT_FLAGS) distclean
	$(call clean_dtb)
	

envtool:
ifeq ($(wildcard $(PUBLIC_STATIC_LIB_DIR)/libarm-env.a),)
	$(MS_MAKEFILE_V)$(MAKE) $(HIBOOT_FLAGS) $(UBOOT_DEFCONFIG)
	$(MS_MAKEFILE_V)$(MAKE) $(HIBOOT_FLAGS) env
	$(MS_MAKEFILE_V)cp -dpRf $(ENV_TOOLS_DIR)/lib.a $(PUBLIC_STATIC_LIB_DIR)/libarm-env.a
	#$(MS_MAKEFILE_V)cp -dpRf $(ENV_TOOLS_DIR)/fw_printenv $(PUBLIC_APP_DIR)/
	#$(MS_MAKEFILE_V)cd $(PUBLIC_APP_DIR) && ln -sf fw_printenv fw_setenv && cd -
	$(MS_MAKEFILE_V)$(MAKE) $(HIBOOT_FLAGS) env clean
endif

envtool-clean:
	#-$(MS_MAKEFILE_V)rm -rf $(PUBLIC_STATIC_LIB_DIR)/libarm-env.a // keep 
	-$(MS_MAKEFILE_V)rm -rf $(PUBLIC_APP_DIR)/fw_printenv $(PUBLIC_APP_DIR)/fw_setenv
