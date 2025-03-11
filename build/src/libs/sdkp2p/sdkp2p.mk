######################################################################
## Filename:      sdkp2p.mk
## Author:        hrz.milesight
## Created at:    2019-04-08
##                
## Description:   编译 sdkp2p lib
##
## Copyright (C)  milesight
##                
######################################################################
#源码目录
SDKP2P_LIB_SRC_DIR	:= $(SRC_DIR)/libs/sdkp2p
#输出目录
SDKP2P_LIB_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/sdkp2p
#目标名称
SDKP2P_LIB_TARGET_NAME	:= libsdkp2p.so

SDKP2P_LIB_COMPILE_OPT 	:= $(MS_MAKE_JOBS) -C $(SDKP2P_LIB_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(SDKP2P_LIB_TARGET_DIR) TARGET_NAME=$(SDKP2P_LIB_TARGET_NAME) INSTALL_DIR=$(PUBLIC_LIB_DIR)

sdkp2p:
	$(MS_MAKEFILE_V)$(MAKE) $(SDKP2P_LIB_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_LIB_DIR)/$(SDKP2P_LIB_TARGET_NAME)
	
sdkp2p-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(SDKP2P_LIB_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
LIBS   += sdkp2p
LIBSCLEAN += sdkp2p-clean
CLEANS += sdkp2p-clean


