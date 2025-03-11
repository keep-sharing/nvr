######################################################################
## Filename:      webserver.mk
## Author:        david
## Created at:    2015-07-13 
##
## Description:   编译web server的全局配置
##                1.定义源码目录
##                2.定义输出目录
##                3.定义目标名称
## Copyright (C)  milesight
##
######################################################################
#源码目录
WEBSERVER_SRC_DIR    := $(SRC_DIR)/web/webserver
#输出目录
WEBSERVER_TARGET_DIR := $(TEMP_TARGET_DIR)/web/webserver
#目标名称
WEBSERVER_TARGET_NAME:= boa

WEBSERVER_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(WEBSERVER_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(WEBSERVER_TARGET_DIR) TARGET_NAME=$(WEBSERVER_TARGET_NAME) INSTALL_DIR=$(PUBLIC_APP_DIR)

webserver:
	$(MS_MAKEFILE_V)$(MAKE) $(WEBSERVER_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_APP_DIR)/$(WEBSERVER_TARGET_NAME)
	
webserver-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(WEBSERVER_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
APPS   += webserver
CLEANS += webserver-clean
