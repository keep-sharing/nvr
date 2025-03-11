################################################################################

PWD           = $(shell pwd)
HW_CROSS_COMPILE = aarch64-mix410-linux-
TOPDIR        =

REG_OFFSET        := 64
REG_RESERVED_SIZE := 0x3C00	# 15 KB

################################################################################
CC       := $(HW_CROSS_COMPILE)gcc
AR       := $(HW_CROSS_COMPILE)ar
LD       := $(HW_CROSS_COMPILE)ld
OBJCOPY  := $(HW_CROSS_COMPILE)objcopy
OBJDUMP  := $(HW_CROSS_COMPILE)objdump

ifeq ($(TOPDIR),)
TOPDIR   := $(realpath $(dir $(lastword $(MAKEFILE_LIST)))/../../../../../..)
endif
DDR_TOP_DIR := $(TOPDIR)/drivers/ddr/vendor

################################################################################
BOOT     := u-boot-$(SOC)
TEXTBASE := 0x48700000
CFLAGS   :=-Os -fno-builtin -ffreestanding \
	-D__KERNEL__ -DTEXT_BASE=$(TEXTBASE) \
	-DREG_OFFSET=$(REG_OFFSET) \
	-DREG_RESERVED_SIZE=$(REG_RESERVED_SIZE) \
	-I$(TOPDIR)/include \
	-I$(TOPDIR)/include/linux \
	-I$(TOPDIR)/drivers/ddr/vendor/boot_adaptation/uboot \
	-I$(TOPDIR)/drivers/ddr/vendor/include \
	-I$(TOPDIR)/arch/arm/include \
	-include $(TOPDIR)/include/linux/kconfig.h \
	-fno-pic  -mstrict-align  -ffunction-sections \
	-fdata-sections -fno-common -ffixed-r9    \
	-fno-common -ffixed-x18 -pipe -march=armv8-a \
	-Wall -Wstrict-prototypes -fno-stack-protector \
	-D__LINUX_ARM_ARCH__=8 -D__ARM__ \
	-DCONFIG_SS626V100 -DCONFIG_MMC \
	$(MKFLAGS) -fno-strict-aliasing

################################################################################

START := start.o
COBJS := lowlevel_init_v300.o \
	init_registers.o \
	sdhci_boot.o \
	uart.o \
	ddrtrn_api_pm.o \
	ddrtrn_api_training_item.o \
	ddrtrn_common_ctl.o \
	ddrtrn_boot.o \
	ddrtrn_exec_time.o \
	ddrtrn_training_custom.o \
	ddrtrn_hal_auto_pd.o \
	ddrtrn_hal_ddrc.o \
	ddrtrn_ddrc_s14.o \
	ddrtrn_hal_ddrt.o \
	ddrtrn_hal_ac.o \
	ddrtrn_hal_adjust.o \
	ddrtrn_hal_dataeye.o \
	ddrtrn_hal_dcc.o \
	ddrtrn_hal_gate.o \
	ddrtrn_hal_hw.o \
	ddrtrn_hal_low_freq.o \
	ddrtrn_hal_lpca.o \
	ddrtrn_hal_pcode.o \
	ddrtrn_hal_phy.o \
	ddrtrn_hal_phy_common.o \
	ddrtrn_hal_vref.o \
	ddrtrn_hal_wakeup.o \
	ddrtrn_hal_write_leveling.o \
	ddrtrn_phy_s14.o \
	ddrtrn_hal_sysctrl_common.o \
	ddrtrn_hal_uart.o \
	ddrtrn_hal_context.o \
	ddrtrn_ac.o \
	ddrtrn_adjust.o \
	ddrtrn_auto_pd.o \
	ddrtrn_console.o \
	ddrtrn_dataeye.o \
	ddrtrn_dcc.o \
	ddrtrn_ddrc.o \
	ddrtrn_ddrt.o \
	ddrtrn_gate.o \
	ddrtrn_hw.o \
	ddrtrn_low_freq.o \
	ddrtrn_lpca.o \
	ddrtrn_pcode.o \
	ddrtrn_phy.o \
	ddrtrn_retrain_enable.o \
	ddrtrn_vref.o \
	ddrtrn_wakeup.o \
	ddrtrn_write_leveling.o \
	startup.o \
	image_data.o \
	div0.o \
	reset.o

SSRC  := arch/arm/cpu/armv8/$(SOC)/start.S \
	arch/arm/cpu/armv8/$(SOC)/reset.S \
	arch/arm/cpu/armv8/$(SOC)/sdhci_boot.c \
	arch/arm/cpu/armv8/$(SOC)/uart.S \
	arch/arm/cpu/armv8/$(SOC)/init_registers.c \
	arch/arm/cpu/armv8/$(SOC)/lowlevel_init_v300.c \
	drivers/ddr/vendor/api/ddrtrn_api_pm.c \
	drivers/ddr/vendor/api/ddrtrn_api_training_item.c \
	drivers/ddr/vendor/boot_adaptation/common/ddrtrn_common_ctl.c \
	drivers/ddr/vendor/boot_adaptation/uboot/ddrtrn_boot.c \
	drivers/ddr/vendor/hal/chip/ss626v100/ddrtrn_exec_time.c \
	drivers/ddr/vendor/hal/chip/ss626v100/ddrtrn_training_custom.c \
	drivers/ddr/vendor/hal/ddrc/common/ddrtrn_hal_auto_pd.c \
	drivers/ddr/vendor/hal/ddrc/common/ddrtrn_hal_ddrc.c \
	drivers/ddr/vendor/hal/ddrc/v0/ddrtrn_ddrc_s14.c \
	drivers/ddr/vendor/hal/ddrt/common/ddrtrn_hal_ddrt.c \
	drivers/ddr/vendor/hal/phy/common/ddrtrn_hal_ac.c \
	drivers/ddr/vendor/hal/phy/common/ddrtrn_hal_adjust.c \
	drivers/ddr/vendor/hal/phy/common/ddrtrn_hal_dataeye.c \
	drivers/ddr/vendor/hal/phy/common/ddrtrn_hal_dcc.c \
	drivers/ddr/vendor/hal/phy/common/ddrtrn_hal_gate.c \
	drivers/ddr/vendor/hal/phy/common/ddrtrn_hal_hw.c \
	drivers/ddr/vendor/hal/phy/common/ddrtrn_hal_low_freq.c \
	drivers/ddr/vendor/hal/phy/common/ddrtrn_hal_lpca.c \
	drivers/ddr/vendor/hal/phy/common/ddrtrn_hal_pcode.c \
	drivers/ddr/vendor/hal/phy/common/ddrtrn_hal_phy.c \
	drivers/ddr/vendor/hal/phy/common/ddrtrn_hal_phy_common.c \
	drivers/ddr/vendor/hal/phy/common/ddrtrn_hal_vref.c \
	drivers/ddr/vendor/hal/phy/common/ddrtrn_hal_wakeup.c \
	drivers/ddr/vendor/hal/phy/common/ddrtrn_hal_write_leveling.c \
	drivers/ddr/vendor/hal/phy/v0/ddrtrn_phy_s14.c \
	drivers/ddr/vendor/hal/sysctrl/ddrtrn_hal_sysctrl_common.c \
	drivers/ddr/vendor/hal/uart/ddrtrn_hal_uart.c \
	drivers/ddr/vendor/hal/ddrtrn_hal_context.c \
	drivers/ddr/vendor/training/ddrtrn_ac.c \
	drivers/ddr/vendor/training/ddrtrn_adjust.c \
	drivers/ddr/vendor/training/ddrtrn_auto_pd.c \
	drivers/ddr/vendor/training/ddrtrn_console.c \
	drivers/ddr/vendor/training/ddrtrn_dataeye.c \
	drivers/ddr/vendor/training/ddrtrn_dcc.c \
	drivers/ddr/vendor/training/ddrtrn_ddrc.c \
	drivers/ddr/vendor/training/ddrtrn_ddrt.c \
	drivers/ddr/vendor/training/ddrtrn_gate.c \
	drivers/ddr/vendor/training/ddrtrn_hw.c \
	drivers/ddr/vendor/training/ddrtrn_low_freq.c \
	drivers/ddr/vendor/training/ddrtrn_lpca.c \
	drivers/ddr/vendor/training/ddrtrn_pcode.c \
	drivers/ddr/vendor/training/ddrtrn_phy.c \
	drivers/ddr/vendor/training/ddrtrn_retrain_enable.c \
	drivers/ddr/vendor/training/ddrtrn_vref.c \
	drivers/ddr/vendor/training/ddrtrn_wakeup.c \
	drivers/ddr/vendor/training/ddrtrn_write_leveling.c \
	arch/arm/lib/div0.c \
	lib/hw_dec/hw_decompress.c \
	lib/hw_dec/hw_decompress_$(SOC).c \
	lib/hw_dec/hw_decompress_v2.c \
	lib/hw_dec/hw_decompress_v2.h

# expressed in decimal
_REG_OFFSET        := $(shell printf '%d' $(REG_OFFSET))
_REG_RESERVED_SIZE := $(shell printf '%d' $(REG_RESERVED_SIZE))
_REG_RESERVED_END  := $(shell expr $(_REG_OFFSET) + $(_REG_RESERVED_SIZE))

REG := $(wildcard $(TOPDIR)/*.reg $(TOPDIR)/.reg)
SRC := $(notdir $(SSRC))

################################################################################
.PHONY: $(BOOT).bin
$(BOOT).bin: $(BOOT).tmp regfile
	@dd if=./$(BOOT).tmp of=./tmp1 bs=1 count=$(_REG_OFFSET) 2>/dev/null
	@dd if=$(REG) of=./tmp2 bs=$(_REG_RESERVED_SIZE) conv=sync 2>/dev/null
	@dd if=./$(BOOT).tmp of=./tmp3 bs=1 skip=$(_REG_RESERVED_END) 2>/dev/null
	@cat tmp1 tmp2 tmp3 > $(BOOT).bin
	@rm -f tmp1 tmp2 tmp3
	@chmod 754 $(BOOT).bin
	@cp -fv $@ $(TOPDIR)
	@echo $(BOOT).bin is Ready.

$(BOOT).tmp: $(BOOT).elf
	$(OBJCOPY) -O srec $< $(BOOT).srec
	$(OBJCOPY) -j .text -O binary $< $(BOOT).text
	$(OBJCOPY) --gap-fill=0xff -O binary $< $@

$(BOOT).elf: image_data.gzip $(SRC) $(START) $(COBJS)
	$(LD) -Bstatic -T u-boot.lds -Ttext $(TEXTBASE) $(START) \
		$(COBJS) -Map $(BOOT).map -o $@
	$(OBJDUMP) -d  $@ > $@.asm

.PHONY: regfile
regfile:
	@if [ "$(words $(REG))" = "0" ]; then ( \
		echo '***' Need '.reg' or '*.reg' file in directory $(TOPDIR); \
		exit 1; \
	) fi
	@if [ "$(words $(REG))" != "1" ]; then ( \
		echo '***' Found multi '.reg' or '*.reg' file in directory $(TOPDIR); \
		echo '***' Files: $(notdir $(REG)); \
		exit 1; \
	) fi

################################################################################
start.o: start.S
	$(CC) -D__ASSEMBLY__ $(CFLAGS) -o $@ $< -c

# -1 : --fast      -9 : --best
image_data.gzip: $(BINIMAGE)
	gzip -fNqc -7 $< > $@

%.o: %.c
	$(CC) $(CFLAGS) -Wall -Wstrict-prototypes \
		-fno-stack-protector -o $@ $< -c

%.o: %.S
	$(CC) -D__ASSEMBLY__ $(CFLAGS) -o $@ $< -c

image_data.o: image_data.S image_data.gzip
	$(CC) -D__ASSEMBLY__ $(CFLAGS) -o $@ $< -c

#############################################################################

$(SRC):
	ln -sf ../../../../../../$(filter %/$@,$(SSRC)) $@
################################################################################
TMPS := $(COBJS) start.o $(SRC) \
	$(BOOT).map $(BOOT).elf $(BOOT).srec $(BOOT).bin $(BOOT).text $(BOOT).tmp \
	image_data.gzip

distclean: clean

clean:
	-rm -f $(TMPS)

################################################################################
.PHONY: clean
################################################################################
