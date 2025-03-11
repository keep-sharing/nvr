## Filename:      update.cgi.makefile
## Author:        bruce.milesight
## Created at:    2014-06-11
##                
## Description:   update.cgi
##               
## Copyright (C)  milesight
##                
######################################################################
#宏定义

#需要包含的目录
INCLUDE         := -I$(PUBLIC_INC_DIR) -I.\
                   -I$(PUBLIC_INC_DIR)/msdb \
	           -I$(SRC_DIR)/msgui/mscore
LDFLAGS         := -O2 -L$(PUBLIC_LIB_DIR)
#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE:="exe"

#导入通用mk文件
include $(BUILD_COMMON_MK)
