######################################################################
## Filename:      webcgi.mk
## Author:        bruce.milesight
## Created at:    2015-11-20 11:43:00
##
## Description:   编译web的全局配置
##                1.定义源码目录
##                2.定义输出目录
##                3.定义目标名称
## Copyright (C)  milesight
##
######################################################################
#源码目录
WEBCGI_SRC_DIR    := $(SRC_DIR)/web/webcgi
#输出目录
WEBCGI_TARGET_DIR := $(TEMP_TARGET_DIR)/web/webcgi
#目标名称
WEBCGI_TARGET_NAME:= main

WEBCGI_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(WEBCGI_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(WEBCGI_TARGET_DIR) TARGET_NAME=$(WEBCGI_TARGET_NAME) INSTALL_DIR=$(PUBLIC_APP_DIR)

webcgi:
	$(MS_MAKEFILE_V)$(MAKE) $(WEBCGI_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_APP_DIR)/$(WEBCGI_TARGET_NAME)

webcgi-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(WEBCGI_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
APPS   += webcgi
CLEANS += webcgi-clean
