######################################################################
## Filename:      mkdosfs.mk
## Author:        eric
## Created at:    2014-07-15
##                
## Description:   编译 mkdosfs 的全局配置
## Copyright (C)  milesight
##                
######################################################################
#源码目录
MKDOSFS_SRC_DIR    	:= $(SRC_DIR)/test/mkdosfs
#输出目录
MKDOSFS_TARGET_DIR 	:= $(TEMP_TARGET_DIR)/test/mkdosfs
#目标名称
MKDOSFS_TARGET_NAME	:= mkdosfs

MKDOSFS_COMPILE_OPT	:= $(MS_MAKE_JOBS) -C $(MKDOSFS_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(MKDOSFS_TARGET_DIR) TARGET_NAME=$(MKDOSFS_TARGET_NAME) INSTALL_DIR=$(PUBLIC_APP_DIR)

mkdosfs:
	$(MS_MAKEFILE_V)$(MAKE) $(MKDOSFS_COMPILE_OPT) all
	$(TARGET_STRIP) $(PUBLIC_APP_DIR)/$(MKDOSFS_TARGET_NAME)
	
mkdosfs-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(MKDOSFS_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
APPS   += mkdosfs
CLEANS += mkdosfs-clean


