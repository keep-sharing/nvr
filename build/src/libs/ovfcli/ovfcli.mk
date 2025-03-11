######################################################################
## Filename:      ovfcli.mk
## Author:        bruce.milesight
## Created at:    2015-11-06
##                
## Description:   编译 ovfcli lib
##
## Copyright (C)  milesight
##                
######################################################################
#源码目录
OVFCLI_LIB_SRC_DIR	:= $(SRC_DIR)/libs/ovfcli
#输出目录
OVFCLI_LIB_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/ovfcli
#目标名称
OVFCLI_LIB_TARGET_NAME	:= libovfcli.so

OVFCLI_LIB_COMPILE_OPT 	:= $(MS_MAKE_JOBS) -C $(OVFCLI_LIB_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(OVFCLI_LIB_TARGET_DIR) TARGET_NAME=$(OVFCLI_LIB_TARGET_NAME) INSTALL_DIR=$(PUBLIC_LIB_DIR)

ovfcli:
	$(MS_MAKEFILE_V)$(MAKE) $(OVFCLI_LIB_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_LIB_DIR)/$(OVFCLI_LIB_TARGET_NAME)
	
ovfcli-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(OVFCLI_LIB_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
LIBS   += ovfcli
LIBSCLEAN += ovfcli-clean
CLEANS += ovfcli-clean


