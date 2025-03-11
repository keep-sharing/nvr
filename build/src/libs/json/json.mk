######################################################################
## Filename:      json.mk
## Author:        bruce.milesight
## Created at:    2015-11-20
##                
## Description:   编译 json lib
##
## Copyright (C)  milesight
##                
######################################################################
#源码目录
JSON_LIB_SRC_DIR	:= $(SRC_DIR)/libs/json
JSON_CONFIG		= $(shell if [ ! -f $(JSON_LIB_SRC_DIR)/config.h ]; then echo "nofile"; else echo "exist"; fi;)
#输出目录
JSON_LIB_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/json
#目标名称
JSON_LIB_TARGET_NAME	:= libjson.so

JSON_LIB_COMPILE_OPT 	:= $(MS_MAKE_JOBS) -C $(JSON_LIB_SRC_DIR) CC=$(TARGET_CC) AR=$(TARGET_AR) TARGET_DIR=$(JSON_LIB_TARGET_DIR) TARGET_NAME=$(JSON_LIB_TARGET_NAME) INSTALL_DIR=$(PUBLIC_LIB_DIR)

json:
ifeq ($(JSON_CONFIG), nofile)
	cd $(JSON_LIB_SRC_DIR); ./configure CC=$(TARGET_CC) --prefix=$(JSON_LIB_TARGET_DIR) --host=$(LINUX_TOOLCHAIN_NAME) --enable-shared=yes --enable-static=no;make install;
endif
	$(MS_MAKEFILE_V)$(MAKE) $(JSON_LIB_COMPILE_OPT) all
	$(MS_MAKEFILE_V)cp -a $(JSON_LIB_TARGET_DIR)/lib/$(JSON_LIB_TARGET_NAME)* $(PUBLIC_LIB_DIR)/
json-clean:
	$(MS_MAKEFILE_V)$(MAKE) $(JSON_LIB_COMPILE_OPT) clean

#添加至全局LIBS/CLEANS
LIBS   += json
LIBSCLEAN += json-clean
CLEANS += json-clean


