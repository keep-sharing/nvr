######################################################################
## Filename:      gsnap.mk
## Author:        huangwx
## Created at:    2016-03-14
##                
## Description:   编译 gsnap 的全局配置
## Copyright (C)  milesight
##                
######################################################################
#源码目录
GS_SRC_DIR      := $(SRC_DIR)/test/gsnap/$(PLATFORM_TYPE)
#输出目录
GS_TARGET_DIR   := $(TEMP_TARGET_DIR)/test/gsnap
#目标名称
GS_TARGET_NAME  := gsnap

GS_COMPILE_OPT  := $(MS_MAKE_JOBS) -C $(GS_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(GS_TARGET_DIR) TARGET_NAME=$(GS_TARGET_NAME) INSTALL_DIR=$(PUBLIC_APP_DIR)

gsnap:
	$(MS_MAKEFILE_V)$(MAKE) $(GS_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_APP_DIR)/$(GS_TARGET_NAME)

gsnap-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(GS_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
APPS   += gsnap
CLEANS += gsnap-clean
