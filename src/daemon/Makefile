######################################################################
## Filename:      daemon.makefile
## Author:        bruce.milesight
## Created at:    2015-12-2
##                
## Description:   daemon for mscore
##               
## Copyright (C)  milesight
##                
######################################################################
#需要包含的目录
#-DEVM_DEBUG 
INCLUDE			:= -I$(PUBLIC_INC_DIR) -I$(PUBLIC_INC_DIR)/msdb -I$(SRC_DIR)/msgui/mscore #-I$(PUBLIC_INC_DIR)/$(PLATFORM_TYPE)

#需要的lib库
LDFLAGS			:= -ldl -lpthread -L$(PUBLIC_LIB_DIR) -lmsstd -lmsdb -L$(STD_USRLIB_DIR) -lsqlite3

#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE:="exe"

#导入通用mk文件
include $(BUILD_COMMON_MK)
