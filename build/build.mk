#以下变量在platform中具体指定
#平台类型
PLATFORM        :=
PLATFORM_NAME   :=

#交叉编译器
TARGET_CC       :=
TARGET_CPP      :=
TARGET_STRIP    :=
TARGET_AR       :=

#导入平台 Makefile,初始化相关变量.
include $(BUILD_DIR)/platform/*.mk



#存放编译生成的中间临时(.o .a)文件
TEMP_TARGET_DIR 		:= $(TOP_DIR)/temp/$(PLATFORM_NAME)

#存放公共头文件
PUBLIC_INC_DIR			:= $(SRC_DIR)/public/include

#存放DB升级文件
PUBLIC_DB_DIR			:= $(SRC_DIR)/db

#存放编译生成的静态库(.a)文件
PUBLIC_STATIC_LIB_DIR 	:= $(SRC_DIR)/public/lib/$(PLATFORM_TYPE)

#存放编译后的动态库(.so)文件(for make filesys)
PUBLIC_LIB_DIR  		:= $(TARGETS_DIR)/$(PLATFORM_NAME)/app/libs

#存放编译后的可执行(app)文件(for make filesys)
PUBLIC_APP_DIR  		:= $(TARGETS_DIR)/$(PLATFORM_NAME)/app/bin

#存放编译后的网页(cgi)文件(for make filesys)
PUBLIC_CGI_DIR 			:= $(TARGETS_DIR)/$(PLATFORM_NAME)/app/www

#存放编译后的系统(drivers)文件(for make filesys)
PUBLIC_SYS_DIR  		:= $(TARGETS_DIR)/$(PLATFORM_NAME)/sys

#以下变量在build/src目录的子mk中被赋值
#所有要编译的 lib
LIBS            :=

#所有要编译的 app
APPS            :=

#所有要编译的 sys
SYS             :=

#所有要编译的 app
LIBSCLEAN				:=

#所有要执行的 App clean
CLEANS			:=

#所有要执行的 Sys clean
SYSCLEANS		:=

#所有要执行的 distclean
DISTCLEANS		:=

#导入src mk,初始化相关变量
include $(BUILD_DIR)/src/src.mk

#编译所有的 lib
libs: $(LIBS)  
libs-clean:$(LIBSCLEAN)#msstd-clean disk-clean cellular-clean ovfcli-clean avilib-clean msdb-clean mssocket-clean libvapi-clean osa-clean encrypt-clean p2pcli-clean msptz-clean

#编译avserver以及它内部的库文件
#avserver: $(IAVLIBS) $(AVSERVER) nfscpy

#编译所有的 app
apps: $(LIBS)  $(APPS) nfscpy

#编译所有的 sys, make all, not include make hiboot, decrease compile time.
ifeq ($(PLATFORM_TYPE),$(PLATFORM_xxx))
sysall: amboot dbtar rootfs nfsroot firmware
else
ifeq ($(PLATFORM_TYPE),$(filter $(PLATFORM_TYPE), $(PLATFORM_ALL)))
sysall: fmtools dbtar rootfs nfsroot firmware
endif
endif

#编译所有的 libs apps sys
all: libs  apps sysall

#执行所有的 sys clean
ifeq ($(PLATFORM_TYPE),$(PLATFORM_xxx))
sysclean: amboot-clean rootfs-clean firmware-clean
else
ifeq ($(PLATFORM_TYPE),$(filter $(PLATFORM_TYPE), $(PLATFORM_ALL)))
sysclean:  rootfs-clean firmware-clean
endif
endif

#添加avserverclean，用于clean  avserver目录下的所有.a库和avserver可执行程序。
#avserverclean: $(AVSERVERCLEANS)

#执行所有的 libs and app clean
appsclean: $(CLEANS) 

#执行所有的  clean
distclean: $(CLEANS) $(DISTCLEANS)


.PHONY: appsclean sysclean distclean libs apps sysall all
	
