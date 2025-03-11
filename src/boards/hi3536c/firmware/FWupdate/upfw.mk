##
## amboot/config/Makefile.upfw
## Author:        zbing@milesight.cn
## Created at:    2015-05-20
##                
## Description:   ±‡“Î firmware Makefileµƒ»´æ÷≈‰÷√
##                1.image package
##                2.image update
##                3.update tools
## Copyright (C)  milesight
##

ifndef DATE
DATE 			:= $(shell date +%Y%m%d)
endif

YEAR 			:= $(shell date +%Y | sed s/^0//)
MONTH 			:= $(shell date +%m | sed s/^0//)
DAY 			:= $(shell date +%d | sed s/^0//)

## Define 
UPFW_SRC_DIR 		:= $(FW_TOPDIR)/FWupdate
UBOOT_DIR 			= $(FW_TOPDIR)/../hiboot/u-boot-2010.06
UPFW_INPUT_FILE		= $(UPFW_SRC_DIR)/baseinfo.h
UPFW_VERSION_FILE	= $(FW_TOPDIR)/../include/hiboot_version.h
UPFW_UPFW_IMG		= $(FW_TOPDIR)/update_IMG
UPFW_BUILD_DIR		= $(FW_TOPDIR)/build

TARGET_CC 	  	= $(CROSS)gcc
TARGET_STRIP  		= $(CROSS)strip
HOSTCC 			= gcc
CFLGS_VER		= -DVER_CPU=$(VERSION_CPU) -DVER_FILESYS=$(VERSION_FS) \
				-DVER_FLASH=$(VERSION_FLASH) -DVER_DDR3_512M=$(DDR3_VER_512M) \
				-DVER_DDR3_1G=$(DDR3_VER_1G) -DVER_LINUX=$(VERSION_KER) \
				-DVER_FS_MAIN=$(IPC_VER0) -DVER_FS_SUB=$(IPC_VER1) \
				-DVER_FS_THIRD=$(IPC_VER2)

# delete -DINPUT_BLD2_IMG=$(UPFW_BLD2_IMG) zbing 20160516
CFLGS_FILE		= -DOUTPUT_UPFW_IMG=$(UPFW_UPFW_IMG) \
				-DINPUT_BLD_IMG=$(UPFW_BLD_IMG) \
				-DINPUT_BLD2_IMG=$(UPFW_BLD2_IMG)\
				-DINPUT_JPG_IMG=$(UPFW_JPG_IMG) \
				-DINPUT_KER_IMG=$(UPFW_KER_IMG) \
				-DINPUT_FS_IMG=$(UPFW_FS_IMG) \
				-DINPUT_CMD_IMG=$(UPFW_CMD_IMG) \
				-DINPUT_TOOL_IMG=$(UPFW_TOOL_IMG)
				
HOST_CFLAGS   	= -Wall -DUSE_HOSTCC \
				-I$(UPFW_SRC_DIR) -I$(FW_TOPDIR)/../include \
				-I. -I$(PUBLIC_INC_DIR) \
				-I$(ENV_SRC_DIR) \
				-I$(UBOOT_DIR)/include \
				$(CFLGS_VER) $(CFLGS_FILE)
				
UPFW_CFLAGS		= -Wall $(PLATFORM_FLAGS) -D_XOPEN_SOURCE=500 -D__YEAR__=$(YEAR) \
				-D__MONTH__=$(MONTH) -D__DAY__=$(DAY) $(CFLGS_VER) $(CFLGS_FILE) \
				-Wall -DBUILD_NANDWRITE \
				-I$(UPFW_SRC_DIR) -I$(FW_TOPDIR)/../include \
				-I$(PUBLIC_INC_DIR)/vapi/$(PLATFORM_TYPE) \
				-I. -I$(PUBLIC_INC_DIR) \
				-I$(ENV_SRC_DIR)/

__MAKE_UPFW_OBJ = \
		$(TARGET_CC) $(UPFW_CFLAGS)  -c $< -o $@
		
__MAKE_UPFW_EXE = \
		$(TARGET_CC) $(UPFW_CFLAGS) -o $@

__MAKE_HOST_OBJ = \
		$(HOSTCC) $(HOST_CFLAGS)  -c $< -o $@
		
__MAKE_HOST_EXE = \
		$(HOSTCC) $(HOST_CFLAGS) -o $@

UPFW_CMD_EXE  := $(FW_TOPDIR)/update
		
ifeq ($(MAKE_VERBOSE),no)
	MAKE_UPFW_OBJ =	$(__MAKE_UPFW_OBJ)
	MAKE_UPFW_EXE =	$(__MAKE_UPFW_EXE)
	MAKE_HOST_OBJ =	$(__MAKE_HOST_OBJ)
	MAKE_HOST_EXE =	$(__MAKE_HOST_EXE)
else
	MAKE_UPFW_OBJ =	@echo "  UPFWCC    $@" ; $(__MAKE_UPFW_OBJ)
	MAKE_UPFW_EXE =	@echo "  UPFWLD    $@" ; $(__MAKE_UPFW_EXE)
	MAKE_HOST_OBJ =	@echo "  HOSTCC    $@" ; $(__MAKE_HOST_OBJ)
	MAKE_HOST_EXE =	@echo "  HOSTLD    $@" ; $(__MAKE_HOST_EXE)
endif

ifeq ($(CONFIG_UPFW_HIBOOT),y)
UPFW_BLD_IMG	:= $(FIRMWARE_IMG_DIR)/uboot_$(DDR_SIZE_512M)
UPFW_BLD2_IMG	:= $(FIRMWARE_IMG_DIR)/uboot_$(DDR_SIZE_1G)
else
UPFW_BLD_IMG	:= 
endif

ifeq ($(CONFIG_UPFW_KERNEL),y)
UPFW_KER_IMG	:= $(FIRMWARE_IMG_DIR)/uImage
else
UPFW_KER_IMG	:= 
endif

ifeq ($(CONFIG_UPFW_FS),y)
UPFW_FS_IMG		:= $(FIRMWARE_IMG_DIR)/ubifs
else
UPFW_FS_IMG		:= 
endif

ifeq ($(CONFIG_UPFW_CMD),y)
UPFW_CMD_IMG	:= $(UPFW_CMD_EXE)
else
UPFW_CMD_IMG	:= 
endif

ifeq ($(CONFIG_UPFW_TOOL),y)
UPFW_TOOL_IMG	:= $(FIRMWARE_IMG_DIR)/tool
else
UPFW_TOOL_IMG	:= 
endif

UPFW_JPG_IMG	:= $(FIRMWARE_IMG_DIR)/boot.jpg 


# we use -DVER_CPU=$(VERSION_CPU) to avoid modify file.
upfw_check_version: 
	#MAKE_VERBOSEsed -i -e "s,\#define VER_CPU.*,\#define VER_CPU\t$(VERSION_CPU)," $(UPFW_VERSION_FILE)
	#$(MS_MAKEFILE_V)sed -i -e "s,\#define VER_FILESYS.*,\#define VER_FILESYS\t$(VERSION_FS)," $(UPFW_VERSION_FILE)
	#$(MS_MAKEFILE_V)sed -i -e "s,\#define VER_FLASH.*,\#define VER_FLASH\t$(VERSION_FLASH)," $(UPFW_VERSION_FILE)
	#$(MS_MAKEFILE_V)sed -i -e "s,\#define VER_DDR.*,\#define VER_DDR\t$(VERSION_DDR)," $(UPFW_VERSION_FILE)
	#$(MS_MAKEFILE_V)sed -i -e "s,\#define VER_HIBOOT.*,\#define VER_HIBOOT\t$(VERSION_UBT)," $(UPFW_VERSION_FILE)
	#$(MS_MAKEFILE_V)sed -i -e "s,\#define VER_LINUX.*,\#define VER_LINUX\t$(VERSION_KER)," $(UPFW_VERSION_FILE)
	#$(MS_MAKEFILE_V)sed -i -e "s,\#define VER_FS_MAIN.*,\#define VER_FS_MAIN\t$(IPC_VER0)," $(UPFW_VERSION_FILE)
	#$(MS_MAKEFILE_V)sed -i -e "s,\#define VER_FS_SUB.*,\#define VER_FS_SUB\t$(IPC_VER1)," $(UPFW_VERSION_FILE)
	#$(MS_MAKEFILE_V)sed -i -e "s,\#define VER_FS_THIRD.*,\#define VER_FS_THIRD\t$(IPC_VER2)," $(UPFW_VERSION_FILE)

# we use -DOUTPUT_UPFW_IMG=$(UPFW_UPFW_IMG) to avoid modify file.	
upfw_check_file:
	#$(MS_MAKEFILE_V)sed -i -e "s,\#define OUTPUT_UPFW_IMG.*,\#define OUTPUT_UPFW_IMG\t\t$(UPFW_UPFW_IMG)," $(UPFW_INPUT_FILE)
	#$(MS_MAKEFILE_V)sed -i -e "s,\#define INPUT_BLD_IMG.*,\#define INPUT_BLD_IMG\t$(UPFW_BLD_IMG)," $(UPFW_INPUT_FILE)
	#$(MS_MAKEFILE_V)sed -i -e "s,\#define INPUT_BLD2_IMG.*,\#define INPUT_BLD2_IMG\t$(UPFW_BLD2_IMG)," $(UPFW_INPUT_FILE)
	#$(MS_MAKEFILE_V)sed -i -e "s,\#define INPUT_KER_IMG.*,\#define INPUT_KER_IMG\t$(UPFW_KER_IMG)," $(UPFW_INPUT_FILE)
	#$(MS_MAKEFILE_V)sed -i -e "s,\#define INPUT_FS_IMG.*,\#define INPUT_FS_IMG\t$(UPFW_FS_IMG)," $(UPFW_INPUT_FILE)		
	#$(MS_MAKEFILE_V)sed -i -e "s,\#define INPUT_CMD_IMG.*,\#define INPUT_CMD_IMG\t$(UPFW_CMD_IMG)," $(UPFW_INPUT_FILE)
	#$(MS_MAKEFILE_V)sed -i -e "s,\#define INPUT_TOOL_IMG.*,\#define INPUT_TOOL_IMG\t$(UPFW_TOOL_IMG)," $(UPFW_INPUT_FILE)
	#$(MS_MAKEFILE_V)sed -i -e "s,\#define INPUT_JPG_IMG.*,\#define INPUT_JPG_IMG\t$(UPFW_JPG_IMG)," $(UPFW_INPUT_FILE)

$(FW_TOPDIR)/host_upfw.o: $(UPFW_SRC_DIR)/host_upfw.c $(UPFW_SRC_DIR)/updateFW.h	
	$(MAKE_HOST_OBJ)

	
$(FW_TOPDIR)/host_md5.o: $(UPFW_SRC_DIR)/md5.c $(UPFW_SRC_DIR)/md5.h
	$(MAKE_HOST_OBJ)
	
$(FW_TOPDIR)/host_crc32.o: $(ENV_SRC_DIR)/crc32.c
	$(MAKE_HOST_OBJ)

$(FW_TOPDIR)/host_updateFW.o: $(UPFW_SRC_DIR)/updateFW.c \
		$(PUBLIC_INC_DIR)/update_cmd.h \
		$(UBT_VER_FILE) \
		$(UPFW_SRC_DIR)/updateFW.h \
		$(UPFW_SRC_DIR)/baseinfo.h \
		$(UPFW_SRC_DIR)/md5.h \
		$(UPFW_SRC_DIR)/nandwrite.h
	$(MAKE_HOST_OBJ) \
		-D__YEAR__=$(YEAR) \
		-D__MONTH__=$(MONTH) \
		-D__DAY__=$(DAY)

host_prepared: upfw_check_version upfw_check_file $(ENV_SRC_DIR)/crc32.c
	
host_upfw: $(FW_TOPDIR)/host_crc32.o \
		$(FW_TOPDIR)/host_md5.o \
		$(FW_TOPDIR)/host_updateFW.o \
		$(FW_TOPDIR)/host_upfw.o
	$(MAKE_HOST_EXE) $^

$(FW_TOPDIR)/nandwrite.o: $(UPFW_SRC_DIR)/nandwrite.c \
	$(UPFW_SRC_DIR)/nandwrite.h
	$(MAKE_UPFW_OBJ)
	
$(FW_TOPDIR)/md5.o: $(UPFW_SRC_DIR)/md5.c \
		$(UPFW_SRC_DIR)/md5.h	
	$(MAKE_UPFW_OBJ)
	
$(FW_TOPDIR)/updateFW.o: $(UPFW_SRC_DIR)/updateFW.c \
		$(PUBLIC_INC_DIR)/update_cmd.h \
		$(UBT_VER_FILE) \
		$(UPFW_SRC_DIR)/updateFW.h \
		$(UPFW_SRC_DIR)/baseinfo.h \
		$(UPFW_SRC_DIR)/md5.h \
		$(UPFW_SRC_DIR)/nandwrite.h
	$(MAKE_UPFW_OBJ)
	
$(FW_TOPDIR)/update.o: $(UPFW_SRC_DIR)/update.c \
		$(UPFW_SRC_DIR)/updateFW.h
	$(MAKE_UPFW_OBJ)
	
$(FW_TOPDIR)/chipinfo.o: $(SRC_DIR)/libs/chipinfo/$(PLATFORM_TYPE)/chipinfo.c 
	$(MAKE_UPFW_OBJ)
	
update: $(FW_TOPDIR)/md5.o \
		$(FW_TOPDIR)/updateFW.o \
		$(FW_TOPDIR)/nandwrite.o \
		$(FW_TOPDIR)/chipinfo.o \
		$(FW_TOPDIR)/update.o $(ENV_OBJS)
	$(MAKE_UPFW_EXE) $^ -o $(UPFW_CMD_EXE)
	$(TARGET_STRIP) $(UPFW_CMD_EXE)
	$(MS_MAKEFILE_V)cp -dpRf $(UPFW_CMD_EXE) $(PUBLIC_APP_DIR)/
#	-$(MS_MAKEFILE_V)cp  update /tftpboot/
	
updatefw: host_upfw fw_env update
	./host_upfw -c
	./host_upfw -v
#	-$(MS_MAKEFILE_V)cp  updatefw/* /tftpboot/

upfwclean:
	-$(MS_MAKEFILE_V)rm -rf $(FW_TOPDIR)/*.o $(FW_TOPDIR)/.update $(FW_TOPDIR)/host_upfw  
	-$(MS_MAKEFILE_V)rm -rf $(PUBLIC_APP_DIR)/update $(UPFW_UPFW_IMG) $(UPFW_CMD_EXE)

