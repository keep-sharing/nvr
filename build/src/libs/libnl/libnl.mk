######################################################################
## Filename:      libnl.mk
## Author:        hrz
## Created at:    2019.07.02
##                
## Description:   编译 libnl
## Copyright (C)  milesight
##                
######################################################################
#源码目录
LIBNL_SRC_DIR		:= $(SRC_DIR)/libs/libnl/

#输出目录
LIBNL_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/libnl

#目标名称
LIBNL_TARGET_NAME	:= libnl

#libnl: libnl-clean
libnl:
	$(MS_MAKEFILE_V)printf "$(COMPILE_P) $(LIBNL_TARGET_NAME) $(NOCOLOR)\n"
	-$(MS_MAKEFILE_V)mkdir -p $(LIBNL_TARGET_DIR)
	cd $(LIBNL_SRC_DIR); ./configure CC=$(TARGET_CC) --prefix=$(LIBNL_TARGET_DIR) --host=$(LINUX_TOOLCHAIN_NAME)
	$(MAKE) -j8 -s -C  $(LIBNL_SRC_DIR)
	$(MS_MAKEFILE_V)$(MAKE) -s -C $(LIBNL_SRC_DIR) install
	$(MS_MAKEFILE_V)$(TARGET_STRIP) $(LIBNL_TARGET_DIR)/lib/$(LIBNL_TARGET_NAME)*
	$(MS_MAKEFILE_V)cp -af $(LIBNL_TARGET_DIR)/lib/$(LIBNL_TARGET_NAME)* $(PUBLIC_LIB_DIR)
	$(MS_MAKEFILE_V)printf  "$(COMPILE_SUCCESS_P) $(LIBNL_TARGET_NAME) success $(NOCOLOR)\n"

libnl-clean:
	$(MS_MAKEFILE_V)printf "$(CLEAN_P) $(LIBNL_TARGET_NAME) $(NOCOLOR)\n"
	$(MS_MAKEFILE_V) $(MAKE) -C $(LIBNL_SRC_DIR) clean  
	#$(MS_MAKEFILE_V)rm -rf $(LIBNL_TARGET_DIR)
	#$(MS_MAKEFILE_V)rm -rf $(PUBLIC_LIB_DIR)/$(LIBNL_TARGET_NAME)*
	$(MS_MAKEFILE_V)printf  "$(CLEAN_SUCCESS_P) $(LIBNL_TARGET_NAME) success $(NOCOLOR)\n"

libnl-distclean: libnl-clean
	$(MS_MAKEFILE_V)rm -rf $(PUBLIC_LIB_DIR)/$(LIBNL_TARGET_NAME)*
	$(MS_MAKEFILE_V)rm -rf $(LIBNL_TARGET_DIR)/*
	
#添加至全局LIBS/CLEANS
#LIBS	+= libnl
#CLEANS	+= libnl-clean

