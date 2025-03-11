######################################################################
## Filename:      msdb.mk
## Author:        bruce.milesight
## Created at:    2015-11-06
##                
## Description:   编译 msdb lib
##
## Copyright (C)  milesight
##                
######################################################################
#源码目录
MSDB_LIB_SRC_DIR	:= $(SRC_DIR)/libs/msdb
#输出目录
MSDB_LIB_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/msdb
#目标名称
MSDB_LIB_TARGET_NAME	:= libmsdb.so

MSDB_LIB_COMPILE_OPT 	:= $(MS_MAKE_JOBS) -C $(MSDB_LIB_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(MSDB_LIB_TARGET_DIR) TARGET_NAME=$(MSDB_LIB_TARGET_NAME) INSTALL_DIR=$(PUBLIC_LIB_DIR)

msdb:
	$(MS_MAKEFILE_V)$(MAKE) $(MSDB_LIB_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_LIB_DIR)/$(MSDB_LIB_TARGET_NAME)
	
msdb-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(MSDB_LIB_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
LIBS   += msdb
LIBSCLEAN += msdb-clean
CLEANS += msdb-clean


