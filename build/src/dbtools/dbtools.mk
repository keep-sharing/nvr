######################################################################
## Filename:      dbtools.mk
## Author:        bruce.milesight
## Created at:    2015-11-17
##                
## Description:   编译中心程序 dbtools
## Copyright (C)  milesight
##                
######################################################################
#源码目录
DBTOOLS_SRC_DIR    	:= $(SRC_DIR)/dbtools
#输出目录
DBTOOLS_TARGET_DIR 	:= $(TEMP_TARGET_DIR)/dbtools
#目标名称
DBTOOLS_TARGET_NAME	:= dbtools

DBTOOLS_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(DBTOOLS_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(DBTOOLS_TARGET_DIR) TARGET_NAME=$(DBTOOLS_TARGET_NAME) INSTALL_DIR=$(PUBLIC_APP_DIR)

dbtools:
	$(MS_MAKEFILE_V)$(MAKE) $(DBTOOLS_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_APP_DIR)/$(DBTOOLS_TARGET_NAME)
	
dbtools-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(DBTOOLS_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
APPS   += dbtools
CLEANS += dbtools-clean


