######################################################################
## Filename:      poe.makefile
## Author:        zbing.milesight
## Created at:    2016-08-25 16:06:04
##                
## Description:   编译 hi3536/poe
##               
## Copyright (C)  milesight
##                
######################################################################
#需要包含的目录
INCLUDE         := -I$(PUBLIC_INC_DIR) -fPIC \
				   -I$(PUBLIC_INC_DIR)/poe -D _GNU_SOURCE -D __USE_XOPEN -D_FILE_OFFSET_BITS=64
#需要的lib库
LDFLAGS         := -L$(STD_LIB_DIR)
#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE:="so"


#导入通用mk文件
include $(BUILD_COMMON_MK)


