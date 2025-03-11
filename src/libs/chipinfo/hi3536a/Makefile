######################################################################
## Filename:      chipinfo.makefile
## Author:        young
## Created at:    2017-11-15
##                
## Description:   chipinfo
##               
## Copyright (C)  milesight
##                
######################################################################
#需要包含的目录
INCLUDE         := -I$(PUBLIC_INC_DIR) -I$(PUBLIC_INC_DIR)/vapi/$(PLATFORM_TYPE)/ -I.

#需要的lib库
LDFLAGS         := -L$(PUBLIC_STATIC_LIB_DIR) -larm-env

CFLAGS		:= $(PLATFORM_FLAGS) -DUSE_HOSTCC
#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE	:="so"


#导入通用mk文件
include $(BUILD_COMMON_MK)

