######################################################################
## Filename:      daemon.mk
## Author:        bruce
## Created at:    2015-12-02
##                
## Description:   编译 daemon 的全局配置
## Copyright (C)  milesight
##                
######################################################################
#源码目录
DAEMON_SRC_DIR    	:= $(SRC_DIR)/daemon
#输出目录
DAEMON_TARGET_DIR 	:= $(TEMP_TARGET_DIR)/daemon
#目标名称
DAEMON_TARGET_NAME	:= daemon

DAEMON_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(DAEMON_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(DAEMON_TARGET_DIR) TARGET_NAME=$(DAEMON_TARGET_NAME) INSTALL_DIR=$(PUBLIC_APP_DIR)

daemon:
	$(MS_MAKEFILE_V)$(MAKE) $(DAEMON_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_APP_DIR)/$(DAEMON_TARGET_NAME)
	
daemon-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(DAEMON_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
APPS   += daemon
CLEANS += daemon-clean


