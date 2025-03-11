######################################################################
## Filename:      dbtar.mk
## Author:        wcm
## Created at:    2021-09-10
##                
## Description:   打包数据库升级文件
## Copyright (C)  milesight
##                
######################################################################

#源文件目录
#DB_TAR_SRC_DIR    	:= $(SRC_DIR)/db
DB_SRC_DIR_1025 	:= $(PUBLIC_DB_DIR)/$(PLATFORM_NAME)

#输出目录
DB_TAR_TARGET_DIR   := $(PLATFORM_DIR)/rootfs/opt/app/db

#目标名称
DB_TAR_TARGET_NAME 	:= dbtar
DB_FILE_NAME 		:= db-*.txt
DB_TAR_NAME			:= db.tar.gz
DB_BAK_TAR_NAME 	:= db_bak.tar.gz

.PHONY: dbtar dbtar-clean

dbtar : 
	$(MS_MAKEFILE_V)printf "$(COMPILE_P) $(DB_TAR_TARGET_NAME) $(NOCOLOR)\n"	
	cp -dpRf $(DB_SRC_DIR_1025)/$(DB_FILE_NAME) $(TOP_DIR)
	cp -dpRf $(PUBLIC_DB_DIR)/$(DB_FILE_NAME) $(TOP_DIR)
	tar -czf $(DB_TAR_TARGET_DIR)/$(DB_TAR_NAME) $(DB_FILE_NAME)
	cp -dpRf $(DB_TAR_TARGET_DIR)/$(DB_TAR_NAME) $(DB_TAR_TARGET_DIR)/$(DB_BAK_TAR_NAME)
	rm -rf ./$(DB_FILE_NAME)
	$(MS_MAKEFILE_V)printf  "$(COMPILE_SUCCESS_P) $(DB_TAR_TARGET_NAME) success $(NOCOLOR)\n"	

dbtar-clean :
	$(MS_MAKEFILE_V)printf "$(CLEAN_P) $(DB_TAR_TARGET_NAME) $(NOCOLOR)\n"
	rm -rf $(DB_TAR_TARGET_DIR)/*
	rm -rf $(TOP_DIR)/$(DB_FILE_NAME)
	$(MS_MAKEFILE_V)printf  "$(CLEAN_SUCCESS_P) $(DB_TAR_TARGET_NAME) success $(NOCOLOR)\n"

#添加至全局LIBS/CLEANS
SYS   += dbtar
DISTCLEANS += dbtar-clean
