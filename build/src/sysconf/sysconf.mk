######################################################################
## Filename:      sysconf.mk
## Author:        bruce.milesight
## Created at:    2015-11-13
##                
## Description:   编译环境配置 sysconf
## Copyright (C)  milesight
##                
######################################################################
#源码目录
SYSCONF_SRC_DIR    	:= $(SRC_DIR)/sysconf
#输出目录
SYSCONF_TARGET_DIR 	:= $(TEMP_TARGET_DIR)/sysconf
#目标名称
SYSCONF_TARGET_NAME	:= sysconf

SYSCONF_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(SYSCONF_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(SYSCONF_TARGET_DIR) TARGET_NAME=$(SYSCONF_TARGET_NAME) INSTALL_DIR=$(PUBLIC_APP_DIR)

sysconf:
	$(MS_MAKEFILE_V)$(MAKE) $(SYSCONF_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_APP_DIR)/$(SYSCONF_TARGET_NAME)
	
sysconf-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(SYSCONF_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
APPS   += sysconf
CLEANS += sysconf-clean


