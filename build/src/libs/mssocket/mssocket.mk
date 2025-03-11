######################################################################
## Filename:      mssocket.mk
## Author:        bruce.milesight
## Created at:    2015-11-06
##                
## Description:   编译 mssocket lib
##
## Copyright (C)  milesight
##                
######################################################################
#源码目录
MSSOCKET_LIB_SRC_DIR		:= $(SRC_DIR)/libs/mssocket
#输出目录
MSSOCKET_LIB_TARGET_DIR		:= $(TEMP_TARGET_DIR)/libs/mssocket
#目标名称
MSSOCKET_LIB_TARGET_NAME	:= libmssocket.so

MSSOCKET_LIB_COMPILE_OPT 	:= $(MS_MAKE_JOBS) -C $(MSSOCKET_LIB_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(MSSOCKET_LIB_TARGET_DIR) TARGET_NAME=$(MSSOCKET_LIB_TARGET_NAME) INSTALL_DIR=$(PUBLIC_LIB_DIR)

mssocket:
	$(MS_MAKEFILE_V)$(MAKE) $(MSSOCKET_LIB_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_LIB_DIR)/$(MSSOCKET_LIB_TARGET_NAME)
	
mssocket-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(MSSOCKET_LIB_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
LIBS   += mssocket
LIBSCLEAN += mssocket-clean
CLEANS += mssocket-clean


