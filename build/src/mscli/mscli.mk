######################################################################
## Filename:      mscli.mk
## Author:        bruce
## Created at:    2015-11-07
##                
## Description:   编译控制台程序 mscli
## Copyright (C)  milesight
##                
######################################################################
#源码目录
MSCLI_SRC_DIR    	:= $(SRC_DIR)/mscli
#输出目录
MSCLI_TARGET_DIR 	:= $(TEMP_TARGET_DIR)/mscli
#目标名称
MSCLI_TARGET_NAME	:= mscli

MSCLI_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(MSCLI_SRC_DIR) CC=$(TARGET_CC) CXX=$(TARGET_CXX) AR=$(TARGET_AR) TARGET_DIR=$(MSCLI_TARGET_DIR) TARGET_NAME=$(MSCLI_TARGET_NAME) INSTALL_DIR=$(PUBLIC_APP_DIR)

mscli:
	$(MS_MAKEFILE_V)$(MAKE) $(MSCLI_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_APP_DIR)/$(MSCLI_TARGET_NAME)
	
mscli-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(MSCLI_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
APPS   += mscli
CLEANS += mscli-clean


