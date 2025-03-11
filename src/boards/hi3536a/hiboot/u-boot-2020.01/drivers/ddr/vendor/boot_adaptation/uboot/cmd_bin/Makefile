##SPDX-License-Identifier: GPL-2.0
##Copyright (c) 2020 Shenshu Technologies CO., LIMITED.

CUR_PATH := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
################################################################################

ifeq ($(TOPDIR),)
TOPDIR   := $(realpath $(dir $(lastword $(MAKEFILE_LIST)))/../../../../../../)
endif
sinclude $(TOPDIR)/arch/$(ARCH)/config.mk       # include architecture dependend rules

PWD      := $(shell pwd)
OPPDIR   := $(subst $(TOPDIR),,$(PWD))
DDR_TOP_DIR = $(realpath $(CUR_PATH)/../../..)

CC       := $(CROSS_COMPILE)gcc
AR       := $(CROSS_COMPILE)ar
LD       := $(CROSS_COMPILE)ld
OBJCOPY  := $(CROSS_COMPILE)objcopy
OBJDUMP  := $(CROSS_COMPILE)objdump

################################################################################
DDR_CMD     := ddr_cmd
sinclude $(TOPDIR)/include/config/auto.conf
ifneq ("$(MAKECMDGOALS)","clean")
CMD_TEXT_BASE := $(shell grep '^\#define.*DDR_TRAINING_RUN_STACK' $(DDR_TOP_DIR)/include/hal/chip/$(SOC)/ddrtrn_training_custom.h|awk '{print $$3}')
endif
STACK_POINT   := $(CMD_TEXT_BASE)

INCLUDE_DIR := -I$(TOPDIR)/include \
	-I$(TOPDIR)/arch/$(ARCH)/include \
	-I$(DDR_TOP_DIR)/include \
	-I$(DDR_TOP_DIR)/boot_adaptation/uboot/cmd_bin/ \
	 -include $(TOPDIR)/include/linux/kconfig.h

SRCS := $(wildcard $(DDR_TOP_DIR)/boot_adaptation/uboot/cmd_bin/*.c)
SRCS += $(wildcard $(DDR_TOP_DIR)/boot_adaptation/common/*.c)
SRCS += $(wildcard $(DDR_TOP_DIR)/api/*.c)
SRCS += $(wildcard $(DDR_TOP_DIR)/training/*.c)
SRCS += $(wildcard $(DDR_TOP_DIR)/hal/*.c)
SRCS += $(wildcard $(DDR_TOP_DIR)/hal/phy/common/*.c)
SRCS += $(wildcard $(DDR_TOP_DIR)/hal/ddrc/common/*.c)
SRCS += $(wildcard $(DDR_TOP_DIR)/hal/ddrt/common/*.c)
SRCS += $(wildcard $(DDR_TOP_DIR)/hal/sysctrl/*.c)
SRCS += $(wildcard $(DDR_TOP_DIR)/hal/uart/*.c)

CFLAGS   := -Os -pipe  \
	-DCMD_TEXT_BASE=$(CMD_TEXT_BASE) \
	-DSTACK_POINT=$(STACK_POINT) \
	-DDDR_TRAINING_CMD \
	-D__KERNEL__ \
	-fno-builtin \
	-ffreestanding \
	-Wall -Werror \
	$(INCLUDE_DIR)

ifneq ("$(MAKECMDGOALS)","clean")
ifeq ($(SOC),ss626v100)
SRCS += $(wildcard $(DDR_TOP_DIR)/hal/phy/v0/*.c)
SRCS += $(wildcard $(DDR_TOP_DIR)/hal/chip/ss626v100/*.c)
SRCS += $(wildcard $(DDR_TOP_DIR)/hal/ddrc/v0/*.c)
CFLAGS += -DDDR_CHIP_SS626V100
else
$(info "CMD Not support soc!!!" $(SOC) "end")
endif
endif

COBJS := $(patsubst %.c,%.o,$(SRCS))
DEPS := $(patsubst %.o,%.d,$(COBJS)) $(patsubst %.o,%.d,$(START))

ifneq ($(OBJTREE),$(SRCTREE))
CFLAGS += -I$(OBJTREE)/include2 -I$(OBJTREE)/include
endif
CFLAGS += $(PLATFORM_RELFLAGS) $(PLATFORM_CPPFLAGS)

ifeq ("$(CONFIG_ARM64)","y")
START       := ddrtrn_cmdbin_entry_64.o
LDS_SCRIPT := ddrtrn_cmdbin_64.lds
CFLAGS += -DCONFIG_ARM64
else
START       := ddrtrn_cmdbin_entry_32.o
LDS_SCRIPT := ddrtrn_cmdbin_32.lds
endif

################################################################################

LINK_FILES = $(SSRC)
.PHONY: $(DDR_CMD).bin
all: $(DDR_CMD).bin
	#remove soft link files
	@rm -f $(LINK_FILES) *.o *.d *.elf *.map *.srec

$(DDR_CMD).bin: $(DDR_CMD).elf
	$(OBJCOPY) -O srec $(PWD)/$(DDR_CMD).elf $(DDR_CMD).srec
	$(OBJCOPY) --gap-fill=0xff -O binary $(PWD)/$(DDR_CMD).elf $@

$(DDR_CMD).elf: $(START) $(COBJS) $(LDS_SCRIPT)
	$(LD) -Bstatic -T $(LDS_SCRIPT) -Ttext $(CMD_TEXT_BASE) $(START) \
		$(COBJS) $(AOBJS) -Map $(DDR_CMD).map -o $@

.PHONY: clean
clean:
	@rm -vf *.o *.d *.elf *.map *.srec $(LINK_FILES) $(DDR_CMD).bin

%.o : %.S
	$(CC) -D__ASSEMBLY__ $(CFLAGS) -o $@ -c $*.S

%.o : %.c
	$(CC) $(CFLAGS) -Wall -Wstrict-prototypes -fno-stack-protector \
		-o $@ -c $*.c

ifneq ("$(MAKECMDGOALS)","clean")
sinclude $(DEPS)
endif

%.d : %.c
	set -e; $(CC) $(CFLAGS) -MM $< | sed 's,$*.o:,$*.o $*.d:,g' > $@

%.d : %.S
	set -e; $(CC) $(CFLAGS) -MM $< | sed 's,$*.o:,$*.o $*.d:,g' > $@

################################################################################
