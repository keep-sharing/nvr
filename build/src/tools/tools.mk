######################################################################
## Filename:      tools.mk
## Author:        david@milesight.cn
## Created at:    2019-01-30
##                
## Description:   编译 tools 的全局配置
## Copyright (C)  milesight
##                
######################################################################
#源码目录

TOOLS_BUILD_DIR = $(BUILD_DIR)/src/tools


#include $(TOOLS_BUILD_DIR)/*/*.mk
include $(TOOLS_BUILD_DIR)/busybox/*.mk
include $(TOOLS_BUILD_DIR)/kconf/*.mk
include $(TOOLS_BUILD_DIR)/makfs/*.mk
#include $(TOOLS_BUILD_DIR)/ntpdate/*.mk
#include $(TOOLS_BUILD_DIR)/nginx/*.mk
#include $(TOOLS_BUILD_DIR)/nghttp2/*.mk

