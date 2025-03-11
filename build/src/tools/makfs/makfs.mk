######################################################################
## Filename:      makfs.mk
## Author:        eric@milesight.cn
## Created at:    2014-04-29
##                
## Description:   编译 rootfs 的全局配置
##                1.定义源码目录
##                2.定义输出目录
##                3.定义目标名称
## Copyright (C)  milesight
##                
######################################################################

#源码目录
ROOTFS_SRC_DIR    		:= $(TOOLS_DIR)/$(PLATFORM_TYPE)/makfs

#目标名称
ROOTFS_TARGET_NAME		:= rootfs
NSFROOT_TARGET_NAME		:= nfsroot

.PHONY: rootfs rootfs-clean nfscpy nfsroot nfsroot-clean

rootfs:
	$(MS_MAKEFILE_V)printf "$(COMPILE_P) $(ROOTFS_TARGET_NAME) $(NOCOLOR)\n"
	$(MS_MAKEFILE_V)$(MAKE) $(MS_MAKE_PARA) -C $(ROOTFS_SRC_DIR) all
	$(MS_MAKEFILE_V)printf  "$(COMPILE_SUCCESS_P) $(ROOTFS_TARGET_NAME) success $(NOCOLOR)\n"

rootfs-clean:
	$(MS_MAKEFILE_V)printf "$(CLEAN_P) $(ROOTFS_TARGET_NAME) $(NOCOLOR)\n"
	$(MS_MAKEFILE_V)$(MAKE) $(MS_MAKE_PARA) -C $(ROOTFS_SRC_DIR) clean
	$(MS_MAKEFILE_V)printf  "$(CLEAN_SUCCESS_P) $(ROOTFS_TARGET_NAME) success $(NOCOLOR)\n"

nfscpy:
	$(MS_MAKEFILE_V)printf "$(COMPILE_P) $(NSFROOT_TARGET_NAME) $(NOCOLOR)\n"	
	$(MS_MAKEFILE_V)$(MAKE) $(MS_MAKE_PARA) -C $(ROOTFS_SRC_DIR) nfsroot
	$(MS_MAKEFILE_V)printf  "$(COMPILE_SUCCESS_P) $(NSFROOT_TARGET_NAME) success $(NOCOLOR)\n"	

nfsroot:	
	$(MS_MAKEFILE_V)printf "$(COMPILE_P) $(NSFROOT_TARGET_NAME) $(NOCOLOR)\n"
	$(MS_MAKEFILE_V)$(MAKE) $(MS_MAKE_PARA) -C $(ROOTFS_SRC_DIR) nfsroot
	$(MS_MAKEFILE_V)printf  "$(COMPILE_SUCCESS_P) $(NSFROOT_TARGET_NAME) success $(NOCOLOR)\n"

nfsroot-clean:
	$(MS_MAKEFILE_V)printf "$(CLEAN_P) $(NSFROOT_TARGET_NAME) $(NOCOLOR)\n"
	$(MS_MAKEFILE_V)$(MAKE) $(MS_MAKE_PARA) -C $(ROOTFS_SRC_DIR) nfsroot-clean
	$(MS_MAKEFILE_V)printf  "$(CLEAN_SUCCESS_P) $(NSFROOT_TARGET_NAME) success $(NOCOLOR)\n"
	
#添加至全局LIBS/CLEANS
SYS   		+= rootfs nfsroot
SYSCLEANS	+= 
DISTCLEANS	+= rootfs-clean nfsroot-clean


