######################################################################
## Filename:      recortsp.mk
## Author:        david.milesight
## Created at:    2021-06-30
##                
## Description:   编译 recortsp lib
##
## Copyright (C)  milesight
## 
## Note：		  recortsp lib依赖media-server，要基于make libmsrtsp编译后编译               
######################################################################
#源码目录
RECORTSP_LIB_SRC_DIR	:= $(SRC_DIR)/libs/recortsp
#输出目录
RECORTSP_LIB_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/recortsp
#目标名称
RECORTSP_LIB_TARGET_NAME	:= librecortsp.so
RECORTSP_LIB_SO_NAME		:= librecortsp.so.1

RECORTSP_LIB_COMPILE_OPT 	:= $(MS_MAKE_JOBS) -C $(RECORTSP_LIB_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(RECORTSP_LIB_TARGET_DIR) TARGET_NAME=$(RECORTSP_LIB_TARGET_NAME) INSTALL_DIR=$(PUBLIC_LIB_DIR)

librecortsp:
	$(MS_MAKEFILE_V)$(MAKE) $(RECORTSP_LIB_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_LIB_DIR)/$(RECORTSP_LIB_TARGET_NAME)
	cd $(PUBLIC_LIB_DIR) && ln -sf $(RECORTSP_LIB_TARGET_NAME) $(RECORTSP_LIB_SO_NAME) 
	
librecortsp-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(RECORTSP_LIB_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
LIBS   += librecortsp
LIBSCLEAN += librecortsp-clean
CLEANS += librecortsp-clean


