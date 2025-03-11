######################################################################
## Filename:      encrypt.makefile
## Author:        bruce.milesight
## Created at:    2015-11-09 
##                
## Description:   编译 encrypt
##               
## Copyright (C)  milesight
##                
######################################################################
#需要包含的目录
INCLUDE         := -I$(PUBLIC_INC_DIR) -fPIC \
				   -I$(PUBLIC_INC_DIR)/openssl
#需要的lib库
LDFLAGS         := -L$(STD_USRLIB_DIR) -lssl -lcrypto 
#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE:="so"


#导入通用mk文件
include $(BUILD_COMMON_MK)
