######################################################################
## Filename:      dhcpv6.mk
## Author:        hrz
## Created at:    2019.07.02
##                
## Description:   编译 dhcpv6
## Copyright (C)  milesight
##                
######################################################################
#源码目录
DHCPV6_SRC_DIR		:= $(SRC_DIR)/dhcpv6/

#输出目录
DHCPV6_TARGET_DIR	:= $(TEMP_TARGET_DIR)/dhcpv6

#目标名称
DHCPV6_TARGET_NAME	:= dhcp6c

#dhcpv6: dhcpv6-clean
dhcpv6:
	$(MS_MAKEFILE_V)printf "$(COMPILE_P) $(DHCPV6_TARGET_NAME) $(NOCOLOR)\n"
	-$(MS_MAKEFILE_V)mkdir -p $(DHCPV6_TARGET_DIR)
	cd $(DHCPV6_SRC_DIR); #./configure CC=$(TARGET_CC)  --prefix=$(DHCPV6_TARGET_DIR) --host=$(LINUX_TOOLCHAIN_NAME) LIBNL_CFLAGS="-I$(TEMP_TARGET_DIR)/libs/libnl/include" LIBNL_LIBS="-L$(TEMP_TARGET_DIR)/libs/libnl/lib -lnl" --with-gnu-ld
	$(MAKE) -j8 -s -C  $(DHCPV6_SRC_DIR)
	$(MS_MAKEFILE_V)$(MAKE) -s -C $(DHCPV6_SRC_DIR) install
	$(MS_MAKEFILE_V)$(TARGET_STRIP) $(DHCPV6_TARGET_DIR)/sbin/$(DHCPV6_TARGET_NAME)
	$(MS_MAKEFILE_V)cp -af $(DHCPV6_TARGET_DIR)/sbin/$(DHCPV6_TARGET_NAME) $(PUBLIC_APP_DIR)
	$(MS_MAKEFILE_V)printf  "$(COMPILE_SUCCESS_P) $(DHCPV6_TARGET_NAME) success $(NOCOLOR)\n"

dhcpv6-clean:
	$(MS_MAKEFILE_V)printf "$(CLEAN_P) $(DHCPV6_TARGET_NAME) $(NOCOLOR)\n"
	$(MS_MAKEFILE_V) $(MAKE) -C $(DHCPV6_SRC_DIR) clean  
	$(MS_MAKEFILE_V)rm -rf $(DHCPV6_TARGET_DIR)
	$(MS_MAKEFILE_V)rm -rf $(PUBLIC_APP_DIR)/$(DHCPV6_TARGET_NAME)
	$(MS_MAKEFILE_V)printf  "$(CLEAN_SUCCESS_P) $(DHCPV6_TARGET_NAME) success $(NOCOLOR)\n"

dhcpv6-distclean: dhcpv6-clean
	-$(MS_MAKEFILE_V)rm -rf $(PUBLIC_LIB_DIR)/$(DHCPV6_TARGET_NAME)*
	-$(MS_MAKEFILE_V)rm -rf $(DHCPV6_TARGET_DIR)/*
	
#添加至全局LIBS/CLEANS
#APPS   += dhcpv6
#CLEANS += dhcpv6-clean

