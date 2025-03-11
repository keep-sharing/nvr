######################################################################
## Filename:      mschip_update.mk
## Author:        eric
## Created at:    2014-07-15
##                
## Description:   编译 mschip_update 的全局配置
## Copyright (C)  milesight
##                
######################################################################
#源码目录
MSCUP_SRC_DIR    	:= $(SRC_DIR)/test/mschip_update/$(PLATFORM_TYPE)
#输出目录
MSCUP_TARGET_DIR 	:= $(TEMP_TARGET_DIR)/test/mschip_update
#目标名称
MSCUP_TARGET_NAME	:= mschip_update

MSCUP_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(MSCUP_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(MSCUP_TARGET_DIR) TARGET_NAME=$(MSCUP_TARGET_NAME) INSTALL_DIR=$(PUBLIC_APP_DIR)

mschip:
	$(MS_MAKEFILE_V)$(MAKE) $(MSCUP_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_APP_DIR)/$(MSCUP_TARGET_NAME)
	
mschip-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(MSCUP_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
APPS   += mschip
CLEANS += mschip-clean


