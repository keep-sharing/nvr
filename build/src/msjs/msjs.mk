######################################################################
## Filename:      cpu_load.mk
## Author:        eric.milesight
## Created at:    2014-09-15
##                
## Description:   编译 msjs 的全局配置
## Copyright (C)  milesight
##                
######################################################################
#源码目录
MSJS_SRC_DIR    	:= $(SRC_DIR)/msjs

#输出目录
MSJS_TARGET_DIR 	:= $(TEMP_TARGET_DIR)/msjs

#目标名称
MSJS_TARGET_NAME	:= msjs

MSJS_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(MSJS_SRC_DIR) CC=$(TARGET_CC) CXX=$(TARGET_CXX) AR=$(TARGET_AR) TARGET_DIR=$(MSJS_TARGET_DIR) TARGET_NAME=$(MSJS_TARGET_NAME) INSTALL_DIR=$(PUBLIC_APP_DIR)

msjs:
	$(MS_MAKEFILE_V)$(MAKE) $(MSJS_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_APP_DIR)/$(MSJS_TARGET_NAME)
	
msjs-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(MSJS_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
APPS   += msjs
CLEANS += msjs-clean


