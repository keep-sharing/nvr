######################################################################
## Filename:      osa.mk
## Author:        bruce.milesight
## Created at:    2015-10-29
##                
## Description:   编译 osa lib
##
## Copyright (C)  milesight
##                
######################################################################
#源码目录
MSOSA_LIB_SRC_DIR			:= $(SRC_DIR)/libs/osa
#输出目录
MSOSA_LIB_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/osa
#目标名称
MSOSA_LIB_TARGET_NAME	:= libosa.so

MSOSA_LIB_COMPILE_OPT 	:= $(MS_MAKE_JOBS) -C $(MSOSA_LIB_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(MSOSA_LIB_TARGET_DIR) TARGET_NAME=$(MSOSA_LIB_TARGET_NAME) INSTALL_DIR=$(PUBLIC_LIB_DIR)

osa:
	$(MS_MAKEFILE_V)$(MAKE) $(MSOSA_LIB_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_LIB_DIR)/$(MSOSA_LIB_TARGET_NAME)
	
osa-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(MSOSA_LIB_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
LIBS   += osa
LIBSCLEAN += osa-clean
CLEANS += osa-clean


