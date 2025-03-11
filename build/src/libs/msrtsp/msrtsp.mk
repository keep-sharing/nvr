######################################################################
## Filename:      msrtsp.mk
## Author:        david
## Created at:    2021-06-29
##                
## Description:   编译 ms rtsp
##
## Copyright (C)  milesight
##                
######################################################################
#源码目录
MS_RTSP_LIB_SRC_DIR		:= $(SRC_DIR)/libs/msrtsp

#头文件目录
MS_RTSP_INC_SRC_DIR		:= $(PUBLIC_INC_DIR)/msrtsp

#输出目录
#MS_RTSP_LIB_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/msrtsp
MS_RTSP_LIB_TARGET_DIR	:=	$(PUBLIC_STATIC_LIB_DIR)

#HI3536c 平台
MS_PLATFORM_FLAG		:= 0

#根据平台设置
ifeq ($(PLATFORM_TYPE), hi3536g)
MS_PLATFORM_FLAG		:= 0
CROSS_PREFIX			:= arm-hisiv400-linux
HI_PLATFORM_FLAGS 		:= "-march=armv7-a -mfloat-abi=softfp -mfpu=neon-vfpv4 -ffunction-sections -mno-unaligned-access -fno-aggressive-loop-optimizations -O2"
else 
ifeq ($(PLATFORM_TYPE), hi3536c)
MS_PLATFORM_FLAG		:= 1
CROSS_PREFIX			:= arm-hisiv500-linux
HI_PLATFORM_FLAGS 		:= "-march=armv7-a -mfloat-abi=softfp -mfpu=neon-vfpv4 -ffunction-sections -mno-unaligned-access -fno-aggressive-loop-optimizations -O2"
else 
ifeq ($(PLATFORM_TYPE), hi3798)
MS_PLATFORM_FLAG		:= 2
CROSS_PREFIX			:= arm-histbv310-linux
HI_PLATFORM_FLAGS 		:= "-march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -ffunction-sections -fdata-sections -O2"
else
ifeq ($(PLATFORM_TYPE), nt98323)
MS_PLATFORM_FLAG		:= 3
CROSS_PREFIX			:= arm-ca9-linux-gnueabihf
HI_PLATFORM_FLAGS 		:= "-march=armv7-a -mfloat-abi=hard -mfpu=neon -ffunction-sections -fdata-sections -O2"
else
ifeq ($(PLATFORM_TYPE), hi3536a)
MS_PLATFORM_FLAG		:= 4
CROSS_PREFIX			:= aarch64-mix410-linux
HI_PLATFORM_FLAGS 		:= "-march=armv8-a -ffunction-sections -fdata-sections -O2"
else
ifeq ($(PLATFORM_TYPE), nt98633)
MS_PLATFORM_FLAG		:= 5
CROSS_PREFIX			:= aarch64-ca53-linux-gnu
HI_PLATFORM_FLAGS 		:= "-march=armv8-a -mtune=cortex-a53 -ftree-vectorize -fno-builtin -fno-common -Wformat=1"
else
CROSS_PREFIX			:= arm-hisiv400-linux
HI_PLATFORM_FLAGS 		:= "-march=armv7-a -mfloat-abi=softfp -mfpu=neon-vfpv4 -ffunction-sections -mno-unaligned-access -fno-aggressive-loop-optimizations -O2"
endif
endif
endif
endif
endif
endif

MS_RTSP_LIB_COMPILE_OPT := $(MS_MAKE_JOBS) -C $(MS_RTSP_LIB_SRC_DIR) HISI_CROSS_TYPE=$(CROSS_PREFIX) INSTALL_DIR=$(MS_RTSP_LIB_TARGET_DIR) PLATFORM_FLAGS=$(HI_PLATFORM_FLAGS) HEADFILE_INSTALL_DIR=$(MS_RTSP_INC_SRC_DIR) RELEASE=1 MSPLATFORM=$(MS_PLATFORM_FLAG)

libmsrtsp:
#	$(shell mkdir $(MS_RTSP_LIB_TARGET_DIR))
	@echo "===== 编译msrtsp:$(PLATFORM_TYPE) start========"
	$(MS_MAKEFILE_V)$(MAKE) $(MS_RTSP_LIB_COMPILE_OPT) all
	#@echo "===== 编译msrtsp:$(PLATFORM_TYPE) clean========"
	#$(MS_MAKEFILE_V)$(MAKE) $(MS_RTSP_LIB_COMPILE_OPT) clean
	@echo "===== 编译msrtsp:$(PLATFORM_TYPE) end========"
	
libmsrtsp-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(MS_RTSP_LIB_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
LIBS   += libmsrtsp
LIBSCLEAN += libmsrtsp-clean
CLEANS += libmsrtsp-clean
