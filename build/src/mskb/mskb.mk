######################################################################
## Filename:      mskb.mk
## Author:        zbing
## Created at:    2017-01-17
##                
## Description:   编译keyboard
## Copyright (C)  milesight
##                
######################################################################
#源码目录
MSKB_SRC_DIR    	:= $(SRC_DIR)/mskb
#输出目录
MSKB_TARGET_DIR 	:= $(TEMP_TARGET_DIR)/mskb
#目标名称
MSKB_TARGET_NAME	:= mskb

MSKB_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(MSKB_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(MSKB_TARGET_DIR) TARGET_NAME=$(MSKB_TARGET_NAME) INSTALL_DIR=$(PUBLIC_APP_DIR)

mskb:
	$(MS_MAKEFILE_V)$(MAKE) $(MSKB_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_APP_DIR)/$(MSKB_TARGET_NAME)
	
mskb-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(MSKB_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
APPS   += mskb
CLEANS += mskb-clean


