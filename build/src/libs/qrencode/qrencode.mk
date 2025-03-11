######################################################################
## Filename:      qrencode.mk
## Author:        david.milesight
## Created at:    2016-12-20 11:43:00
##
## Description:   编译qrencode的全局配置

## Copyright (C)  milesight
##
######################################################################
#源码目录
QRENCODE_SRC_DIR    := $(SRC_DIR)/libs/qrencode
#输出目录
QRENCODE_TARGET_DIR := $(TEMP_TARGET_DIR)/libs/qrencode
#目标名称
QRENCODE_TARGET_NAME:= libqrencode.so

QRENCODE_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(QRENCODE_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(QRENCODE_TARGET_DIR) TARGET_NAME=$(QRENCODE_TARGET_NAME) INSTALL_DIR=$(PUBLIC_LIB_DIR)

libqrencode:
	$(MS_MAKEFILE_V)$(MAKE) $(QRENCODE_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_LIB_DIR)/$(QRENCODE_TARGET_NAME)

libqrencode-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(QRENCODE_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
LIBS   += libqrencode
LIBSCLEAN += libqrencode-clean
CLEANS += libqrencode-clean
