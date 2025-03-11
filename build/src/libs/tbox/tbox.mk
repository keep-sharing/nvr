######################################################################
## Filename:      tbox.mk
## Author:        zbing.milesight
## Created at:    2017-04-19
##                
## Description:   编译 tbox lib
##
## Copyright (C)  milesight
##                
######################################################################
#源码目录
TBOX_LIB_SRC_DIR	:= $(SRC_DIR)/libs/tbox
#输出目录
TBOX_LIB_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/tbox
#目标名称
TBOX_LIB_TARGET_NAME	:= libtbox.a

TBOX_LIB_COMPILE_OPT 	:= $(MS_MAKE_JOBS) -C $(TBOX_LIB_SRC_DIR) SDK=$(ARM_LINUX_TOOCHAIN_DIR) TARGET_DIR=$(TBOX_LIB_TARGET_DIR) TARGET_NAME=$(TBOX_LIB_TARGET_NAME) INSTALL_DIR=$(PUBLIC_STATIC_LIB_DIR)

libtbox-xmake:
	$(MS_MAKEFILE_V)$(MAKE) -C $(TBOX_LIB_SRC_DIR) xmake_install

libtbox:
	$(MS_MAKEFILE_V)$(MAKE) $(TBOX_LIB_COMPILE_OPT) all
	
libtbox-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(TBOX_LIB_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
LIBS   += libtbox
LIBSCLEAN += libtbox-clean
CLEANS += libtbox-clean


