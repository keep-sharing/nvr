######################################################################
## Filename:      update.mk
## Author:        bruce
## Created at:    2014-06-12
##                
## Description:   编译 update 的全局配置
## Copyright (C)  milesight
##                
######################################################################
#源码目录
UPDATE_SRC_DIR    := $(SRC_DIR)/update
#输出目录
UPDATE_TARGET_DIR := $(TEMP_TARGET_DIR)/update
#目标名称
UPDATE_TARGET_NAME:= msupdate

UPDATE_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(UPDATE_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(UPDATE_TARGET_DIR) TARGET_NAME=$(UPDATE_TARGET_NAME) INSTALL_DIR=$(PUBLIC_APP_DIR)

update:
	$(MS_MAKEFILE_V)$(MAKE) $(UPDATE_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_APP_DIR)/$(UPDATE_TARGET_NAME)
	
update-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(UPDATE_COMPILE_OPT) clean
	
#添加至全局LIBS/CLEANS
APPS   += update
CLEANS += update-clean


