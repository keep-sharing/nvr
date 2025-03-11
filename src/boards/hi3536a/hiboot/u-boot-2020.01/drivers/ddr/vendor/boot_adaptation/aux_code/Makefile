##SPDX-License-Identifier: GPL-2.0
##Copyright (c) 2020 Shenshu Technologies CO., LIMITED.

sinclude $(TOPDIR)/arch/$(ARCH)/config.mk       # include architecture dependend rules
################################################################################

ifeq ($(TOPDIR),)
TOPDIR   :=
endif
PWD      := $(shell pwd)
OPPDIR   := $(subst $(TOPDIR),,$(PWD))
DDR_TOP_DIR := $(TOPDIR)/drivers/ddr/vendor

CC       := $(CROSS_COMPILE)gcc
AR       := $(CROSS_COMPILE)ar
LD       := $(CROSS_COMPILE)ld
OBJCOPY  := $(CROSS_COMPILE)objcopy
OBJDUMP  := $(CROSS_COMPILE)objdump

################################################################################
DDR_CMD     := ddrtrn_cmd
#sinclude $(TOPDIR)/include/config/auto.conf
CMD_TEXT_BASE := $(shell grep '^\#define.*DDR_TRAINING_RUN_STACK' $(TOPDIR)/drivers/ddr/vendor/include/hal/chip/$(SOC)/ddrtrn_training_custom.h|awk '{print $$3}')
STACK_POINT   := $(CMD_TEXT_BASE)
SRCS :=    $(DDR_TOP_DIR)/hal/ddrc/v0/ddrtrn_ddrc_s14.c \
    $(DDR_TOP_DIR)/training/ddrtrn_ac.c \
    $(DDR_TOP_DIR)/training/ddrtrn_pcode.c \
    $(DDR_TOP_DIR)/hal/phy/common/ddrtrn_hal_adjust.c \
    $(DDR_TOP_DIR)/training/ddrtrn_adjust.c \
    $(DDR_TOP_DIR)/training/ddrtrn_gate.c \
    $(DDR_TOP_DIR)/training/ddrtrn_dataeye.c \
    $(DDR_TOP_DIR)/training/ddrtrn_write_leveling.c \
    $(DDR_TOP_DIR)/training/ddrtrn_vref.c \
    $(DDR_TOP_DIR)/training/ddrtrn_ddrc.c \
    $(DDR_TOP_DIR)/training/ddrtrn_dcc.c \
    $(DDR_TOP_DIR)/training/ddrtrn_hw.c \
    $(DDR_TOP_DIR)/training/ddrtrn_phy.c \
    $(DDR_TOP_DIR)/training/ddrtrn_ddrt.c \
    $(DDR_TOP_DIR)/hal/ddrtrn_hal_context.c \
    $(DDR_TOP_DIR)/hal/phy/common/ddrtrn_hal_dcc.c \
    $(DDR_TOP_DIR)/hal/phy/common/ddrtrn_hal_write_leveling.c \
    $(DDR_TOP_DIR)/hal/phy/common/ddrtrn_hal_ac.c \
    $(DDR_TOP_DIR)/hal/phy/common/ddrtrn_hal_gate.c \
    $(DDR_TOP_DIR)/hal/phy/common/ddrtrn_hal_pcode.c \
    $(DDR_TOP_DIR)/hal/phy/common/ddrtrn_hal_phy.c \
    $(DDR_TOP_DIR)/hal/phy/common/ddrtrn_hal_dataeye.c \
    $(DDR_TOP_DIR)/hal/phy/common/ddrtrn_hal_vref.c \
    $(DDR_TOP_DIR)/hal/phy/common/ddrtrn_hal_hw.c \
    $(DDR_TOP_DIR)/hal/ddrt/common/ddrtrn_hal_ddrt.c \
    $(DDR_TOP_DIR)/hal/ddrc/common/ddrtrn_hal_ddrc.c \
    $(DDR_TOP_DIR)/hal/phy/v0/ddrtrn_phy_s14.c \
    $(DDR_TOP_DIR)/boot_adaptation/hsl/ddrtrn_boot.c \
    $(DDR_TOP_DIR)/hal/chip/ss626v100/ddrtrn_training_custom.c

COBJS := $(patsubst %.c,%.o,$(SRCS))
DEPS := $(patsubst %.o,%.d,$(OBJS))

INCLUDE_DIR := -I$(TOPDIR)/arch/$(ARCH)/include \
    -I$(TOPDIR)/include \
    -I$(DDR_TOP_DIR)/ \
    -I$(DDR_TOP_DIR)/boot_adaptation/uboot/ \
    -I$(DDR_TOP_DIR)/boot_adaptation/uboot/cmd_bin/ \
    -I$(DDR_TOP_DIR)/hal/chip/$(SOC)/ \
    -I$(DDR_TOP_DIR)/include \
    -I$(DDR_TOP_DIR)/training/ \
    -I$(DDR_TOP_DIR)/config/ \
    -I$(DDR_TOP_DIR)/include/hal/ \
    -I$(DDR_TOP_DIR)/include/hal/phy/v0 \
    -I$(DDR_TOP_DIR)/include/hal/ddrc/v0 \
    -I$(DDR_TOP_DIR)/include/hal/ddrt/v0 \
    -I$(DDR_TOP_DIR)/include/hal/ddrt/v1 \
    -I$(DDR_TOP_DIR)/include/hal/ddrt/v2

CFLAGS   := -Os -pipe  \
    -DCMD_TEXT_BASE=$(CMD_TEXT_BASE) \
    -DSTACK_POINT=$(STACK_POINT) \
    -DDDR_TRAINING_CMD \
    -D__KERNEL__ \
    -fno-builtin \
    -ffreestanding \
    -Wall -Werror \
    $(INCLUDE_DIR)

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
    @rm -f *.o *.d *.elf *.map *.srec

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
