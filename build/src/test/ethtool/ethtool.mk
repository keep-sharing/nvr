######################################################################
## Filename:      ethtool.mk
## Author:        eric
## Created at:    2014-07-15
##                
## Description:   编译 ethtool 的全局配置
## Copyright (C)  milesight
##                
######################################################################
#源码目录
ETHTOOL_SRC_DIR    	:= $(SRC_DIR)/test/ethtool

#输出目录
ETHTOOL_TARGET_DIR 	:= $(TEMP_TARGET_DIR)/test/ethtool

#目标名称
ETHTOOL_TARGET_NAME	:= ethtool

ethtool-config:
	$(MS_MAKEFILE_V)if test ! -f $(ETHTOOL_SRC_DIR)/Makefile; then \
		cd $(ETHTOOL_SRC_DIR); chmod +x configure; ./configure --prefix=$(ETHTOOL_TARGET_DIR) --build=i386-linux --host=$(TARGET_HOST) --target=$(CROSS_COMPILE) CC=$(TARGET_CC)  LDFLAGS=-static ; fi

ethtool: ethtool-config
	$(MS_MAKEFILE_V)printf "$(COMPILE_P) $(ETHTOOL_TARGET_NAME) $(NOCOLOR)\n"
	-$(MS_MAKEFILE_V)mkdir  -p $(ETHTOOL_TARGET_DIR)
	$(MS_MAKEFILE_V)$(MAKE) -C $(ETHTOOL_SRC_DIR) distclean
	$(MS_MAKEFILE_V)$(MAKE) $(MS_MAKE_PARA) ethtool-config
	$(MS_MAKEFILE_V)$(MAKE) $(MS_MAKE_PARA) $(MS_MAKE_JOBS) -C $(ETHTOOL_SRC_DIR) #CC=$(TARGET_CC) CXX=$(TARGET_CXX) TARGET_DIR=$(ETHTOOL_TARGET_DIR) TARGET_NAME=$(ETHTOOL_TARGET_NAME)
	$(MS_MAKEFILE_V)$(TARGET_STRIP) $(ETHTOOL_SRC_DIR)/$(ETHTOOL_TARGET_NAME)
	$(MS_MAKEFILE_V)cp $(ETHTOOL_SRC_DIR)/$(ETHTOOL_TARGET_NAME) $(PUBLIC_APP_DIR)/$(ETHTOOL_TARGET_NAME)
	$(MS_MAKEFILE_V)printf  "$(COMPILE_SUCCESS_P) $(ETHTOOL_TARGET_NAME) success $(NOCOLOR)\n"

ethtool-clean:
	$(MS_MAKEFILE_V)printf "$(CLEAN_P) $(ETHTOOL_TARGET_NAME) $(NOCOLOR)\n"
	$(MS_MAKEFILE_V)$(MAKE) -C $(ETHTOOL_SRC_DIR) distclean TARGET_DIR=$(ETHTOOL_TARGET_DIR) TARGET_NAME=$(ETHTOOL_TARGET_NAME)
	-$(MS_MAKEFILE_V)rm -rf $(PUBLIC_APP_DIR)/$(ETHTOOL_TARGET_NAME)
	$(MS_MAKEFILE_V)printf  "$(CLEAN_SUCCESS_P) $(ETHTOOL_TARGET_NAME) success $(NOCOLOR)\n"

#添加至全局LIBS/CLEANS
APPS   += #ethtool
CLEANS += #ethtool-clean


