######################################################################
## Filename:      avilib.mk
## Author:        bruce.milesight
## Created at:    2015-11-06
##                
## Description:   编译 avilib lib
##
## Copyright (C)  milesight
##                
######################################################################
#源码目录
AVILIB_LIB_SRC_DIR	:= $(SRC_DIR)/libs/avilib
#输出目录
AVILIB_LIB_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/avilib
#目标名称
AVILIB_LIB_TARGET_NAME	:= libavilib.so

AVILIB_LIB_COMPILE_OPT 	:= $(MS_MAKE_JOBS) -C $(AVILIB_LIB_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(AVILIB_LIB_TARGET_DIR) TARGET_NAME=$(AVILIB_LIB_TARGET_NAME) INSTALL_DIR=$(PUBLIC_LIB_DIR)

avilib:
	$(MS_MAKEFILE_V)$(MAKE) $(AVILIB_LIB_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_LIB_DIR)/$(AVILIB_LIB_TARGET_NAME)
	
avilib-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(AVILIB_LIB_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
LIBS   += avilib
LIBSCLEAN += avilib-clean
CLEANS += avilib-clean


