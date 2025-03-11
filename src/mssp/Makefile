######################################################################
## Filename:      mssp.makefile
## Author:        bruce.milesight
## Created at:    2015-12-04
##                
## Description:   编译 mssp
##               
## Copyright (C)  milesight
##                
######################################################################
#需要包含的目录
INCLUDE         := -I$(PUBLIC_INC_DIR) \
				   -I$(PUBLIC_INC_DIR)/msdb \
			       -I$(PUBLIC_INC_DIR)/mssocket \
				   -I$(PUBLIC_INC_DIR)/pcap \
				   -I$(PUBLIC_INC_DIR)/openssl
#需要的lib库
LDFLAGS         := -ldl -lpthread -L$(PUBLIC_LIB_DIR) -lmsstd -lmsdb -lmssocket -L$(PUBLIC_STATIC_LIB_DIR) -lpcap -lencrypt -L$(STD_USRLIB_DIR) -lsqlite3 -lcrypto -lssl
#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE:="exe"


#导入通用mk文件
include $(BUILD_COMMON_MK)



