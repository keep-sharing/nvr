######################################################################
## Filename:      dosfsck.mk
## Author:        eric
## Created at:    2014-07-15
##                
## Description:   编译 dosfsck 的全局配置
## Copyright (C)  milesight
##                
######################################################################
#源码目录
DOSFSCK_SRC_DIR    	:= $(SRC_DIR)/test/dosfsck
#输出目录
DOSFSCK_TARGET_DIR 	:= $(TEMP_TARGET_DIR)/test/dosfsck
#目标名称
DOSFSCK_TARGET_NAME	:= dosfsck

DOSFSCK_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(DOSFSCK_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(DOSFSCK_TARGET_DIR) TARGET_NAME=$(DOSFSCK_TARGET_NAME) INSTALL_DIR=$(PUBLIC_APP_DIR)

dosfsck:
	$(MS_MAKEFILE_V)$(MAKE) $(DOSFSCK_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_APP_DIR)/$(DOSFSCK_TARGET_NAME)
	
dosfsck-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(DOSFSCK_COMPILE_OPT) clean
	
#添加至全局LIBS/CLEANS
APPS   += dosfsck
CLEANS += dosfsck-clean


