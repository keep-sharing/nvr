######################################################################
## Filename:      msfs.mk
## Author:        zbing.milesight
## Created at:    2017-04-19
##                
## Description:   编译 msfs lib
##
## Copyright (C)  milesight
##                
######################################################################
#源码目录
MSFS_LIB_SRC_DIR	:= $(SRC_DIR)/libs/msfs
#输出目录
MSFS_LIB_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/msfs
#目标名称
MSFS_LIB_TARGET_NAME	:= libmsfs.so

MSFS_LIB_COMPILE_OPT 	:= $(MS_MAKE_JOBS) -C $(MSFS_LIB_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(MSFS_LIB_TARGET_DIR) TARGET_NAME=$(MSFS_LIB_TARGET_NAME) INSTALL_DIR=$(PUBLIC_LIB_DIR)

libmsfs:
	$(MS_MAKEFILE_V)$(MAKE) $(MSFS_LIB_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_LIB_DIR)/$(MSFS_LIB_TARGET_NAME)
	
libmsfs-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(MSFS_LIB_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
LIBS   += libmsfs
LIBSCLEAN += libmsfs-clean
CLEANS += libmsfs-clean


