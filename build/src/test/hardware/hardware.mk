######################################################################
## Filename:      hardware.mk
## Author:        zbing
## Created at:    2016-01-05
##                
## Description:   编译 hardware 的全局配置
## Copyright (C)  milesight
##                
######################################################################
#源码目录
HD_SRC_DIR    	:= $(SRC_DIR)/test/hardware
#输出目录
HD_TARGET_DIR 	:= $(TEMP_TARGET_DIR)/test/hardware
#目标名称
HD_TARGET_NAME	:= hardware

HD_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(HD_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(HD_TARGET_DIR) TARGET_NAME=$(HD_TARGET_NAME) INSTALL_DIR=$(PUBLIC_APP_DIR)

hardware:
	$(MS_MAKEFILE_V)$(MAKE) $(HD_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_APP_DIR)/$(HD_TARGET_NAME)
	
hardware-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(HD_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
APPS   += hardware
CLEANS += hardware-clean


