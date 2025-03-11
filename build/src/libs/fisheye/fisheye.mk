######################################################################
## Filename:      fisheye.mk
## Author:        zbing.milesight
## Created at:    2020-03-05
##                
## Description:   编译 fisheye lib
##
## Copyright (C)  milesight
##                
######################################################################
#源码目录
FISHEYE_LIB_SRC_DIR	:= $(SRC_DIR)/libs/fisheye
#输出目录
FISHEYE_LIB_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/fisheye
#目标名称
FISHEYE_LIB_TARGET_NAME	:= libfisheye.so

FISHEYE_LIB_COMPILE_OPT 	:= $(MS_MAKE_JOBS) -C $(FISHEYE_LIB_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(FISHEYE_LIB_TARGET_DIR) TARGET_NAME=$(FISHEYE_LIB_TARGET_NAME) INSTALL_DIR=$(PUBLIC_LIB_DIR)

libfisheye:
	$(MS_MAKEFILE_V)$(MAKE) $(FISHEYE_LIB_COMPILE_OPT) all
	-$(TARGET_STRIP) $(PUBLIC_LIB_DIR)/$(FISHEYE_LIB_TARGET_NAME)
	
libfisheye-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(FISHEYE_LIB_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
LIBS   += libfisheye
LIBSCLEAN += libfisheye-clean
CLEANS += libfisheye-clean


