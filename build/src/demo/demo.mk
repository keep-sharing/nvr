######################################################################
## Filename:      demo.mk
## Author:        zbing
## Created at:    2016-04-22
##                
## Description:   编译临时demo
## Copyright (C)  milesight
##                
######################################################################
#源码目录
DEMO_SRC_DIR    	:= $(SRC_DIR)/demo
#输出目录
DEMO_TARGET_DIR 	:= $(TEMP_TARGET_DIR)/demo
#目标名称
DEMO_TARGET_NAME	:= demo

DEMO_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(DEMO_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(DEMO_TARGET_DIR) TARGET_NAME=$(DEMO_TARGET_NAME) INSTALL_DIR=$(PUBLIC_APP_DIR)

demo:
	$(MS_MAKEFILE_V)$(MAKE) $(DEMO_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_APP_DIR)/$(DEMO_TARGET_NAME)
	
demo-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(DEMO_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
APPS   += demo
CLEANS += demo-clean


