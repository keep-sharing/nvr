######################################################################
## Filename:      pppoe.mk
## Author:        hrz
## Created at:    2019.09.25
##                
## Description:   编译 rp-pppoe-3.8
## Copyright (C)  milesight
##                
######################################################################
#源码目录
PPPOE_SRC_DIR		:= $(SRC_DIR)/rp-pppoe/src

#输出目录
PPPOE_TARGET_DIR	:= $(TEMP_TARGET_DIR)/rp-pppoe

#目标名称
PPPOE_TARGET_NAME	:= pppoe

#pppoe: pppoe-clean
pppoe:
	$(MS_MAKEFILE_V)printf "$(COMPILE_P) $(PPPOE_TARGET_NAME) $(NOCOLOR)\n"
	-$(MS_MAKEFILE_V)mkdir -p $(PPPOE_TARGET_DIR)
	cd $(PPPOE_SRC_DIR); ./configure --prefix=$(PPPOE_TARGET_DIR) --host=arm-linux#$(LINUX_TOOLCHAIN_NAME)
	sed -i 's/ar -rc/$(LINUX_TOOLCHAIN_NAME)-ar -rc/g' $(PPPOE_SRC_DIR)/Makefile
	sed -i 's/gcc/$(LINUX_TOOLCHAIN_NAME)-gcc/g' $(PPPOE_SRC_DIR)/Makefile
	sed -i 's/AR=ar/AR=$(LINUX_TOOLCHAIN_NAME)-ar/g' $(PPPOE_SRC_DIR)/libevent/Makefile
	sed -i 's/gcc/$(LINUX_TOOLCHAIN_NAME)-gcc/g' $(PPPOE_SRC_DIR)/libevent/Makefile
	$(MS_MAKEFILE_V)$(MAKE) -j8 -s CC=$(TARGET_CC) -C $(PPPOE_SRC_DIR) all
	$(MS_MAKEFILE_V)$(MAKE) -s -C $(PPPOE_SRC_DIR) install
	$(MS_MAKEFILE_V)$(TARGET_STRIP) $(PPPOE_TARGET_DIR)/sbin/$(PPPOE_TARGET_NAME)
	$(MS_MAKEFILE_V)cp -af $(PPPOE_TARGET_DIR)/sbin/$(PPPOE_TARGET_NAME) $(PUBLIC_APP_DIR)
	$(MS_MAKEFILE_V)printf  "$(COMPILE_SUCCESS_P) $(PPPOE_TARGET_NAME) success $(NOCOLOR)\n"

pppoe-clean:
	$(MS_MAKEFILE_V)printf "$(CLEAN_P) $(PPPOE_TARGET_NAME) $(NOCOLOR)\n"
	$(MS_MAKEFILE_V) $(MAKE) -C $(PPPOE_SRC_DIR) clean  
	$(MS_MAKEFILE_V)rm -rf $(PPPOE_TARGET_DIR)
	$(MS_MAKEFILE_V)rm -rf $(PUBLIC_APP_DIR)/$(PPPOE_TARGET_NAME)
	$(MS_MAKEFILE_V)printf  "$(CLEAN_SUCCESS_P) $(PPPOE_TARGET_NAME) success $(NOCOLOR)\n"

pppoe-distclean: pppoe-clean
	-$(MS_MAKEFILE_V)rm -rf $(PUBLIC_LIB_DIR)/$(PPPOE_TARGET_NAME)*
	-$(MS_MAKEFILE_V)rm -rf $(PPPOE_TARGET_DIR)/*
	
#添加至全局LIBS/CLEANS
#APPS   += pppoe
#CLEANS += pppoe-clean

