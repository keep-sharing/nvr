## Filename:      fio.makefile
## Author:        zbing.milesight
## Created at:    2016-01-18
##                
## Description:   extre io ctrl
##               
## Copyright (C)  milesight
##                
######################################################################
#宏定义

#需要包含的目录
INCLUDE         := -I$(PUBLIC_INC_DIR) \
		   -I$(PUBLIC_INC_DIR)/msdb \
		   -I$(PUBLIC_INC_DIR)/mssocket \
	           -I$(PUBLIC_INC_DIR)/gpio \
		   -I$(PUBLIC_INC_DIR)/gpio/$(PLATFORM_TYPE) \
		   -I$(PUBLIC_INC_DIR)/vapi/$(PLATFORM_TYPE)
#需要的lib库
LDFLAGS         := -lm -ldl -lpthread -lrt -L$(PUBLIC_LIB_DIR) -lgpio -lmsstd -lmsdb -lmssocket -lencrypt \
		   -L$(STD_USRLIB_DIR) -lsqlite3 -lssl -lcrypto

CFLAGS += -std=gnu99

ifeq ($(PLATFORM_TYPE), hi3798)
LDFLAGS += -L$(STD_USRLIB_DIR) -lhi_msp -lhi_common -lhi_jpeg
endif
#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE:="exe"

#导入通用mk文件
include $(BUILD_COMMON_MK)
