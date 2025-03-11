######################################################################
## Filename:      board.mk
## Author:        eric@milesight.cn
## Created at:    2014-05-13
##                
## Description:   编译 amboot kernel filesys 的全局配置
## Copyright (C)  milesight
##                
######################################################################
#源码目录

BOARD_BUILD_DIR = $(BUILD_DIR)/src/boards

export BOARD_BUILD_DIR

include $(BOARD_BUILD_DIR)/$(PLATFORM_TYPE)/*/*.mk


