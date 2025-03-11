#
# Copyright (c) 2016 HiSilicon Technologies Co., Ltd.
#
# This program is free software; you can redistribute  it and/or modify it
# under  the terms of  the GNU General Public License as published by the
# Free Software Foundation;  either version 2 of the  License, or (at your
# option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
################################################################################

TOPDIR   :=
PWD      := $(shell pwd)
OPPDIR   := $(subst $(TOPDIR),,$(PWD))

CC       := $(CROSS_COMPILE)gcc
AR       := $(CROSS_COMPILE)ar
LD       := $(CROSS_COMPILE)ld
OBJCOPY  := $(CROSS_COMPILE)objcopy
OBJDUMP  := $(CROSS_COMPILE)objdump

################################################################################
BOOT        := ddr_training
TEXTBASE    := 0x04010C00
STACK_POINT := 0x04010C00
START       := start.o
COBJS       += uart.o libc.o
DEPS        := $(COBJS:.o=.d) $(START:.o=.d)
AOBJS       :=
SSRC        := uart.S start.S libc.c

CFLAGS   := -Os -fno-strict-aliasing -fno-common -ffixed-r8 \
	-DTEXT_BASE=$(TEXTBASE) -DSTACK_POINT=$(STACK_POINT) \
	-fno-builtin -ffreestanding -I./ -I$(TOPDIR)/include -I../ \
	-pipe -D__ARM__ -marm  -mabi=aapcs-linux  \
	-mno-thumb-interwork -march=armv7-a

################################################################################

TMPS += $(TOPDIR)/$(BOOT).bin \
	$(TOPDIR)/common/$(CPU)_ddr_training_printf.c \
	$(TOPDIR)/common/$(CPU)_ddr_training.h \
	$(SSRC)

.PHONY: $(BOOT).bin
all: $(BOOT).bin
	cd $(TOPDIR)/common && \
		ln -sf ../$(OPPDIR)/ddr_training_printf.c $(CPU)_ddr_training_printf.c && \
		ln -sf ../$(OPPDIR)/ddr_training.h $(CPU)_ddr_training.h && \
	cd -

TMPS += $(BOOT).bin $(BOOT).srec
$(BOOT).bin: $(BOOT).elf
	$(OBJCOPY) -O srec $(PWD)/$(BOOT).elf $(BOOT).srec
	$(OBJCOPY) --gap-fill=0xff -O binary $(PWD)/$(BOOT).elf $@

TMPS += $(BOOT).elf $(COBJS) $(START) $(BOOT).map $(DEPS)
$(BOOT).elf: $(START) $(COBJS) ../boot.lds
	$(LD) -Bstatic -T ../boot.lds -Ttext $(TEXTBASE) $(START) \
		$(COBJS) $(AOBJS) -Map $(BOOT).map -o $@

$(SSRC):
	ln -sf ../$@ $@

.PHONY: clean
clean:
	@rm -vf $(TMPS)

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
