######################################################################
## Filename:      vapi.makefile
## Author:        zbing
## Created at:    2015-10-20
##                
## Description:   编译vapi
##               
## Copyright (C)  milesight
##                
######################################################################
#需要包含的目录
INCLUDE         	:= -I$(PUBLIC_INC_DIR) \
		           -I$(PUBLIC_INC_DIR)/vapi \
			   -I$(PUBLIC_INC_DIR)/vapi/$(PLATFORM_TYPE) \
			   -I$(PUBLIC_INC_DIR)/msstd\
			   -I$(PUBLIC_INC_DIR)/vapi/$(PLATFORM_TYPE)/speex \

#需要的lib库
LDFLAGS         	:= -lrt -lm -L$(STD_USRLIB_DIR) -lhdal -lvendor_media -lsamplerate \
       		         -L$(PUBLIC_LIB_DIR) -lmsstd 

CFLAGS += -Werror -Wno-error=unused-variable -Wno-error=unused-function

#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE		:="so"


#导入通用mk文件
include $(BUILD_COMMON_MK)

