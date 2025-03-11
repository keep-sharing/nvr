######################################################################
## Filename:      dosfsck.makefile
## Author:        eric.milesight
## Created at:    2014.11.26
##                
## Description:   dosfsck for check sd/mmc error
##               
## Copyright (C)  milesight
##                
######################################################################
#需要包含的目录
INCLUDE         := -I. -O2 -fomit-frame-pointer \
				-D_GNU_SOURCE $(shell getconf LFS_CFLAGS) \
				-Wall -Wextra -Wno-sign-compare \
				-Wno-missing-field-initializers \
				-Wmissing-prototypes \
				-Wstrict-prototypes

#需要的lib库
LDFLAGS         := -O2 -O2 -fomit-frame-pointer \
				-D_GNU_SOURCE $(shell getconf LFS_CFLAGS) \
				-Wall -Wextra -Wno-sign-compare \
				-Wno-missing-field-initializers \
				-Wmissing-prototypes \
				-Wstrict-prototypes

#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE	:="exe"


#导入通用mk文件
include $(BUILD_COMMON_MK)

