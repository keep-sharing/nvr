######################################################################
## Filename:      cpu_load.mk
## Author:        eric.milesight
## Created at:    2014-09-15
##                
## Description:   编译 armloading 的全局配置
## Copyright (C)  milesight
##                
######################################################################
#源码目录
ARMLOAD_SRC_DIR    	:= $(SRC_DIR)/test/armloading

#输出目录
ARMLOAD_TARGET_DIR 	:= $(TEMP_TARGET_DIR)/test/armloading

#目标名称
ARMLOAD_TARGET_NAME	:= arm_loading

ARMLOAD_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(ARMLOAD_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(ARMLOAD_TARGET_DIR) TARGET_NAME=$(ARMLOAD_TARGET_NAME) INSTALL_DIR=$(PUBLIC_APP_DIR)

armloading:
	$(MS_MAKEFILE_V)$(MAKE) $(ARMLOAD_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_APP_DIR)/$(ARMLOAD_TARGET_NAME)
	
armloading-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(ARMLOAD_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
APPS   += armloading
CLEANS += armloading-clean


