######################################################################
## Filename:      msini.mk
## Author:        zbing.milesight
## Created at:    2019-05-23
##                
## Description:   编译 msini lib
##
## Copyright (C)  milesight
##                
######################################################################
#源码目录
MSINI_LIB_SRC_DIR	:= $(SRC_DIR)/libs/msini
#输出目录
MSINI_LIB_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/msini
#目标名称
MSINI_LIB_TARGET_NAME	:= libmsini.so

MSINI_LIB_COMPILE_OPT 	:= $(MS_MAKE_JOBS) -C $(MSINI_LIB_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(MSINI_LIB_TARGET_DIR) TARGET_NAME=$(MSINI_LIB_TARGET_NAME) INSTALL_DIR=$(PUBLIC_LIB_DIR)

libmsini:
	$(MS_MAKEFILE_V)$(MAKE) $(MSINI_LIB_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_LIB_DIR)/$(MSINI_LIB_TARGET_NAME)
	
libmsini-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(MSINI_LIB_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
LIBS   += libmsini
LIBSCLEAN += libmsini-clean
CLEANS += libmsini-clean


