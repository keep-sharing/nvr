######################################################################
## Filename:      dhcpcd.mk
## Author:        bruce
## Created at:    2014-06-12
##                
## Description:   编译dhcpcd的全局配置
## Copyright (C)  milesight
##                
######################################################################
#源码目录
DHCP_SRC_DIR    := $(SRC_DIR)/dhcpcd
#输出目录
DHCP_TARGET_DIR := $(TEMP_TARGET_DIR)/dhcpcd
#目标名称
DHCP_TARGET_NAME:= dhcpcd

DHCP_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(DHCP_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(DHCP_TARGET_DIR) TARGET_NAME=$(DHCP_TARGET_NAME) INSTALL_DIR=$(PUBLIC_APP_DIR)

dhcpcd:
	$(MS_MAKEFILE_V)$(MAKE) $(DHCP_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_APP_DIR)/$(DHCP_TARGET_NAME)
	
dhcpcd-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(DHCP_COMPILE_OPT) clean
	
#添加至全局LIBS/CLEANS
APPS   += dhcpcd
CLEANS += dhcpcd-clean


