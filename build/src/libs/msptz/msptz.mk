######################################################################
## Filename:      msptz.mk
## Author:        bruce.milesight
## Created at:    2015-11-06
##                
## Description:   编译 msptz lib
##
## Copyright (C)  milesight
##                
######################################################################
#源码目录
MSPTZ_LIB_SRC_DIR	:= $(SRC_DIR)/libs/msptz
#输出目录
MSPTZ_LIB_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/msptz
#目标名称
MSPTZ_LIB_TARGET_NAME	:= libmsptz.so

MSPTZ_LIB_COMPILE_OPT 	:= $(MS_MAKE_JOBS) -C $(MSPTZ_LIB_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(MSPTZ_LIB_TARGET_DIR) TARGET_NAME=$(MSPTZ_LIB_TARGET_NAME) INSTALL_DIR=$(PUBLIC_LIB_DIR)

msptz:
	$(MS_MAKEFILE_V)$(MAKE) $(MSPTZ_LIB_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_LIB_DIR)/$(MSPTZ_LIB_TARGET_NAME)
	
msptz-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(MSPTZ_LIB_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
LIBS   += msptz
LIBSCLEAN += msptz-clean
CLEANS += msptz-clean


