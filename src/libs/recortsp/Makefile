######################################################################
## Filename:      recortsp.makefile
## Author:        david.milesight
## Created at:    2021-06-30 
##                
## Description:   编译 recortsp
##               
## Copyright (C)  milesight
##                
######################################################################
#需要包含的目录
INCLUDE         := -fPIC -DHI_3536 -DMS_PRIVATE_RTSP\
		   -I$(PUBLIC_INC_DIR) \
		   -I$(PUBLIC_INC_DIR)/osa \
		   -I$(PUBLIC_INC_DIR)/rtsp \
		   -I$(PUBLIC_INC_DIR)/jpeg \
		   -I$(PUBLIC_INC_DIR)/x264 \
		   -I$(PUBLIC_INC_DIR)/x265 \
		   -I$(PUBLIC_INC_DIR)/msrtsp \
		   -I$(PUBLIC_INC_DIR)/ffmpeg
		   
#ifeq ($(PLATFORM_TYPE), hi3536g)	
#INCLUDE += -DHI_3536   
#else
#ifeq ($(PLATFORM_TYPE), hi3536g)
#INCLUDE += -DHI_3536   
#else
#ifeq ($(PLATFORM_TYPE), hi3536g)
#INCLUDE += -DHI_3536   
#else
#INCLUDE += -DHI_3536 
#endif
#endif
#endif

#需要的lib库
LDFLAGS         := -L$(STD_LIB_DIR) -L$(STD_USRLIB_DIR) -lx264 -lx265 \
			-L$(PUBLIC_STATIC_LIB_DIR) -lmsstream -lrtsp -lrtp -lhttp -lmediasdk -laio \
			-L$(PUBLIC_LIB_DIR) -lmsstd

#ifeq ($(PLATFORM_TYPE), hi3536g)
#LDFLAGS += -ltcmalloc
#endif
#目标类型("exe"-生成可执行文件 "so"-生成动态库 "a"-生成静态库)
TARGET_TYPE:="so"

#导入通用mk文件
include $(BUILD_COMMON_MK)


