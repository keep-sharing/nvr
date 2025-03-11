######################################################################
## Filename:      fisheye.makefile
## Author:        zbing.milesight
## Created at:    2020-03-05 09:18:04
##                
## Description:   编译 fisheye
##               
## Copyright (C)  milesight
##                
######################################################################
#需要包含的目录
INCLUDE         := -D_GNU_SOURCE -D__USE_XOPEN -D_FILE_OFFSET_BITS=64 -DEGL_API_FB -DEGL_FBDEV\
		   -DMALI_USE_DMA_BUF -DEGL_EGLEXT_PROTOTYPES -DGL_GLEXT_PROTOTYPES \
		   -I$(PUBLIC_INC_DIR) \
		   -I$(PUBLIC_INC_DIR)/fisheye \
	           -I$(PWD)
#需要的lib库
LDFLAGS         := -L$(STD_LIB_DIR)
#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE:="so"

#暂时先这样添加宏定义
CFLAGS += -D_HI3536_

#ifeq ($(PLATFORM_TYPE), hi3798)
ifeq ($(PLATFORM_TYPE), hi3536g)
#导入通用mk文件
include $(BUILD_COMMON_MK)
else ifeq ($(PLATFORM_TYPE), hi3536a)
include $(BUILD_COMMON_MK)
else
all:
clean:
endif

