######################################################################
## Filename:      sysconf Makefile
## Author:        bruce.milesight
## Created at:    2015-11-13
##                
## Description:   编译 sysconf
##
## Copyright (C)  milesight
##                
######################################################################

#需要包含的目录
INCLUDE         := -I$(PUBLIC_INC_DIR)
#需要的lib库
LDFLAGS         := -Wall -O2 -L$(PUBLIC_LIB_DIR) -L$(STD_USRLIB_DIR) -lcrypto -lssl
#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE:="exe"


#导入通用mk文件
include $(BUILD_COMMON_MK)
