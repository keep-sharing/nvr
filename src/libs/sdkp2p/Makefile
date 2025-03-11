######################################################################
## Filename:      sdkp2p.makefile
## Author:        hrz
## Created at:    2019-04-08
##                
## Description:   编译sdkp2p
##               
## Copyright (C)  milesight
##                
######################################################################
#需要包含的目录
INCLUDE         := -I$(PUBLIC_INC_DIR)  -fPIC \
				   -I$(PUBLIC_INC_DIR)/sdkp2p \
                   -I$(PUBLIC_INC_DIR)/mssocket \
                   -I$(PUBLIC_INC_DIR)/msfs \
                   -I$(PUBLIC_INC_DIR)/msdb \
                   -I$(PUBLIC_INC_DIR)/disk \
                   -I$(PUBLIC_INC_DIR)/msptz \
                   -I$(PUBLIC_INC_DIR)/log \
                   -I$(PUBLIC_INC_DIR)/msstd -D _GNU_SOURCE -D __USE_XOPEN -D_FILE_OFFSET_BITS=64

#需要的lib库
LDFLAGS         := 
#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE:="so"


#导入通用mk文件
include $(BUILD_COMMON_MK)

	
