######################################################################
## Filename:      tde.mk
## Author:        zbing
## Created at:    2015-10-20
##                
## Description:   编译 tde lib
##
## Copyright (C)  milesight
##                
######################################################################
#源码目录
MSTDE_LIB_SRC_DIR			:= $(SRC_DIR)/libs/tde/$(PLATFORM_TYPE)
#输出目录
MSTDE_LIB_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/tde
#目标名称
MSTDE_LIB_TARGET_NAME	:= libmstde.so

MSTDE_LIB_COMPILE_OPT 	:= $(MS_MAKE_JOBS) -C $(MSTDE_LIB_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(MSTDE_LIB_TARGET_DIR) TARGET_NAME=$(MSTDE_LIB_TARGET_NAME) INSTALL_DIR=$(PUBLIC_LIB_DIR)

libmstde:
	$(MS_MAKEFILE_V)$(MAKE) $(MSTDE_LIB_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_LIB_DIR)/$(MSTDE_LIB_TARGET_NAME)
	
libmstde-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(MSTDE_LIB_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
LIBS   += libmstde
LIBSCLEAN += libmstde-clean
CLEANS += libmstde-clean


