######################################################################
## Filename:      test.mk
## Author:        eric@milesight.cn
## Created at:    2014-05-13
##                
## Description:   编译 test exe 的全局配置
## Copyright (C)  milesight
##                
######################################################################
#源码目录

TEST_BUILD_DIR 	= $(BUILD_DIR)/src/test

export TEST_BUILD_DIR

#include $(TEST_BUILD_DIR)/*/*.mk
include $(TEST_BUILD_DIR)/cpu_load/*.mk
include $(TEST_BUILD_DIR)/dosfsck/*.mk
include $(TEST_BUILD_DIR)/ethtool/*.mk
include $(TEST_BUILD_DIR)/gsnap/*.mk
include $(TEST_BUILD_DIR)/hardware/*.mk
include $(TEST_BUILD_DIR)/iperf/*.mk
include $(TEST_BUILD_DIR)/mkdosfs/*.mk
include $(TEST_BUILD_DIR)/mschip_update/*.mk
include $(TEST_BUILD_DIR)/temperature/*.mk


