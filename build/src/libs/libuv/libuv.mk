######################################################################
## Filename:      libuv.mk
## Author:        hrz
## Created at:    2019.07.03
##                
## Description:   编译 libuv
## Copyright (C)  milesight
##                
######################################################################
#源码目录
LIBUV_SRC_DIR		:= $(SRC_DIR)/libs/libuv/

#输出目录
LIBUV_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/libuv

#目标名称
LIBUV_TARGET_NAME	:= libuv

#libuv: libuv-clean
libuv:
	$(MS_MAKEFILE_V)printf "$(COMPILE_P) $(LIBUV_TARGET_NAME) $(NOCOLOR)\n"
	-$(MS_MAKEFILE_V)mkdir -p $(LIBUV_TARGET_DIR)
	cd $(LIBUV_SRC_DIR); ./configure CC=$(TARGET_CC) --prefix=$(LIBUV_TARGET_DIR) --host=$(LINUX_TOOLCHAIN_NAME)
	$(MAKE) -j8 -s -C  $(LIBUV_SRC_DIR)
	$(MS_MAKEFILE_V)$(MAKE) -s -C $(LIBUV_SRC_DIR) install
	$(MS_MAKEFILE_V)$(TARGET_STRIP) $(LIBUV_TARGET_DIR)/lib/$(LIBUV_TARGET_NAME).so*
	$(MS_MAKEFILE_V)cp -af $(LIBUV_TARGET_DIR)/lib/$(LIBUV_TARGET_NAME).so* $(TOP_DIR)/platform/$(PLATFORM_NAME)/rootfs/usr/lib
	$(MS_MAKEFILE_V)printf  "$(COMPILE_SUCCESS_P) $(LIBUV_TARGET_NAME) success $(NOCOLOR)\n"

libuv-clean:
	$(MS_MAKEFILE_V)printf "$(CLEAN_P) $(LIBUV_TARGET_NAME) $(NOCOLOR)\n"
	$(MS_MAKEFILE_V) $(MAKE) -C $(LIBUV_SRC_DIR) clean 
	#$(MS_MAKEFILE_V)rm -rf $(LIBUV_TARGET_DIR)/*
	#$(MS_MAKEFILE_V)rm -rf $(TOP_DIR)/platform/$(PLATFORM_NAME)/rootfs/usr/lib/$(LIBUV_TARGET_NAME).so*
	$(MS_MAKEFILE_V)printf  "$(CLEAN_SUCCESS_P) $(LIBUV_TARGET_NAME) success $(NOCOLOR)\n"

libuv-distclean: libuv-clean
	#$(MS_MAKEFILE_V)rm -rf $(TOP_DIR)/platform/$(PLATFORM_NAME)/rootfs/usr/lib/$(LIBUV_TARGET_NAME).so*
	$(MS_MAKEFILE_V)rm -rf $(LIBUV_TARGET_DIR)/*
	
#添加至全局LIBS/CLEANS
#LIBS	+= libuv
#CLEANS	+= libuv-clean

