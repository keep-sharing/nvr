######################################################################
## Filename:      mscrypt.mk
## Author:        lin
## Created at:    2018-07-09
##                
## Description:   编译mscrypt
##
## Copyright (C)  milesight
##                
######################################################################
#源码目录
MSCRYPT_SRC_DIR		:= $(SRC_DIR)/libs/mscrypt
#输出目录
MSCRYPT_TARGET_DIR		:= $(TEMP_TARGET_DIR)/libs/mscrypt
#目标名称
MSCRYPT_TARGET_NAME	:= libmscrypt.so

MSCRYPT_COMPILE_OPT 	:= $(MS_MAKE_JOBS) -C $(MSCRYPT_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(MSCRYPT_TARGET_DIR) TARGET_NAME=$(MSCRYPT_TARGET_NAME) INSTALL_DIR=$(PUBLIC_LIB_DIR)

mscrypt:
	$(MS_MAKEFILE_V)$(MAKE) $(MSCRYPT_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_LIB_DIR)/$(MSCRYPT_TARGET_NAME)
	
mscrypt-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(MSCRYPT_COMPILE_OPT) clean
	

#添加至全局LIBS/CLEANS
LIBS   += mscrypt
LIBSCLEAN += mscrypt-clean
CLEANS += mscrypt-clean


