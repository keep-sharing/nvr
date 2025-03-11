##
## rootfs/Makefile
##
## History:
##    2008/02/21 - [Anthony Ginger] Create
##
## Copyright (C) 2004-2009, Ambarella, Inc.
##
## All rights reserved. No Part of this file may be reproduced, stored
## in a retrieval system, or transmitted, in any form, or by any means,
## electronic, mechanical, photocopying, recording, or otherwise,
## without the prior consent of Ambarella, Inc.
##

default: all

PWD                     := $(shell pwd)
CPU_BIT_WIDTH           := $(shell uname -m)

include $(BUILD_DIR)/platform/*.mk 
#include $(SRC_DIR)/boards/hisi/config


BUILD_ROOTFS_DIR	:= $(PWD)
FAKEROOT_SRC_DIR	:= $(PLATFORM_DIR)/rootfs
FAKEROOT_DES_DIR	:= $(PWD)/fakeroot
FAKEROOT_DIR      := $(FAKEROOT_DES_DIR)

DEFAULTCP               := cp -dpRf
DEFAULTINSTALL		:= install -p -D -o root -m 0755 
DEFAULTRM               := rm -rf
DEFAULTLN               := ln -sf
DEFAULTTAR		:= tar -zcf

#CONFIG_NFS_MOUNT_CMD	:= 192.168.9.32:/home/nfsroot nfs rw,tcp,intr,nolock
ROOTFS_TABLE_NAME	:= _fakeroot.ubifs


# config for busybox
BUSYBOX_SYSROOT         := $(SYS_LIB_DIR)/..
BUSYBOX_CC              := $(TARGET_CC)
BUSYBOX_CROSS_COMPILE  	:= $(TARGET_CROSS)
BUSYBOX_CROSS           := $(TARGET_CROSS)
BUSYBOX_BUILD_CFLAGS    := -pipe -O3 $(call cc-option,-mno-unaligned-accesss,)
BUSYBOX_BUILD_CFLAGS    += -w -mcpu=cortex-a53 -fstack-protector-strong -fPIE
BUSYBOX_EXTRA_CFLAGS    := "$(BUSYBOX_BUILD_CFLAGS)"
BUSYBOX_OUT_DIR		:= $(BUILD_ROOTFS_DIR)/busybox
BUSYBOX_DIR             := $(BUILD_ROOTFS_DIR)/../busybox
BUSYBOX_TMP_CONFIG		:= _fakeroot.busybox
BUSYBOX_BUILD_CONFIG    := $(BUSYBOX_DIR)/config_aarch64_a53_softfp_neon
BUSYBOX_TOTALL_FLAGS	:= CC=$(BUSYBOX_CC) CROSS_COMPILE=$(BUSYBOX_CROSS_COMPILE) CROSS=$(BUSYBOX_CROSS) ARCH=arm EXTRA_CFLAGS=$(BUSYBOX_EXTRA_CFLAGS) PREFIX=$(BUSYBOX_OUT_DIR) -C $(BUSYBOX_DIR)

### use 128k/block 2k/page
#CONFIG_NAND_P2K_E128K	:= y
#BSP_STR                 := a5sipcam
#BOARD_VERSION_STR	:= A5SIPCAM_AUTO

MTD_PAGESIZE             := 0x0200
MTD_PEBSIZE              := 0x4000
UBI_LEBSIZE              := 0x3C00
UBI_SUBPAGESIZE		 := 0x0200
UBIFSOUTFLAG		 := -F -o

ifeq ($(CONFIG_NAND_P512_E16K), y)
MTD_PAGESIZE             := 0x0200
MTD_PEBSIZE              := 0x4000
UBI_LEBSIZE              := 0x3C00
endif
ifeq ($(CONFIG_NAND_P2K_E64K), y)
MTD_PAGESIZE             := 0x00800
MTD_PEBSIZE              := 0x10000
UBI_LEBSIZE              := 0x0F000
UBI_SUBPAGESIZE		 := 0x0800
endif
ifeq ($(CONFIG_NAND_P2K_E128K), y)
MTD_PAGESIZE              := 0x00800
MTD_PEBSIZE               := 0x20000
UBI_LEBSIZE               := 0x1F000
UBI_SUBPAGESIZE		  := 0x0800
endif
ifeq ($(CONFIG_NAND_P4K_E64K), y)
MTD_PAGESIZE             := 0x01000
MTD_PEBSIZE              := 0x10000
UBI_LEBSIZE              := 0x0E000
UBI_SUBPAGESIZE		 := 0x10000
endif
ifeq ($(CONFIG_NAND_P4K_E128K), y)
MTD_PAGESIZE             := 0x01000
MTD_PEBSIZE              := 0x20000
UBI_LEBSIZE              := 0x1E000
UBI_SUBPAGESIZE		 := 0x10000
endif

#debug 
#ifeq ($(CONFIG_NAND_P512_E16K), y)
MTD_PAGESIZE              := 0x00800
MTD_PEBSIZE               := 0x20000
UBI_LEBSIZE               := 0x1F000
UBI_SUBPAGESIZE		 			 	:= 0x0800
ifeq ($(CPU_BIT_WIDTH), x86_64)
DEFAULTMAKEDEVS         := $(BUILD_ROOTFS_DIR)/bin/makedevs-64
DEFAULTMKUBIFS          := $(BUILD_ROOTFS_DIR)/bin/mkfs.ubifs-64
DEFAULTUBINIZE          := $(BUILD_ROOTFS_DIR)/bin/ubinize-64
else
DEFAULTMAKEDEVS         := $(BUILD_ROOTFS_DIR)/bin/makedevs-32
DEFAULTMKUBIFS          := $(BUILD_ROOTFS_DIR)/bin/mkfs.ubifs-32
DEFAULTUBINIZE          := $(BUILD_ROOTFS_DIR)/bin/ubinize-32
endif

DEFAULTMKUBIFS          := $(BUILD_ROOTFS_DIR)/ubifs_tool/mkfs.ubifs
DEFAULTUBINIZE          := $(BUILD_ROOTFS_DIR)/ubifs_tool/ubinize

DEFAULTMKEXT4FS          := $(BUILD_ROOTFS_DIR)/extfs_tool/make_ext4fs

FAKE_SIZE		:= $(shell du -s $(FAKEROOT_DIR) | cut -f1) 
FAKE_COUNT		:= $(shell expr $(FAKE_SIZE) \* 2)  #$(FAKE_SIZE)*1024/512

.PHONY: all clean cp_nfsroot nfsroot nfsroot-clean cp_fakeroot ubifs ext4fs
.PHONY: prepare_busybox busybox busybox-clean busybox-distclean busybox_menuconfig

all: ext4fs

prepare_busybox:
	@sed s,^CONFIG_PREFIX=.*,CONFIG_PREFIX=\"$(BUSYBOX_OUT_DIR)\", $(BUSYBOX_BUILD_CONFIG) > $(BUSYBOX_TMP_CONFIG)
	@sed s,"CONFIG_STATIC=y","# CONFIG_STATIC is not set",g $(BUSYBOX_TMP_CONFIG) > $(BUSYBOX_DIR)/.config
	@mkdir -p $(BUSYBOX_OUT_DIR)
	-@$(DEFAULTRM) $(BUSYBOX_TMP_CONFIG)
	
busybox_menuconfig: prepare_busybox
	@$(MAKE) $(MS_MAKE_JOBS) $(BUSYBOX_TOTALL_FLAGS) menuconfig
	@$(DEFAULTCP) $(BUSYBOX_DIR)/.config $(BUSYBOX_BUILD_CONFIG)
	
busybox: prepare_busybox
	@$(MAKE) $(MS_MAKE_JOBS) $(BUSYBOX_TOTALL_FLAGS) 
	@$(MAKE) $(MS_MAKE_JOBS) $(BUSYBOX_TOTALL_FLAGS) install
	#cp -av $(BUSYBOX_OUT_DIR)/* $(FAKEROOT_SRC_DIR)/
	#cp -av $(BUSYBOX_OUT_DIR)/* $(NSFROOT_DIR)/
	
busybox-clean:
	@$(MAKE) -C $(BUSYBOX_DIR) clean
	-@$(DEFAULTRM) $(BUSYBOX_OUT_DIR)
	
busybox-distclean:
	@$(MAKE) -C $(BUSYBOX_DIR) distclean
	-@$(DEFAULTRM) $(BUSYBOX_OUT_DIR)
	
		
cp_fakeroot:
	-@$(DEFAULTRM) $(FAKEROOT_DIR)
	-@$(DEFAULTRM) $(FAKEROOT_DIR)
	@mkdir -p $(FAKEROOT_DIR)
	@$(DEFAULTCP) $(FAKEROOT_SRC_DIR)/*  $(FAKEROOT_DIR)/
	@mkdir -p $(FAKEROOT_DIR)/tmp/udev
	#@$(TARGET_STRIP) $(PUBLIC_APP_DIR)/*
	@echo "$(DEFAULTCP) $(PUBLIC_LIB_DIR)/* -> $(FAKEROOT_DIR)/opt/app/libs/"
	-@$(DEFAULTCP)  $(PUBLIC_LIB_DIR)/* $(FAKEROOT_DIR)/opt/app/libs/
	@echo "$(DEFAULTCP) $(PUBLIC_APP_DIR)/* -> $(FAKEROOT_DIR)/opt/app/bin/"
	-@$(DEFAULTCP) $(PUBLIC_APP_DIR)/* $(FAKEROOT_DIR)/opt/app/bin/
	#@echo "$(DEFAULTCP) $(FAKEROOT_DIR)/opt/app/bin/logo.jpg $(FAKEROOT_DIR)/opt/app/bin/logo.jpg_bk"
	#-@$(DEFAULTCP) $(FAKEROOT_DIR)/opt/app/bin/logo.jpg $(FAKEROOT_DIR)/opt/app/bin/logo.jpg_bk
	#@echo "$(DEFAULTCP) $(FAKEROOT_DIR)/opt/app/bin/gui.rcc $(FAKEROOT_DIR)/opt/app/bin/gui.rcc_bk"
	#-@$(DEFAULTCP) $(FAKEROOT_DIR)/opt/app/bin/gui.rcc $(FAKEROOT_DIR)/opt/app/bin/gui.rcc_bk
	@echo "$(DEFAULTCP) $(FAKEROOT_SRC_DIR)/opt/app/db/* -> $(FAKEROOT_DIR)/opt/app/db/"
	-@$(DEFAULTCP) $(FAKEROOT_SRC_DIR)/opt/app/db/* $(FAKEROOT_DIR)/opt/app/db/
	@echo "$(DEFAULTCP) $(MODULE_INSTALL_DIR)/* -> $(FAKEROOT_DIR)/"
	-@$(DEFAULTCP) $(MODULE_INSTALL_DIR)/* $(FAKEROOT_DIR)/
	@find $(FAKEROOT_DIR)/ -name ".svn" 2> /dev/null | xargs rm -rf {} ;
	@sed -i s,"ttyS000::respawn:/bin/login -f root","ttyS000::respawn:/bin/login",g $(FAKEROOT_DIR)/etc/inittab
	@find $(FAKEROOT_DIR) -type d | xargs chmod 755
	@find $(FAKEROOT_DIR) -type f -name *.so | xargs chmod 755
	@find $(FAKEROOT_DIR) -type f -name *.so.* | xargs chmod 755
	-@chmod a+x $(FAKEROOT_DIR)/bin/* $(FAKEROOT_DIR)/sbin/*  $(FAKEROOT_DIR)/bin/* $(FAKEROOT_DIR)/usr/bin/* $(FAKEROOT_DIR)/usr/sbin/* #> /dev/null
	-@chmod a+x $(FAKEROOT_DIR)/usr/local/bin/* $(FAKEROOT_DIR)/opt/app/bin/*  #> /dev/null
	@chmod 777 $(FAKEROOT_DIR)
	cd $(FAKEROOT_DIR)/

ubifs:  cp_fakeroot
	@$(DEFAULTRM) $(ROOTFS_TABLE_NAME)
	@touch $(ROOTFS_TABLE_NAME)
	@echo "chown -R 0:0 $(FAKEROOT_DIR)" >> $(ROOTFS_TABLE_NAME)
	#@echo "$(DEFAULTMAKEDEVS) -d $(BUILD_ROOTFS_DIR)/device_table.txt $(FAKEROOT_DIR)" >> $(ROOTFS_TABLE_NAME)
	@echo "$(DEFAULTMKUBIFS) -r $(FAKEROOT_DIR) -m $(MTD_PAGESIZE) -e $(UBI_LEBSIZE) -c 1024 $(UBIFSOUTFLAG) $(BUILD_ROOTFS_DIR)/ubifs.img" >> $(ROOTFS_TABLE_NAME)
	@echo "$(DEFAULTUBINIZE) -o $(BUILD_ROOTFS_DIR)/ubifs -p $(MTD_PEBSIZE) -m $(MTD_PAGESIZE) -s $(UBI_SUBPAGESIZE)  ubicfg.ini" >> $(ROOTFS_TABLE_NAME)
	@chmod +x $(ROOTFS_TABLE_NAME)
	@fakeroot -- $(BUILD_ROOTFS_DIR)/$(ROOTFS_TABLE_NAME)
	@$(DEFAULTCP) $(BUILD_ROOTFS_DIR)/ubifs $(ROOTFS_IMG_DIR)/ubifs
	-@$(DEFAULTRM) $(ROOTFS_TABLE_NAME) $(TARGET_EXT_CMD) $(TMP_EXT_CMD) ubifs.img ubifs $(FAKEROOT_DIR)
	
ext4fs:  cp_fakeroot
	$(DEFAULTMKEXT4FS) -l 256M -s $(BUILD_ROOTFS_DIR)/ext4fs $(FAKEROOT_DIR)
	@$(DEFAULTCP) $(BUILD_ROOTFS_DIR)/ext4fs $(ROOTFS_IMG_DIR)/ext4fs
	pushd $(FAKEROOT_DIR);tar -czf ext4fs.tar.tgz *; mv ext4fs.tar.tgz $(BUILD_ROOTFS_DIR);popd;
	@$(DEFAULTCP) $(BUILD_ROOTFS_DIR)/ext4fs.tar.tgz $(ROOTFS_IMG_DIR)/ext4fs.tar.tgz
	-@$(DEFAULTRM) $(ROOTFS_TABLE_NAME) $(TARGET_EXT_CMD) $(TMP_EXT_CMD) ext4fs ext4fs.tar.tgz $(FAKEROOT_DIR)
	
cp_nfsroot:
ifneq ($(wildcard $(NSFROOT_DIR)),$(NSFROOT_DIR))
	-@mkdir -p $(NSFROOT_DIR)
	@$(DEFAULTCP) $(FAKEROOT_SRC_DIR)/*  $(NSFROOT_DIR)/
	@echo "$(DEFAULTCP) $(MODULE_INSTALL_DIR)/* -> $(NSFROOT_DIR)/"
	-@$(DEFAULTCP) $(MODULE_INSTALL_DIR)/* $(NSFROOT_DIR)/
	@find $(NSFROOT_DIR)/ -name ".svn" 2> /dev/null | xargs rm -rf {} ;
	#@$(DEFAULTMAKEDEVS) -d $(BUILD_ROOTFS_DIR)/device_table.txt $(NSFROOT_DIR)
	@mkdir -p $(NSFROOT_DIR)/tmp/udev	
endif
	
nfsroot: cp_nfsroot
	#-@$(TARGET_STRIP) $(PUBLIC_APP_DIR)/*
	@echo "$(DEFAULTCP) $(PUBLIC_LIB_DIR)/* -> $(NSFROOT_DIR)/opt/app/libs/"
	-@$(DEFAULTCP) $(PUBLIC_LIB_DIR)/* $(NSFROOT_DIR)/opt/app/libs/
	@echo "$(DEFAULTCP) $(PUBLIC_APP_DIR)/* -> $(NSFROOT_DIR)/opt/app/bin/"
	-@$(DEFAULTCP) $(PUBLIC_APP_DIR)/* $(NSFROOT_DIR)/opt/app/bin/
	@echo "$(DEFAULTCP) $(FAKEROOT_SRC_DIR)/opt/app/db/* -> $(NSFROOT_DIR)/opt/app/db/"
	-@$(DEFAULTCP) $(FAKEROOT_SRC_DIR)/opt/app/db/* $(NSFROOT_DIR)/opt/app/db/
	@find $(NSFROOT_DIR) -type d | xargs chmod 755
	@find $(NSFROOT_DIR) -type f -name *.so | xargs chmod 755
	@find $(NSFROOT_DIR) -type f -name *.so.* | xargs chmod 755
	-@chmod a+x $(NSFROOT_DIR)/bin/* $(NSFROOT_DIR)/sbin/*  $(NSFROOT_DIR)/bin/* $(NSFROOT_DIR)/usr/bin/* $(NSFROOT_DIR)/usr/sbin/* #> /dev/null
	-@chmod a+x $(NSFROOT_DIR)/usr/local/bin/* $(NSFROOT_DIR)/opt/app/bin/*  #> /dev/null
	@chmod 777 -R $(NSFROOT_DIR)
		
nfsroot-clean:
	-@$(DEFAULTRM) $(NSFROOT_DIR)
	
clean:
	-@$(DEFAULTRM) $(ROOTFS_TABLE_NAME) $(TARGET_EXT_CMD) $(TMP_EXT_CMD) ubifs.img ubifs $(FAKEROOT_DIR)
	-@$(DEFAULTRM) $(ROOTFS_IMG_DIR)/ubifs

	
