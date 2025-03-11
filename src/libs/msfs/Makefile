######################################################################
## Filename:      index.makefile
## Author:        zbing.milesight
## Created at:    2017-04-19 09:18:04
##                
## Description:   编译 index
##               
## Copyright (C)  milesight
##                
######################################################################
#需要包含的目录
INCLUDE         := -D_GNU_SOURCE -D__USE_XOPEN -D_FILE_OFFSET_BITS=64 \
		   -I$(PUBLIC_INC_DIR) -fPIC \
		   -I$(PUBLIC_INC_DIR)/msfs \
		   -I$(PUBLIC_INC_DIR)/log
#需要的lib库
LDFLAGS         := -L$(STD_LIB_DIR) -L$(STD_USRLIB_DIR)

CFLAGS += -fgnu89-inline

#ifeq ($(PLATFORM_TYPE), hi3536g)
#LDFLAGS += -ltcmalloc
#endif
#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE:="so"

#导入通用mk文件
include $(BUILD_COMMON_MK)


