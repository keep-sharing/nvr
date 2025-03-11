######################################################################
## Filename:      pppd.mk
## Author:        hrz
## Created at:    2019.09.25
##                
## Description:   编译 ppp-2.4.5
## Copyright (C)  milesight
##                
######################################################################
#源码目录
PPP_SRC_DIR		:= $(SRC_DIR)/pppd/

#输出目录
PPP_TARGET_DIR	:= $(TEMP_TARGET_DIR)/pppd

#目标名称
PPP_TARGET_NAME	:= pppd

#pppd: pppd-clean
pppd:
	$(MS_MAKEFILE_V)printf "$(COMPILE_P) $(PPP_TARGET_NAME) $(NOCOLOR)\n"
	-$(MS_MAKEFILE_V)mkdir -p $(PPP_TARGET_DIR)
	cd $(PPP_SRC_DIR); ./configure --prefix=$(PPP_TARGET_DIR)
	$(MS_MAKEFILE_V)$(MAKE) -j8 CC=$(TARGET_CC) -C $(PPP_SRC_DIR)
	$(MS_MAKEFILE_V)printf "disabled Makefile strip(-s)\n"
	sed -i 's/-s//' $(PPP_SRC_DIR)chat/Makefile
	sed -i 's/-s//' $(PPP_SRC_DIR)pppd/Makefile
	sed -i 's/-s//' $(PPP_SRC_DIR)pppd/plugins/radius/Makefile
	sed -i 's/-s//' $(PPP_SRC_DIR)pppdump/Makefile
	sed -i 's/-s//' $(PPP_SRC_DIR)pppstats/Makefile
	$(MS_MAKEFILE_V)$(MAKE) -C $(PPP_SRC_DIR) install
	$(MS_MAKEFILE_V)$(TARGET_STRIP) $(PPP_TARGET_DIR)/sbin/$(PPP_TARGET_NAME)
	$(MS_MAKEFILE_V)cp -af $(PPP_TARGET_DIR)/sbin/$(PPP_TARGET_NAME) $(PUBLIC_APP_DIR)
	$(MS_MAKEFILE_V)printf  "$(COMPILE_SUCCESS_P) $(PPP_TARGET_NAME) success $(NOCOLOR)\n"

pppd-clean:
	$(MS_MAKEFILE_V)printf "$(CLEAN_P) $(PPP_TARGET_NAME) $(NOCOLOR)\n"
	$(MS_MAKEFILE_V) $(MAKE) -C $(PPP_SRC_DIR) clean  
	$(MS_MAKEFILE_V)rm -rf $(PPP_TARGET_DIR)
	$(MS_MAKEFILE_V)rm -rf $(PUBLIC_APP_DIR)/$(PPP_TARGET_NAME)
	$(MS_MAKEFILE_V)printf  "$(CLEAN_SUCCESS_P) $(PPP_TARGET_NAME) success $(NOCOLOR)\n"

pppd-distclean: pppd-clean
	-$(MS_MAKEFILE_V)rm -rf $(PUBLIC_LIB_DIR)/$(PPP_TARGET_NAME)*
	-$(MS_MAKEFILE_V)rm -rf $(PPP_TARGET_DIR)/*
	
#添加至全局LIBS/CLEANS
#APPS   += pppd
#CLEANS += pppd-clean

