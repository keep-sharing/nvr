######################################################################
## Filename:      msop.makefile
## Author:        hrz.milesight
## Created at:    2022-05-06
##                
## Description:   ms operations
##               
## Copyright (C)  milesight
##                
######################################################################
#需要包含的目录
#-DEVM_DEBUG 
INCLUDE			:= -I$(PUBLIC_INC_DIR) \
			       -I$(PUBLIC_INC_DIR)/mssocket

#需要的lib库
LDFLAGS			:= -ldl -lpthread -L$(PUBLIC_LIB_DIR) -lmsstd -lmssocket -L$(PUBLIC_STATIC_LIB_DIR) -lencrypt -L$(STD_USRLIB_DIR) -lcrypto

#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE:="exe"

#导入通用mk文件
include $(BUILD_COMMON_MK)
