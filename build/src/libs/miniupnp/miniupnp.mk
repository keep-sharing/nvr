######################################################################
## Filename:      miniupnp.mk
## Author:        eirc
## Created at:    2016.05.29
##                
## Description:   编译 miniupnp
## Copyright (C)  milesight
##                
######################################################################
#源码目录
MINIUPNP_SRC_DIR	:= $(SRC_DIR)/libs/miniupnpc-1.8.20131209

#输出目录
MINIUPNP_TARGET_DIR	:= $(TEMP_TARGET_DIR)/libs/miniupnp
MINIUPNP_FLAGS		:= CC=$(TARGET_CC) CFLAGS='-O2 -fPIC -Wstrict-prototypes -fno-common -DMINIUPNPC_SET_SOCKET_TIMEOUT -DMINIUPNPC_GET_SRC_ADDR -D_BSD_SOURCE -D_POSIX_C_SOURCE=1 -ansi $(PLATFORM_FLAGS)' DESTDIR=$(MINIUPNP_TARGET_DIR)

#目标名称
MINIUPNP_TARGET_NAME	:= libminiupnpc

miniupnp: miniupnp-clean
	$(MS_MAKEFILE_V)printf "$(COMPILE_P) $(MINIUPNP_TARGET_NAME) $(NOCOLOR)\n"
	-$(MS_MAKEFILE_V)mkdir -p $(MINIUPNP_TARGET_DIR)
	$(MS_MAKEFILE_V)$(MAKE) -j8 $(MINIUPNP_FLAGS) -C $(MINIUPNP_SRC_DIR) all
	$(MS_MAKEFILE_V)$(MAKE) -j8 $(MINIUPNP_FLAGS) -C $(MINIUPNP_SRC_DIR) install
	$(MS_MAKEFILE_V)cp -av $(MINIUPNP_TARGET_DIR)/usr/lib/$(MINIUPNP_TARGET_NAME).a $(PUBLIC_STATIC_LIB_DIR)/
	$(MS_MAKEFILE_V)printf  "$(COMPILE_SUCCESS_P) $(MINIUPNP_TARGET_NAME) success $(NOCOLOR)\n"

miniupnp-clean:
	$(MS_MAKEFILE_V)printf "$(CLEAN_P) $(MINIUPNP_TARGET_NAME) $(NOCOLOR)\n"
	$(MS_MAKEFILE_V)$(MAKE) -j8 -s $(MINIUPNP_FLAGS) -C $(MINIUPNP_SRC_DIR) distclean
	-$(MS_MAKEFILE_V)rm -rf $(MINIUPNP_TARGET_DIR)
	$(MS_MAKEFILE_V)printf  "$(CLEAN_SUCCESS_P) $(MINIUPNP_TARGET_NAME) success $(NOCOLOR)\n"

miniupnp-distclean: miniupnp-clean
	-$(MS_MAKEFILE_V)rm -rf $(PUBLIC_LIB_DIR)/$(MINIUPNP_TARGET_NAME)*
	-$(MS_MAKEFILE_V)rm -rf $(MINIUPNP_TARGET_DIR)/* $(PUBLIC_STATIC_LIB_DIR)/$(MINIUPNP_TARGET_NAME)*.a
	
#添加至全局LIBS/CLEANS
PREBUILD_LIBS	+= miniupnp
PREBUILD_CLEANS	+= miniupnp-clean


