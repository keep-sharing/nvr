######################################################################
## Filename:      encrypt.mk
## Author:        bruce.milesight
## Created at:    2015-11-06
##                
## Description:   编译 encrypt lib
##
## Copyright (C)  milesight
##                
######################################################################
#源码目录
ENCRYPT_LIB_SRC_DIR	:= $(SRC_DIR)/libs/encrypt
#输出目录
ENCRYPT_LIB_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/encrypt
#目标名称
ENCRYPT_LIB_TARGET_NAME	:= libencrypt.so

ENCRYPT_LIB_COMPILE_OPT 	:= $(MS_MAKE_JOBS) -C $(ENCRYPT_LIB_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(ENCRYPT_LIB_TARGET_DIR) TARGET_NAME=$(ENCRYPT_LIB_TARGET_NAME) INSTALL_DIR=$(PUBLIC_LIB_DIR)

encrypt:
	$(MS_MAKEFILE_V)$(MAKE) $(ENCRYPT_LIB_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_LIB_DIR)/$(ENCRYPT_LIB_TARGET_NAME)
	
encrypt-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(ENCRYPT_LIB_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
LIBS   += encrypt
LIBSCLEAN += encrypt-clean
CLEANS += encrypt-clean


