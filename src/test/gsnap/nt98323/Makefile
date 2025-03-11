######################################################################
## Filename:      gsnap.makefile
## Author:        huangwx.milesight 
## Created at:    2016-03-14
##                
## Description:   gsnap test demo
##               
## Copyright (C)  milesight
##                
######################################################################
#需要包含的目录
INCLUDE         :=

#需要的lib库
LDFLAGS              := -lm  \
                        -L$(PUBLIC_STATIC_LIB_DIR) -lpng -ljpeg -lz

#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE     :="exe"

#导入通用mk文件
include $(BUILD_COMMON_MK)

