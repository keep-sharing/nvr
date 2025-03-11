######################################################################
## Filename:      cellular.mk
## Author:        bruce.milesight
## Created at:    2015-11-09
##                
## Description:   编译 cellular lib
##
## Copyright (C)  milesight
##                
######################################################################
#源码目录
CELLULAR_LIB_SRC_DIR	:= $(SRC_DIR)/libs/cellular
#输出目录
CELLULAR_LIB_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/cellular
#目标名称
CELLULAR_LIB_TARGET_NAME	:= libcellular.so

CELLULAR_LIB_COMPILE_OPT 	:= $(MS_MAKE_JOBS) -C $(CELLULAR_LIB_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(CELLULAR_LIB_TARGET_DIR) TARGET_NAME=$(CELLULAR_LIB_TARGET_NAME) INSTALL_DIR=$(PUBLIC_LIB_DIR)

libcellular:
	$(MS_MAKEFILE_V)$(MAKE) $(CELLULAR_LIB_COMPILE_OPT) all
	
libcellular-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(CELLULAR_LIB_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
LIBS   += libcellular
LIBSCLEAN += libcellular-clean
CLEANS += libcellular-clean


