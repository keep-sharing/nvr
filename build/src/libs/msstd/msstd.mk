######################################################################
## Filename:      msstd.mk
## Author:        bruce.milesight
## Created at:    2015-11-06
##                
## Description:   编译 msstd lib
##
## Copyright (C)  milesight
##                
######################################################################
#源码目录
MSSTD_LIB_SRC_DIR	:= $(SRC_DIR)/libs/msstd
#输出目录
MSSTD_LIB_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/msstd
#目标名称
MSSTD_LIB_TARGET_NAME	:= libmsstd.so

MSSTD_LIB_COMPILE_OPT 	:= $(MS_MAKE_JOBS) -C $(MSSTD_LIB_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(MSSTD_LIB_TARGET_DIR) TARGET_NAME=$(MSSTD_LIB_TARGET_NAME) INSTALL_DIR=$(PUBLIC_LIB_DIR)

msstd:
	$(MS_MAKEFILE_V)$(MAKE) $(MSSTD_LIB_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_LIB_DIR)/$(MSSTD_LIB_TARGET_NAME)
	
msstd-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(MSSTD_LIB_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
LIBS   += msstd
LIBSCLEAN += msstd-clean
CLEANS += msstd-clean


