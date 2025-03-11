######################################################################
## Filename:      poe.mk
## Author:        zbing.milesight
## Created at:    2016-08-25
##                
## Description:   编译 poe lib
##
## Copyright (C)  milesight
##                
######################################################################
#源码目录
POE_LIB_SRC_DIR	:= $(SRC_DIR)/libs/poe#/$(PLATFORM_TYPE)
#输出目录
POE_LIB_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/poe
#目标名称
POE_LIB_TARGET_NAME	:= libpoe.so

POE_LIB_COMPILE_OPT 	:= $(MS_MAKE_JOBS) -C $(POE_LIB_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(POE_LIB_TARGET_DIR) TARGET_NAME=$(POE_LIB_TARGET_NAME) INSTALL_DIR=$(PUBLIC_LIB_DIR)

libpoe:
	$(MS_MAKEFILE_V)$(MAKE) $(POE_LIB_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_LIB_DIR)/$(POE_LIB_TARGET_NAME)
	
libpoe-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(POE_LIB_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
LIBS   += libpoe
LIBSCLEAN += libpoe-clean
CLEANS += libpoe-clean


