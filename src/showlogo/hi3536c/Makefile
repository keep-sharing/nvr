######################################################################
## Filename:      showlogo.makefile
## Author:        zbing.milesight
## Created at:    2015-12-15
##                
## Description:   show logo
##               
## Copyright (C)  milesight
##                
######################################################################
#需要包含的目录
#-DEVM_DEBUG 
INCLUDE			:= -I$(PUBLIC_INC_DIR) \
					     -I$(PUBLIC_INC_DIR)/vapi \
					     -I$(PUBLIC_INC_DIR)/vapi/$(PLATFORM_TYPE)
#需要的lib库
LDFLAGS			:= -ldl -lpthread -lm -L$(PUBLIC_LIB_DIR) -lmsstd \
			-L$(PUBLIC_STATIC_LIB_DIR)/vapi -lmpi -lhdmi -ljpeg -lVoiceEngine -lupvqe -ldnvqe
#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE:="exe"

#导入通用mk文件
include $(BUILD_COMMON_MK)
