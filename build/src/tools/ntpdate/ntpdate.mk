######################################################################
## Filename:      ntpdate.mk
## Author:        david.milesight
## Created at:    2019-01-30
##                
## Description:   编译环境配置 ntpdate
## Copyright (C)  milesight
##                
######################################################################
#源码目录
MSSP_SRC_DIR    	:= $(TOOLS_DIR)/third-party/ntpdate
#输出目录
MSSP_TARGET_DIR 	:= $(TEMP_TARGET_DIR)/ntpdate
#目标名称
MSSP_TARGET_NAME	:= ntpdate

MSSP_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(MSSP_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(MSSP_TARGET_DIR) TARGET_NAME=$(MSSP_TARGET_NAME) INSTALL_DIR=$(PUBLIC_APP_DIR)

ntpdate:
	$(MS_MAKEFILE_V)$(MAKE) $(MSSP_COMPILE_OPT) all
	
ntpdate-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(MSSP_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
APPS   += ntpdate
CLEANS += ntpdate-clean


