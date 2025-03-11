######################################################################
## Filename:      tsar.makefile
## Author:        hrz.milesight
## Created at:    2022-04-16
##                
## Description:   编译 tsar
##               
## Copyright (C)  milesight
##                
######################################################################
#需要包含的目录
INCLUDE         := -I$(PUBLIC_INC_DIR) \
				   -I$(STD_INC_DIR) -I. -D _GNU_SOURCE -D __USE_XOPEN 

#需要的lib库
LDFLAGS         := -ldl -lpthread -L$(PUBLIC_LIB_DIR) -lmsstd
#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE:="exe"


#导入通用mk文件
include $(BUILD_COMMON_MK)



