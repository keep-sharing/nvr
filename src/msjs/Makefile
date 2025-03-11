######################################################################
## Filename:      armloading.makefile
## Author:        eric.milesight created 2014.9.15
## Created at:    2014-9-15
##                
## Description:   armloading
##               
## Copyright (C)  milesight
##                
######################################################################
#需要包含的目录
INCLUDE         := -I. -I$(PUBLIC_INC_DIR) \
		   -I$(PUBLIC_INC_DIR)/mssocket \
			-I$(PUBLIC_INC_DIR)/msdb

#需要的lib库
LDFLAGS         := -lm -ldl -lpthread -lrt -L$(PUBLIC_LIB_DIR) -lmsstd -lmssocket -lmsdb -lencrypt \
                   -L$(STD_USRLIB_DIR) -lsqlite3 -lssl -lcrypto 

#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE	:="exe"


#导入通用mk文件
include $(BUILD_COMMON_MK)

