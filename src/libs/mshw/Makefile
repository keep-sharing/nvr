######################################################################
## Filename:      hardware.makefile
## Author:        zbing.milesight 
## Created at:    2016-01-05
##                
## Description:   hardware test demo
##               
## Copyright (C)  milesight
##                
######################################################################
#需要包含的目录
INCLUDE         := -I$(PUBLIC_INC_DIR)\
								   -I$(PUBLIC_INC_DIR)/vapi \
					   			 -I$(PUBLIC_INC_DIR)/vapi/$(PLATFORM_TYPE) \
								-I$(PUBLIC_INC_DIR)/vapi/$(PLATFORM_TYPE)/speex \
					   			 -I$(PUBLIC_INC_DIR)/rtsp \
								 -I$(PUBLIC_INC_DIR)/mshw \
					   			 -I$(PUBLIC_INC_DIR)/gpio \
				           -I$(PUBLIC_INC_DIR)/gpio/$(PLATFORM_TYPE)

#需要的lib库
LDFLAGS         	:= -lstdc++ -lm -ldl -lpthread -lrt \
			 -L$(PUBLIC_LIB_DIR)/ -losa -lmsstd  -lvapi -lfaad -lgpio -lencrypt -lrecortsp -lchipinfo\
			 -L$(PUBLIC_STATIC_LIB_DIR) -lpcap -lIOTCAPIs -lAVAPIs \
			 -L$(PUBLIC_STATIC_LIB_DIR)/vapi \
			 -L$(STD_USRLIB_DIR) -lssl -lcrypto -lx264 -lx265 -lsqlite3 -lcurl -luuid

ifeq ($(PLATFORM_TYPE), hi3536c)
LDFLAGS += -L./ -lkey_verify
endif

ifeq ($(PLATFORM_TYPE), hi3536g)
LDFLAGS += -L./ -lkey_verify
endif

ifeq ($(PLATFORM_TYPE), hi3798)
LDFLAGS += -L$(STD_USRLIB_DIR) -ljpeg -lhi_msp -lhi_common -lhi_jpeg -lfaad -lasound -lspeexdsp
endif

#ifeq ($(PLATFORM_TYPE), nt98323)
ifneq ($(filter $(PLATFORM_TYPE), $(nt98323) $(nt98633)), )
LDFLAGS += -lhdal -lgm2d -lvendor_media -lsamplerate -lspeexdsp
endif


#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE	:="so"


#导入通用mk文件
include $(BUILD_COMMON_MK)

