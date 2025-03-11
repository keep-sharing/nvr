######################################################################
## Filename:      qrencode.makefile
## Author:        david.milesight
## Created at:    2016-12-23 11:43:21
##
## Description:   编译 qrencode
##
## Copyright (C)  milesight
##
######################################################################
#需要包含的目录
INCLUDE         := -I$(PUBLIC_INC_DIR) -D_GNU_SOURCE -D__USE_XOPEN \
				   -I$(PUBLIC_INC_DIR) -fPIC \
				   -I$(PUBLIC_INC_DIR)/qrencode \
#需要的lib库
LDFLAGS         := -ldl -lpthread -L$(PUBLIC_LIB_DIR) -lmsstd -lmssocket -lencrypt -L$(STD_USRLIB_DIR) -lssl -lcrypto 
#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE:="so"


#导入通用mk文件
include $(BUILD_COMMON_MK)



