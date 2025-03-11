######################################################################
## Filename:      firmware Makefile
## Author:        zbing@milesight.cn
## Created at:    2015-05-20
##                
## Description:   ���� firmware Makefile��ȫ������
##                1.image package
##                2.image update
##                3.update tools
## Copyright (C)  milesight
######################################################################

FW_TOPDIR			:= $(shell pwd)

-include $(CONFIG_DIR)/ms_config

CROSS		:= $(CROSS_COMPILE)
AR			:= ar
CC			:= gcc
CPP			:= gcc -E
LD			:= ld
NM			:= nm
OBJCOPY	:= objcopy
RANLIB		:= ranlib
STRIP		:= strip
SED			:= sed
GCCDEP		:= gcc -M -P

MAKE_VERBOSE		= no

###############################################################
##	define firware version
###############################################################
ifeq ($(CONFIG_CPU_HI3536),y)
	VERSION_CPU 	= 7
endif
ifeq ($(CONFIG_CPU_HI3535),y)
	VERSION_CPU 	= 8
endif

VERSION_FS		= $(VERSION_CPU)1

## define nand flash version number
#deafult VERSION_FLASH 	= 2
VERSION_FLASH 	= 1
ifeq ($(CONFIG_NAND_SIZE_256MB),y)
	VERSION_FLASH 	= 1
endif
ifeq ($(CONFIG_NAND_SIZE_512MB),y)
	VERSION_FLASH 	= 2
endif

## define DDR version number
#default VERSION_DDR 	= 3
VERSION_DDR 	?= 3
DDR_NAME	?= 4G

DDR_SIZE_512M	= 512M
DDR3_VER_512M	= 0
DDR_SIZE_1G	= 1024M
DDR3_VER_1G	= 1
DDR_SIZE_2G	= 2048M
DDR3_VER_2G	= 2
DDR_SIZE_4G	= 4096M
DDR3_VER_4G	= 3


ifeq ($(CONFIG_DDR3_MEM_512M),y)
	VERSION_DDR 	= $(DDR3_VER_512M)
	DDR_SIZE	= $(DDR_SIZE_512M)
	DDR_NAME	= $(DDR_SIZE)
endif

ifeq ($(CONFIG_DDR2_MEM_1024M),y)
	VERSION_DDR 	= $(DDR3_VER_1G)
	DDR_SIZE	= $(DDR_SIZE_1G)
	DDR_NAME	= $(DDR_SIZE)
endif

ifeq ($(CONFIG_DDR3_MEM_2048M),y)
	VERSION_DDR 	= $(DDR3_VER_2G)
	DDR_SIZE	= $(DDR_SIZE_2G)
	DDR_NAME	= $(DDR_SIZE)
	ARM_DDR_SIZE	= $(DDR_SIZE_1G)
endif

ifeq ($(CONFIG_DDR3_MEM_4096M),y)
	VERSION_DDR 	= $(DDR3_VER_4G)
	DDR_SIZE	= $(DDR_SIZE_4G)
	DDR_NAME	= $(DDR_SIZE)
	ARM_DDR_SIZE	= $(DDR_SIZE_2G)
endif


## define amboot/kernel sub version

#MS_VER_FILE		= $(FIRMWARE_IMG_DIR)/hiboot_version.h
MS_VER_FILE		= $(FW_TOPDIR)/../include/hiboot_version.h
KER_VER_FILE	= $(FIRMWARE_IMG_DIR)/ms_kernel_version.h
IPC_VER_FILE	= $(SRC_DIR)/msgui/mssysconf.h
VERSION_UBT		= $(shell grep "\#define[ ]*VER_HIBOOT.*" $(MS_VER_FILE) | sed -e s/[^0-9,.]//g)
VERSION_KER		= $(shell grep "\#define[ ]*KERNEL_SUB_VERSION.*" $(KER_VER_FILE) | sed -e s/[^0-9,.]//g)
VERSION_JPG		= $(shell grep "\#define[ ]*VER_JPG\b.*" $(MS_VER_FILE) | sed -e s/[^0-9,.]//g)
#VERSION_IPC		= $(shell grep "\#define[ ]*MS_HI_IPC_VERSION.*" $(IPC_VER_FILE) | sed -e s/[^0-9,.]//g)
VERSION_IPC		= $(shell grep "\#define[ ]*MS_HI_NVR_VERSION.*" $(IPC_VER_FILE) | awk '{print $$3}')
VERSION_IPC_S	= $(shell grep "\#define[ ]*MS_HI_NVR_VERSION.*" $(IPC_VER_FILE) | sed -e s/[^0-9,.,-]//g | sed -e "s/[.,-]/ /g")
IPC_VER0			= $(word 1, $(VERSION_IPC_S))
IPC_VER1			= $(word 2, $(VERSION_IPC_S))
IPC_VER2			= $(word 3, $(VERSION_IPC_S))

HIBOOT_VERSION  = $(VERSION_CPU).$(VERSION_FLASH).$(VERSION_DDR).$(VERSION_UBT)
HIBOOT_VER_4G	= $(VERSION_CPU).$(VERSION_FLASH).$(DDR3_VER_4G).$(VERSION_UBT)
HIBOOT_VER_2G	= $(VERSION_CPU).$(VERSION_FLASH).$(DDR3_VER_2G).$(VERSION_UBT)
JPG_VER		= $(VERSION_CPU).$(VERSION_FLASH).0.$(VERSION_JPG)
KERNEL_VERSION	= $(VERSION_CPU).$(VERSION_FLASH).x.$(VERSION_KER)

UPFW_VERSION	:= $(VERSION_FS).$(VERSION_IPC)
FILESYS_VERSION := $(VERSION_FS).$(VERSION_IPC)

###############################################################
## end define firware version
###############################################################
include $(FW_TOPDIR)/*/*.mk

KER_IMAGE		:= $(UPFW_KER_IMG)
FS_IMAGE		:= $(UPFW_FS_IMG)
UBT_IMAGE		:= $(UPFW_BLD_IMG)
UBT2_IMAGE		:= $(UPFW_BLD2_IMG)
JPG_IMAGE		:= $(UPFW_JPG_IMG)
UPFW_IMAGE		:= $(UPFW_UPFW_IMG)

.PHONY: tools tools-clean  install uninstall

tools: tools-clean host_prepared host_upfw update fw_env

tools-clean: upfwclean fw_env-clean

install: uninstall tools updatefw
	$(MS_MAKEFILE_V)[ -z $(UBT_IMAGE) ] || cp -dpRf $(UBT_IMAGE) $(FIRMWARE_IMG_DIR)/UBOOT_$(HIBOOT_VER_2G).bin
	$(MS_MAKEFILE_V)[ -z $(UBT2_IMAGE) ] || cp -dpRf $(UBT2_IMAGE) $(FIRMWARE_IMG_DIR)/UBOOT_$(HIBOOT_VER_4G).bin
	$(MS_MAKEFILE_V)[ -z $(JPG_IMAGE) ] || cp -dpRf $(JPG_IMAGE) $(FIRMWARE_IMG_DIR)/JPG_$(JPG_VER).bin
	$(MS_MAKEFILE_V)[ -z $(KER_IMAGE) ] || cp -dpRf $(KER_IMAGE) $(FIRMWARE_IMG_DIR)/KImage_$(KERNEL_VERSION).bin
	$(MS_MAKEFILE_V)[ -z $(FS_IMAGE) ] || cp -dpRf $(FS_IMAGE) $(FIRMWARE_IMG_DIR)/Filesys_$(FILESYS_VERSION).bin
	$(MS_MAKEFILE_V)[ -z $(UPFW_IMAGE) ] || cp -dpRf $(UPFW_IMAGE) $(FIRMWARE_IMG_DIR)/MSFImage_$(UPFW_VERSION)

uninstall:
	-$(MS_MAKEFILE_V)rm -rf $(FIRMWARE_IMG_DIR)/Filesys_$(FILESYS_VERSION).bin 
	-$(MS_MAKEFILE_V)rm -rf $(FIRMWARE_IMG_DIR)/UBOOT_$(HIBOOT_VER_2G).bin
	-$(MS_MAKEFILE_V)rm -rf $(FIRMWARE_IMG_DIR)/UBOOT_$(HIBOOT_VER_4G).bin
	-$(MS_MAKEFILE_V)rm -rf $(FIRMWARE_IMG_DIR)/JPG_$(JPG_VER).bin
	-$(MS_MAKEFILE_V)rm -rf $(FIRMWARE_IMG_DIR)/KImage_$(KERNEL_VERSION).bin
	-$(MS_MAKEFILE_V)rm -rf $(FIRMWARE_IMG_DIR)/Filesys_$(FILESYS_VERSION).bin
	-$(MS_MAKEFILE_V)rm -rf $(FIRMWARE_IMG_DIR)/MSFImage_$(UPFW_VERSION)
	
