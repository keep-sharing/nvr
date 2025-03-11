######################################################################
## Filename:      mshw.mk
## Author:        zbing
## Created at:    2015-10-20
##                
## Description:   编译 mshw lib
##
## Copyright (C)  milesight
##                
######################################################################
#源码目录
MSHW_LIB_SRC_DIR			:= $(SRC_DIR)/libs/mshw
#输出目录
MSHW_LIB_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/mshw
#目标名称
MSHW_LIB_TARGET_NAME	:= libmshw.so

MSHW_LIB_COMPILE_OPT 	:= $(MS_MAKE_JOBS) -C $(MSHW_LIB_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(MSHW_LIB_TARGET_DIR) TARGET_NAME=$(MSHW_LIB_TARGET_NAME) INSTALL_DIR=$(PUBLIC_LIB_DIR)

libmshw:
	$(MS_MAKEFILE_V)$(MAKE) $(MSHW_LIB_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_LIB_DIR)/$(MSHW_LIB_TARGET_NAME)
	
libmshw-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(MSHW_LIB_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
LIBS   += libmshw
LIBSCLEAN += libmshw-clean
CLEANS += libmshw-clean


