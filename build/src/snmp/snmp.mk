######################################################################
## Filename:      snmp.mk
## Author:        hrz
## Created at:    2019.07.01
##                
## Description:   编译 snmp
## Copyright (C)  milesight
##                
######################################################################
#源码目录
SNMP_SRC_DIR		:= $(SRC_DIR)/snmp

#输出目录
SNMP_TARGET_DIR	:= $(TEMP_TARGET_DIR)/snmp

#目标名称
SNMP_TARGET_NAME	:= snmpd

snmp: snmp-clean
	$(MS_MAKEFILE_V)printf "$(COMPILE_P) $(SNMP_TARGET_NAME) $(NOCOLOR)\n"
	-$(MS_MAKEFILE_V)mkdir -p $(SNMP_TARGET_DIR)
	cd $(SNMP_SRC_DIR); ./configure CC=$(TARGET_CC)  --prefix=$(SNMP_TARGET_DIR) --host=$(LINUX_TOOLCHAIN_NAME) --with-cc=$(TARGET_CC) --with-ndianness=little --disable-manuals --with-mib-modules='ucd-snmp/diskio ip-mib/ipv4InterfaceTable ip-mib/ipv6InterfaceTable'  --enable-as-needed --disable-embedded-perl --without-perl-modules --disable-snmptrapd-subagent --disable-applications --disable-scripts --with-default-snmp-version="3" --with-sys-contact="amos@milesight.com" --with-sys-location="china" --with-logfile="/var/log/snmpd.log" --with-persistent-directory="/var/net-snmp" --enable-ipv6
	$(MAKE) -j8 -s -C  $(SNMP_SRC_DIR)  LDFLAGS="-static"
	$(MS_MAKEFILE_V)$(MAKE) -s -C $(SNMP_SRC_DIR) install
	$(MS_MAKEFILE_V)$(TARGET_STRIP) $(SNMP_TARGET_DIR)/sbin/$(SNMP_TARGET_NAME)
	$(MS_MAKEFILE_V)cp -a $(SNMP_TARGET_DIR)/sbin/$(SNMP_TARGET_NAME) $(PUBLIC_APP_DIR)
	$(MS_MAKEFILE_V)printf  "$(COMPILE_SUCCESS_P) $(SNMP_TARGET_NAME) success $(NOCOLOR)\n"

snmp-clean:
	$(MS_MAKEFILE_V)printf "$(CLEAN_P) $(SNMP_TARGET_NAME) $(NOCOLOR)\n"
	$(MS_MAKEFILE_V) $(MAKE) -C $(SNMP_SRC_DIR) clean  
	$(MS_MAKEFILE_V)rm -rf $(SNMP_TARGET_DIR)
	$(MS_MAKEFILE_V)rm -rf $(PUBLIC_APP_DIR)/$(SNMP_TARGET_NAME)
	$(MS_MAKEFILE_V)printf  "$(CLEAN_SUCCESS_P) $(SNMP_TARGET_NAME) success $(NOCOLOR)\n"

snmp-distclean: snmp-clean
	-$(MS_MAKEFILE_V)rm -rf $(PUBLIC_LIB_DIR)/$(SNMP_TARGET_NAME)*
	-$(MS_MAKEFILE_V)rm -rf $(SNMP_TARGET_DIR)/*
	
#添加至全局LIBS/CLEANS
#APPS   += snmp
#CLEANS += snmp-clean

