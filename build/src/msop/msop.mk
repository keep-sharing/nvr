######################################################################
## Filename:      msop.mk
## Author:        hrz
## Created at:    2022-05-06
##                
## Description:   编译 msop 的全局配置
## Copyright (C)  milesight
##                
######################################################################
#源码目录
MSOP_SRC_DIR    	:= $(SRC_DIR)/msop
#输出目录
MSOP_TARGET_DIR 	:= $(TEMP_TARGET_DIR)/msop
#目标名称
MSOP_TARGET_NAME	:= msop

MSOP_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(MSOP_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(MSOP_TARGET_DIR) TARGET_NAME=$(MSOP_TARGET_NAME) INSTALL_DIR=$(PUBLIC_APP_DIR)

msop: msop-clean
	$(MS_MAKEFILE_V)$(MAKE) $(MSOP_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_APP_DIR)/$(MSOP_TARGET_NAME)
	
msop-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(MSOP_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
APPS   += msop
CLEANS += msop-clean


