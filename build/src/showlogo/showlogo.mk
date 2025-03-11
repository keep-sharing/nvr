######################################################################
## Filename:      showlogo.mk
## Author:        zbing
## Created at:    2015-12-15
##                
## Description:   编译 showlogo
##
## Copyright (C)  milesight
##                
######################################################################
#源码目录
SHOWLOGO_SRC_DIR			:= $(SRC_DIR)/showlogo/$(PLATFORM_TYPE)
#输出目录
SHOWLOGO_TARGET_DIR	:= $(TEMP_TARGET_DIR)/showlogo
#目标名称
SHOWLOGO_TARGET_NAME	:= showlogo

SHOWLOGO_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(SHOWLOGO_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(SHOWLOGO_TARGET_DIR) TARGET_NAME=$(SHOWLOGO_TARGET_NAME) INSTALL_DIR=$(PUBLIC_APP_DIR)

showlogo:
	$(MS_MAKEFILE_V)$(MAKE) $(SHOWLOGO_COMPILE_OPT) all
	
showlogo-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(SHOWLOGO_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
APPS   += showlogo
CLEANS += showlogo-clean


