######################################################################
## Filename:      upnp.mk
## Author:        eirc
## Created at:    2016.05.29
##                
## Description:   编译 upnp
## Copyright (C)  milesight
##                
######################################################################
#源码目录
UPNP_SRC_DIR		:= $(SRC_DIR)/libs/libupnp-1.6.19

#输出目录
UPNP_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/upnp

#目标名称
UPNP_TARGET_NAME	:= libupnp

upnp: upnp-clean
	$(MS_MAKEFILE_V)printf "$(COMPILE_P) $(UPNP_TARGET_NAME) $(NOCOLOR)\n"
	-$(MS_MAKEFILE_V)mkdir -p $(UPNP_TARGET_DIR)
	cd $(UPNP_SRC_DIR); ./configure CC=$(TARGET_CC) CFLAGS='-O3 $(PLATFORM_FLAGS)' --prefix=$(UPNP_TARGET_DIR) --host=$(LINUX_TOOLCHAIN_NAME) --disable-dependency-tracking --disable-float-api --disable-vbr  --enable-fixed-point --with-pic --enable-static --disable-shared
	$(MS_MAKEFILE_V)$(MAKE) -j8 -s -C $(UPNP_SRC_DIR) 
	$(MS_MAKEFILE_V)$(MAKE) -s -C $(UPNP_SRC_DIR) install
	$(MS_MAKEFILE_V)cp -av $(UPNP_TARGET_DIR)/lib/$(UPNP_TARGET_NAME)*.a $(PUBLIC_STATIC_LIB_DIR)
	#$(MS_MAKEFILE_V)$(TARGET_STRIP) $(UPNP_TARGET_DIR)/lib/$(UPNP_TARGET_NAME)*.so*
	#$(MS_MAKEFILE_V)cp -a $(UPNP_TARGET_DIR)/lib/$(UPNP_TARGET_NAME).so.* $(PUBLIC_LIB_DIR)/
	$(MS_MAKEFILE_V)printf  "$(COMPILE_SUCCESS_P) $(UPNP_TARGET_NAME) success $(NOCOLOR)\n"

upnp-clean:
	$(MS_MAKEFILE_V)printf "$(CLEAN_P) $(UPNP_TARGET_NAME) $(NOCOLOR)\n"
	$(MS_MAKEFILE_V)if test -f $(UPNP_SRC_DIR)/Makefile; then $(MAKE) -s -j8 -C $(UPNP_SRC_DIR) distclean ; fi
	-$(MS_MAKEFILE_V)rm -rf $(UPNP_TARGET_DIR)
	$(MS_MAKEFILE_V)printf  "$(CLEAN_SUCCESS_P) $(UPNP_TARGET_NAME) success $(NOCOLOR)\n"

upnp-distclean: upnp-clean
	-$(MS_MAKEFILE_V)rm -rf $(PUBLIC_LIB_DIR)/$(UPNP_TARGET_NAME)*
	-$(MS_MAKEFILE_V)rm -rf $(UPNP_TARGET_DIR)/* $(PUBLIC_STATIC_LIB_DIR)/$(UPNP_TARGET_NAME)*.a
	
#添加至全局LIBS/CLEANS
PREBUILD_LIBS	+= upnp
PREBUILD_CLEANS	+= upnp-clean


