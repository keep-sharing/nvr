#
#
#
#
#
PLATFORM_FLAGS		:= -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4 -mno-unaligned-access -fno-aggressive-loop-optimizations -ffunction-sections -Wl,-gc-sections -g -fstack-protector -fstack-protector-all -rdynamic -funwind-tables
STD_LIB_FLAGS		:= -D_UCLIBC_ -D_HI3536C_ -g
export PLATFORM_FLAGS
export STD_LIB_FLAGS

QT_PATH 		:= $(TOOLS_DIR)/hi3536c/QTLibs/
CHIP_MODEL 	= hi3536c
CROSS_TYPE 	= arm-hisiv500-linux
export CHIP_MODEL
export CROSS_TYPE

LINUX_TOOLCHAIN_PREFIX		=/opt/hisi-linux/x86-arm

ARM_LINUX_TOOCHAIN_DIR		=$(LINUX_TOOLCHAIN_PREFIX)/$(CROSS_TYPE)
LINUX_TOOLCHAIN_NAME		=$(CROSS_TYPE)
LINUX_TOOLCHAIN_VERSION		=4.9.4

# define common path of toolchain 
TOOLCHAIN_PATH			=$(ARM_LINUX_TOOCHAIN_DIR)/target/bin

# define cross compile
CROSS_COMPILE		:= $(TOOLCHAIN_PATH)/$(LINUX_TOOLCHAIN_NAME)-
TARGET_HOST   		:= $(TOOLCHAIN_PATH)/$(LINUX_TOOLCHAIN_NAME)
TARGET_CROSS  		:= $(TOOLCHAIN_PATH)/$(LINUX_TOOLCHAIN_NAME)-
TARGET_CC     		:= $(TARGET_CROSS)gcc
TARGET_CXX    		:= $(TARGET_CROSS)g++
TARGET_AR     		:= $(TARGET_CROSS)ar
TARGET_LD     		:= $(TARGET_CROSS)ld
TARGET_STRIP  		:= $(TARGET_CROSS)strip


# define platform name and platform base information
PLATFORM_NAME  		:= hi3536c

PLATFORM_DIR   		:= $(TOP_DIR)/platform/$(PLATFORM_NAME)
ROOT_DIR       		:= $(PLATFORM_DIR)/rootfs
INFO_DIR       		:= $(PLATFORM_DIR)/info
PERSISTENT_DIR 		:= $(PLATFORM_DIR)/persistent
CONFIG_DIR		:= $(PLATFORM_DIR)/HisiConfig

# define target directory
TARGET_DIR		:= $(TOP_DIR)/targets/$(PLATFORM_NAME)
TARGET_SYS_DIR		:= $(TARGET_DIR)/sys
FIRMWARE_IMG_DIR	:= $(TARGET_DIR)/firmware
NSFROOT_DIR		:= $(TARGET_DIR)/nfsroot
MODULE_INSTALL_DIR	:= $(TARGET_SYS_DIR)/modules
KERNEL_IMG_DIR 		:= $(FIRMWARE_IMG_DIR)
ROOTFS_IMG_DIR		:= $(FIRMWARE_IMG_DIR)


# define std libs
STD_LIB_DIR    		:= $(ROOT_DIR)/lib
STD_USRLIB_DIR 		:= $(ROOT_DIR)/usr/lib
STD_LOCAL_BIN_DIR	:= $(ROOT_DIR)/usr/local/bin

UPFW_CONFIG_PATH    := 

export FIRMWARE_IMG_DIR
export STD_LIB_DIR
export STD_USRLIB_DIR
export CONFIG_DIR


APP_DIR        		:= $(TARGETS_DIR)/$(PLATFORM_NAME)/app
SHLIB_DIR      		:= $(TARGETS_DIR)/$(PLATFORM_NAME)/shlib
IMG_DIR        		:= $(TARGETS_DIR)/$(PLATFORM_NAME)/image

TFTP_HOME      		:= /tftpboot

image:
	@echo "make image"
	@echo "############################################################"
	@echo "生成 hisi 鏡像成功"
	@echo "############################################################"
	

