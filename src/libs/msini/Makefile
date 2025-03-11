######################################################################
## Filename:      msini.makefile
## Author:        zbing.milesight
## Created at:    2019-05-23 16:06:04
##                
## Description:   编译 hi3536/msini
##               
## Copyright (C)  milesight
##                
######################################################################
#需要包含的目录
INCLUDE         := -I$(PUBLIC_INC_DIR) \
		   -I$(PUBLIC_INC_DIR)/msini
#需要的lib库
LDFLAGS         := -L$(STD_LIB_DIR)
#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE:="so"


#导入通用mk文件
include $(BUILD_COMMON_MK)


