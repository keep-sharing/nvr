######################################################################
## Filename:      webfile.mk
## Author:        bruce.milesight
## Created at:    2015-11-20 11:43:00
##
## Description:   编译web的全局配置
##                1.定义源码目录
##                2.定义输出目录
##                3.定义目标名称
## Copyright (C)  milesight
##
######################################################################
#源码目录
WEBHTML_DIR	  := $(SRC_DIR)/web/webfile

webfile:
	cp -a $(WEBHTML_DIR)/ $(PUBLIC_CGI_DIR)/
webfile-clean:
	rm -rf $(PUBLIC_CGI_DIR)/webfile
#添加至全局LIBS/CLEANS
APPS   += webfile
CLEANS += webfile-clean
