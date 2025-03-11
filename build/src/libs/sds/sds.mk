######################################################################
## Filename:      sds.mk
## Author:        lhy
## Created at:    2022-08-01
##                
## Description:   编译 sds
##
## Copyright (C)  milesight
##                
######################################################################
#源码目录
SDS_LIB_SRC_DIR	:= $(SRC_DIR)/libs/sds
#输出目录
SDS_LIB_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/sds
#目标名称
SDS_LIB_TARGET_NAME	:= libsds.so

SDS_LIB_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(SDS_LIB_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(SDS_LIB_TARGET_DIR) TARGET_NAME=$(SDS_LIB_TARGET_NAME) INSTALL_DIR=$(PUBLIC_LIB_DIR)

sds:
	$(MS_MAKEFILE_V)$(MAKE) $(SDS_LIB_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_LIB_DIR)/$(SDS_LIB_TARGET_NAME)
	
sds-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(SDS_LIB_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
LIBS += sds
LIBSCLEAN += sds-clean
CLEANS += sds-clean


