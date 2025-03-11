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
UBOOT_DIR 		= $(HIBOOT_DIR)/u-boot-2010.06
ENV_SRC_DIR 	:= $(FW_TOPDIR)/env
ENV_SRCS		:= $(ENV_SRC_DIR)/fw_env.c $(ENV_SRC_DIR)/crc32.c
ENV_OBJS 		:= $(ENV_SRC_DIR)/fw_env.o $(ENV_SRC_DIR)/crc32.o
ALL_SRCS		:= $(ENV_SRC_DIR)/crc32.c  $(ENV_SRC_DIR)/fw_env.c $(ENV_SRC_DIR)/fw_env_main.c
UBOOT_VER_FILE	:= $(UBOOT_DIR)/include/configs/hi3536.h
UBOOT_REG_CFG	:= reg_info_$(DDR_NAME).bin

include $(UBOOT_DIR)/config.mk
ENVFLAGS 		:= -Wall -s $(PLATFORM_FLAGS) -DUSE_HOSTCC -I$(UBOOT_DIR)/include -I$(ENV_SRC_DIR)
ifeq ($(MTD_VERSION),old)
ENVFLAGS 		+= -DMTD_OLD
endif
HIBOOT_FLAGS 	:= $(MS_MAKE_JOBS) -C $(UBOOT_DIR) O=$(UBOOT_DIR) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) 

ifeq ($(DDR_SIZE),$(DDR_SIZE_2G))
UBOOT_DDR_SIZE	= (2*1024*1024*1024)
else
UBOOT_DDR_SIZE	= (3*1024*1024*1024)
endif
UBOOT_CMDLINE	= mem=$(ARM_DDR_SIZE) console=ttyAMA0,115200 ubi.mtd=fs1 root=ubi0:rootfs rw rootfstype=ubifs vmalloc=1024M


hiboot-prepare:
	$(MS_MAKEFILE_V)echo "Configure total memory=$(DDR_SIZE) version=$(HIBOOT_VERSION)"
	$(MS_MAKEFILE_V)echo "Configure cmdline '$(UBOOT_CMDLINE)'"
	@sed -i -e "s,\#define MS_UBOOT_VERSION.*,\#define MS_UBOOT_VERSION\t\"$(HIBOOT_VERSION)\"," $(UBOOT_VER_FILE)
	@sed -i -e "s,\#define MS_DDR_VERSION.*,\#define MS_DDR_VERSION\t$(VERSION_DDR)," $(UBOOT_VER_FILE)
	@sed -i -e "s%\#define CONFIG_BOOTARGS.*%\#define CONFIG_BOOTARGS\t\t\"$(UBOOT_CMDLINE)\"%" $(UBOOT_VER_FILE)
	@sed -i -e "s,\#define CFG_DDR_SIZE.*,\#define CFG_DDR_SIZE\t\t$(UBOOT_DDR_SIZE)," $(UBOOT_VER_FILE)

hiboot-config:
	$(MS_MAKEFILE_V)$(MAKE) $(HIBOOT_FLAGS) distclean
	$(MS_MAKEFILE_V)cp $(HIBOOT_DIR)/mkboot.sh $(UBOOT_DIR)
	$(MS_MAKEFILE_V)$(MAKE) $(HIBOOT_FLAGS) $(CHIP_MODEL)_config

hiboot: hiboot-prepare
	$(MS_MAKEFILE_V)if ! test -f $(UBOOT_DIR)/mkboot.sh; then \
		$(MAKE) hiboot-config; \
	fi;
	$(MS_MAKEFILE_V)cp $(HIBOOT_DIR)/$(UBOOT_REG_CFG) $(UBOOT_DIR)/reg_info.bin
	$(MS_MAKEFILE_V)$(MAKE) $(HIBOOT_FLAGS) 
	$(MS_MAKEFILE_V)cp $(UBOOT_DIR)/ms_uboot.bin $(FIRMWARE_IMG_DIR)/uboot_$(DDR_NAME)

hiboot-clean:
	-$(MS_MAKEFILE_V)rm -rf $(UBOOT_DIR)/mkboot.sh $(UBOOT_DIR)/reg_info*.bin
	$(MS_MAKEFILE_V)$(MAKE) $(HIBOOT_FLAGS) distclean
	

$(ENV_SRC_DIR)/crc32.c:
	#$(MS_MAKEFILE_V)cp -f $(UBOOT_DIR)/lib/crc32.c $(ENV_SRC_DIR)/crc32.c
	$(MS_MAKEFILE_V)ln -sf $(UBOOT_DIR)/lib/crc32.c $(ENV_SRC_DIR)/crc32.c

$(ENV_SRC_DIR)/crc32.o : $(ENV_SRC_DIR)/crc32.c
	$(CROSS_COMPILE)gcc $(ENVFLAGS) -c $< -o $@
		
$(ENV_SRC_DIR)/fw_env.o : $(ENV_SRC_DIR)/fw_env.c 
	$(CROSS_COMPILE)gcc $(ENVFLAGS) -c $< -o $@

fw_env: $(ENV_OBJS)
	$(CROSS_COMPILE)gcc $(ENVFLAGS) $(ALL_SRCS) -o $(ENV_SRC_DIR)/fw_printenv	
	$(MS_MAKEFILE_V)cp -dpRf $(ENV_SRC_DIR)/fw_printenv $(PUBLIC_APP_DIR)/
	$(MS_MAKEFILE_V)cd $(PUBLIC_APP_DIR) && ln -sf fw_printenv fw_setenv
	
fw_env-clean:
	-$(MS_MAKEFILE_V)rm -rf $(ENV_SRC_DIR)/fw_printenv $(ENV_SRC_DIR)/fw_setenv $(ENV_SRC_DIR)/crc32.c
	-$(MS_MAKEFILE_V)rm -rf $(ENV_OBJS) $(ENV_SRC_DIR)/*.o
	-$(MS_MAKEFILE_V)rm -rf $(PUBLIC_APP_DIR)/fw_printenv $(PUBLIC_APP_DIR)/fw_setenv
	
