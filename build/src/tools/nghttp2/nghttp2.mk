######################################################################
## Filename:      libnghttp2.mk
## Author:        david
## Created at:    2019.11.15
##                
## Description:   编译 libnghttp2
## Copyright (C)  milesight
##                
######################################################################
#源码目录
NGHTTP2_SRC_DIR		:= $(TOOLS_DIR)/third-party/nghttp2

#输出目录
NGHTTP2_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/nghttp2

#目标名称
NGHTTP2_TARGET_NAME	:= libnghttp2

libnghttp2: 
	$(MS_MAKEFILE_V)printf "$(COMPILE_P) $(NGHTTP2_TARGET_NAME) $(NOCOLOR)\n"
	-$(MS_MAKEFILE_V)mkdir -p $(NGHTTP2_TARGET_DIR)
	cd $(NGHTTP2_SRC_DIR); ./configure CC=$(TARGET_CC) --prefix=$(NGHTTP2_TARGET_DIR) --host=$(LINUX_TOOLCHAIN_NAME) --disable-static --enable-lib-only
	$(MAKE) -j8 -s -C  $(NGHTTP2_SRC_DIR)
	$(MS_MAKEFILE_V)$(MAKE) -s -C $(NGHTTP2_SRC_DIR) install
	$(MS_MAKEFILE_V)$(TARGET_STRIP) $(NGHTTP2_TARGET_DIR)/lib/$(NGHTTP2_TARGET_NAME).so*
	$(MS_MAKEFILE_V)cp -af $(NGHTTP2_TARGET_DIR)/lib/$(NGHTTP2_TARGET_NAME).so* $(TOP_DIR)/targets/$(PLATFORM_NAME)/app/libs
	$(MS_MAKEFILE_V)printf  "$(COMPILE_SUCCESS_P) $(NGHTTP2_TARGET_NAME) success $(NOCOLOR)\n"

libnghttp2-clean:
	$(MS_MAKEFILE_V)printf "$(CLEAN_P) $(NGHTTP2_TARGET_NAME) $(NOCOLOR)\n"
	$(MS_MAKEFILE_V) $(MAKE) -C $(NGHTTP2_SRC_DIR) clean 
	$(MS_MAKEFILE_V)rm -rf $(NGHTTP2_TARGET_DIR)/*
	$(MS_MAKEFILE_V)rm -rf $(TOP_DIR)/targets/$(PLATFORM_NAME)/app/libs/$(NGHTTP2_TARGET_NAME).so*
	$(MS_MAKEFILE_V)printf  "$(CLEAN_SUCCESS_P) $(NGHTTP2_TARGET_NAME) success $(NOCOLOR)\n"

	
#添加至全局LIBS/CLEANS
#LIBS	+= libnghttp2
#CLEANS	+= libnghttp2-clean

