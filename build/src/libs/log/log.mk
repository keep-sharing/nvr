######################################################################
## Filename:      log.mk
## Author:        bruce.milesight
## Created at:    2015-11-06
##                
## Description:   编译 log lib
##
## Copyright (C)  milesight
##                
######################################################################
#源码目录
LOG_LIB_SRC_DIR	:= $(SRC_DIR)/libs/log
#输出目录
LOG_LIB_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/log
#目标名称
LOG_LIB_TARGET_NAME	:= liblog.so

LOG_LIB_COMPILE_OPT 	:= $(MS_MAKE_JOBS) -C $(LOG_LIB_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(LOG_LIB_TARGET_DIR) TARGET_NAME=$(LOG_LIB_TARGET_NAME) INSTALL_DIR=$(PUBLIC_LIB_DIR)

log:
	$(MS_MAKEFILE_V)$(MAKE) $(LOG_LIB_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_LIB_DIR)/$(LOG_LIB_TARGET_NAME)
	
log-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(LOG_LIB_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
LIBS   += log
LIBSCLEAN += log-clean
CLEANS += log-clean


