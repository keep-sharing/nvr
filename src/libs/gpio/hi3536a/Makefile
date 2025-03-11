######################################################################
## Filename:      msdb.makefile
## Author:        bruce
## Created at:    2014-05-04
##                
## Description:   编译msdb
##               
## Copyright (C)  milesight
##                
######################################################################
#需要包含的目录
INCLUDE := -I$(PUBLIC_INC_DIR)/gpio \
					 -I$(PUBLIC_INC_DIR)/gpio/$(PLATFORM_TYPE)

#需要的lib库
LDFLAGS         	:= -L$(PUBLIC_LIB_DIR) -L$(STD_LIB_DIR) -L$(STD_USRLIB_DIR) -lmsstd

#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE		:="so"


#导入通用mk文件
include $(BUILD_COMMON_MK)

