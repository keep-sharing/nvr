######################################################################
## Filename:      msmov.mk
## Author:        david.milesight
## Created at:    2020-02-26
##                
## Description:   编译 mov lib
##
## Copyright (C)  milesight
##                
######################################################################
#源码目录
MSMOV_LIB_SRC_DIR	:= $(SRC_DIR)/libs/msmov
#输出目录
MSMOV_LIB_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/msmov
#目标名称
MSMOV_LIB_TARGET_NAME	:= libmov.so

MSMOV_LIB_COMPILE_OPT 	:= $(MS_MAKE_JOBS) -C $(MSMOV_LIB_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(MSMOV_LIB_TARGET_DIR) TARGET_NAME=$(MSMOV_LIB_TARGET_NAME) INSTALL_DIR=$(PUBLIC_LIB_DIR)

msmov:
	$(MS_MAKEFILE_V)$(MAKE) $(MSMOV_LIB_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_LIB_DIR)/$(MSMOV_LIB_TARGET_NAME)
	
msmov-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(MSMOV_LIB_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
LIBS   += msmov
LIBSCLEAN += msmov-clean
CLEANS += msmov-clean


