## Filename:      demo.makefile
## Author:        zbing.milesight
## Created at:    2016-04-22
##                
## Description:   临时测试demo
##               
## Copyright (C)  milesight
##                
######################################################################
#宏定义

#需要包含的目录
INCLUDE		:= -D_GNU_SOURCE -D__USE_XOPEN -DEGL_API_FB -DEGL_FBDEV \
			-I$(PUBLIC_INC_DIR) \
			-I$(PUBLIC_INC_DIR)/vapi \
			-I$(PUBLIC_INC_DIR)/vapi/$(PLATFORM_TYPE) \
			-I$(PUBLIC_INC_DIR)/rtsp \
			-I$(PUBLIC_INC_DIR)/msfs \
			-I$(PUBLIC_INC_DIR)/poe \
			-I$(PUBLIC_INC_DIR)/log \
			-I$(PUBLIC_INC_DIR)/msdb \
			-I$(PUBLIC_INC_DIR)/gpio \
			-I$(PUBLIC_INC_DIR)/gpio/$(PLATFORM_TYPE) \
			-I$(PUBLIC_INC_DIR)/fisheye \
			-I$(PUBLIC_INC_DIR)/mssocket
					


LDFLAGS		:= -lstdc++ -lm -ldl -lpthread \
			-L$(PUBLIC_LIB_DIR)/ -losa -lmsstd -lmsdb -llog -lvapi -lfaad -lgpio -lencrypt -lrecortsp -lpoe -lmsfs -luv -lmssocket \
  			-L$(PUBLIC_STATIC_LIB_DIR) -lpcap -lIOTCAPIs -lAVAPIs \
			-L$(PUBLIC_STATIC_LIB_DIR)/vapi \
			-L$(STD_USRLIB_DIR) -lssl -lcrypto -lx264 -lx265 -lsqlite3 -lcurl -luuid -lnghttp2 #-lparted# \
			#-L./ -lkey_verify

ifeq ($(PLATFORM_TYPE), hi3798)
LDFLAGS += -L$(STD_USRLIB_DIR) -ljpeg -lhi_msp -lhi_common -lhi_jpeg -lasound -lspeexdsp
else ifeq ($(PLATFORM_TYPE), hi3536g)
LDFLAGS += -lfisheye  -lGLESv2  -lmali 
else ifeq ($(PLATFORM_TYPE), hi3536a)
LDFLAGS += -lfisheye  -lGLESv2  -lmali 
else ifeq ($(PLATFORM_TYPE), nt98323)
LDFLAGS += -lhdal -lgm2d -lvendor_media -lsamplerate -lspeexdsp
else
$(error PLATFORM_TYPE ERROR: $(PLATFORM_TYPE))
endif

#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE	:= "exe"

#导入通用mk文件
include $(BUILD_COMMON_MK)
