######################################################################
## Filename:      backup.makefile
## Author:        bruce.milesight
## Created at:    2015-12-8
##                
## Description:   编译 backup
##               
## Copyright (C)  milesight
##                
######################################################################
#需要包含的目录
INCLUDE         	:= -I$(PUBLIC_INC_DIR) -D_GNU_SOURCE -D__USE_XOPEN \
					   -I$(PUBLIC_INC_DIR)/msdb \
					   -I$(PUBLIC_INC_DIR)/cellular \
					   -I$(PUBLIC_INC_DIR)/disk \
					   -I$(PUBLIC_INC_DIR)/avilib
#需要的lib库
LDFLAGS         	:= -ldl -lpthread -L$(PUBLIC_LIB_DIR)/ -lmsstd -lcellular -ldisk -lavilib -lmsdb -L$(STD_USRLIB_DIR) -lsqlite3
#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE		:="exe"


#导入通用mk文件
include $(BUILD_COMMON_MK)

