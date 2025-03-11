######################################################################
## Filename:      mssp.mk
## Author:        bruce.milesight
## Created at:    2015-11-13
##                
## Description:   编译环境配置 mssp
## Copyright (C)  milesight
##                
######################################################################
#源码目录
MSSP_SRC_DIR    	:= $(SRC_DIR)/mssp
#输出目录
MSSP_TARGET_DIR 	:= $(TEMP_TARGET_DIR)/mssp
#目标名称
MSSP_TARGET_NAME	:= mssp

MSSP_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(MSSP_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(MSSP_TARGET_DIR) TARGET_NAME=$(MSSP_TARGET_NAME) INSTALL_DIR=$(PUBLIC_APP_DIR)

mssp:
	$(MS_MAKEFILE_V)$(MAKE) $(MSSP_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_APP_DIR)/$(MSSP_TARGET_NAME)
	
mssp-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(MSSP_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
APPS   += mssp
CLEANS += mssp-clean


