######################################################################
## Filename:      vapi.mk
## Author:        zbing
## Created at:    2015-10-20
##                
## Description:   编译 vapi lib
##
## Copyright (C)  milesight
##                
######################################################################
#源码目录
MSVAPI_LIB_SRC_DIR			:= $(SRC_DIR)/libs/vapi/$(PLATFORM_TYPE)
#输出目录
MSVAPI_LIB_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/vapi
#目标名称
MSVAPI_LIB_TARGET_NAME	:= libvapi.so

MSVAPI_LIB_COMPILE_OPT 	:= $(MS_MAKE_JOBS) -C $(MSVAPI_LIB_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(MSVAPI_LIB_TARGET_DIR) TARGET_NAME=$(MSVAPI_LIB_TARGET_NAME) INSTALL_DIR=$(PUBLIC_LIB_DIR)

libvapi:
	$(MS_MAKEFILE_V)$(MAKE) $(MSVAPI_LIB_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_LIB_DIR)/$(MSVAPI_LIB_TARGET_NAME)
	
libvapi-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(MSVAPI_LIB_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
LIBS   += libvapi
LIBSCLEAN += libvapi-clean
CLEANS += libvapi-clean


