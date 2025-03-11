######################################################################
## Filename:      mschip_update.makefile
## Author:        eric.milesight created 2014.7.15
## Created at:    2014-05-13
##                
## Description:   mschip_update
##               
## Copyright (C)  milesight
##                
######################################################################
#需要包含的目录
INCLUDE         := -I. -I$(PUBLIC_INC_DIR) -I$(PUBLIC_INC_DIR)/vapi/$(PLATFORM_TYPE) -I$(SRC_DIR)/boards/$(PLATFORM_TYPE)/firmware/env

#需要的lib库
LDFLAGS         := -L$(PUBLIC_LIB_DIR) -lmsstd -lchipinfo -lpthread

CFLAGS		:= $(PLATFORM_FLAGS)

#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE	:="exe"


#导入通用mk文件
include $(BUILD_COMMON_MK)

