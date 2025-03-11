######################################################################
## Filename:      backup.mk
## Author:        hugo
## Created at:    2015-10-20
##                
## Description:   编译中心程序backup
## Copyright (C)  milesight
##                
######################################################################
#源码目录
BACKUP_SRC_DIR    	:= $(SRC_DIR)/backup
#输出目录
BACKUP_TARGET_DIR 	:= $(TEMP_TARGET_DIR)/backup
#目标名称
BACKUP_TARGET_NAME	:= backup

BACKUP_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(BACKUP_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(BACKUP_TARGET_DIR) TARGET_NAME=$(BACKUP_TARGET_NAME) INSTALL_DIR=$(PUBLIC_APP_DIR)

backup:
	$(MS_MAKEFILE_V)$(MAKE) $(BACKUP_COMPILE_OPT) all
	
backup-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(BACKUP_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
#APPS   += backup
#CLEANS += backup-clean


