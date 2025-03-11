######################################################################
## Filename:      fio.mk
## Author:        zbing
## Created at:    2016-01-18
##                
## Description:   编译fio
## Copyright (C)  milesight
##                
######################################################################
#源码目录
FIO_SRC_DIR    	:= $(SRC_DIR)/fio
#输出目录
FIO_TARGET_DIR 	:= $(TEMP_TARGET_DIR)/fio
#目标名称
FIO_TARGET_NAME	:= fio

FIO_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(FIO_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(FIO_TARGET_DIR) TARGET_NAME=$(FIO_TARGET_NAME) INSTALL_DIR=$(PUBLIC_APP_DIR)

fio:
	$(MS_MAKEFILE_V)$(MAKE) $(FIO_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_APP_DIR)/$(FIO_TARGET_NAME)
	
fio-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(FIO_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
APPS   += fio
CLEANS += fio-clean


