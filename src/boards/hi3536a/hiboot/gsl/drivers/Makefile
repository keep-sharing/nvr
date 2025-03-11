CSRC  = $(wildcard *.c wildcard common/*.c wildcard otp/*.c wildcard ddr_init/*.c ddr_init/boot_adaptation/common/*.c ddr_init/boot_adaptation/hsl/*.c ddr_init/hal/*.c ddr_init/hal/chip/ss626v100/*.c ddr_init/hal/ddrc/common/*.c ddr_init/hal/ddrc/v0/*.c ddr_init/hal/ddrt/common/*.c ddr_init/hal/phy/common/*.c ddr_init/hal/phy/v0/*.c ddr_init/hal/sysctrl/*.c ddr_init/hal/uart/*.c ddr_init/training/*.c ddr_init/api/*.c wildcard key_slot/*.c wildcard klad/*.c wildcard pke/*.c wildcard cipher/*.c wildcard securelib/*.c wildcard uart/*.c wildcard emmc/*.c wildcard sdio/*.c wildcard wildcard tzasc/*.c)
OBJS := $(patsubst %.c,%.o,$(CSRC) )

#.SILENT:

CFLAGS += -I./
CFLAGS += -I./common/
CFLAGS += -I./otp/
CFLAGS += -I./ddr_init/
CFLAGS += -I./ddr_init/boot_adaptation/common/
CFLAGS += -I./ddr_init/boot_adaptation/hsl/
CFLAGS += -I./ddr_init/hal/
CFLAGS += -I./ddr_init/hal/chip/ss626v100/
CFLAGS += -I./ddr_init/hal/ddrc/common/
CFLAGS += -I./ddr_init/hal/ddrc/v0/
CFLAGS += -I./ddr_init/hal/ddrt/common/
CFLAGS += -I./ddr_init/hal/phy/common/
CFLAGS += -I./ddr_init/hal/phy/v0/
CFLAGS += -I./ddr_init/hal/sysctrl/
CFLAGS += -I./ddr_init/hal/uart/
CFLAGS += -I./ddr_init/include/
CFLAGS += -I./ddr_init/include/hal/
CFLAGS += -I./ddr_init/include/hal/ss626v100/
CFLAGS += -I./ddr_init/include/hal/ddrc/v0/
CFLAGS += -I./ddr_init/include/hal/ddrt/v0/
CFLAGS += -I./ddr_init/include/hal/phy/v0/
CFLAGS += -I./ddr_init/include/hal/uart/
CFLAGS += -I./ddr_init/include/training/
CFLAGS += -I./ddr_init/training/
CFLAGS += -I./ddr_init/api/
CFLAGS += -I./key_slot/
CFLAGS += -I./klad/
CFLAGS += -I./klad/include/
CFLAGS += -I./pke/
CFLAGS += -I./cipher/
CFLAGS += -I./securelib/
CFLAGS += -I./uart/
CFLAGS += -I./emmc/
CFLAGS += -I./sdio/
CFLAGS += -I./tzasc/

default: libdrv.a

libdrv.a: $(OBJS)
	echo "  AR    libdrv.a"
	$(AR) r libdrv.a $(OBJS)
	echo "  RL    $@"
	$(RANLIB) $@

%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@
#.c.o:
#	echo "  CC    $<"
#	$(CC) $(CFLAGS) $<

clean:
	rm *.o *.a -f
	rm common/*.o    common/*.a -f
	rm otp/*.o       otp/*.a -f
	rm ddr_init/*.o  ddr_init/*.a -f
	rm ddr_init/boot_adaptation/common/*.o  ddr_init/boot_adaptation/common/*.a -f
	rm ddr_init/boot_adaptation/hsl/*.o  ddr_init/boot_adaptation/hsl/*.a -f
	rm ddr_init/hal/*.o  ddr_init/hal/*.a -f
	rm ddr_init/hal/chip/ss626v100/*.o  ddr_init/hal/chip/ss626v100/*.a -f
	rm ddr_init/hal/ddrc/common/*.o  ddr_init/hal/ddrc/common/*.a -f
	rm ddr_init/hal/ddrc/v0/*.o  ddr_init/hal/ddrc/v0/*.a -f
	rm ddr_init/hal/ddrt/common/*.o  ddr_init/hal/ddrt/common/*.a -f
	rm ddr_init/hal/phy/common/*.o  ddr_init/hal/phy/common/*.a -f
	rm ddr_init/hal/phy/v0/*.o  ddr_init/hal/phy/v0/*.a -f
	rm ddr_init/hal/sysctrl/*.o  ddr_init/hal/sysctrl/*.a -f
	rm ddr_init/hal/uart/*.o  ddr_init/hal/uart/*.a -f
	rm ddr_init/training/*.o  ddr_init/training/*.a -f
	rm ddr_init/api/*.o  ddr_init/api/*.a -f
	rm key_slot/*.o  key_slot/*.a -f
	rm klad/*.o      klad/*.a -f
	rm pke/*.o       pke/*.a -f
	rm cipher/*.o    cipher/*.a -f
	rm securelib/*.o securelib/*.a -f
	rm uart/*.o      uart/*.a -f
	rm emmc/*.o      emmc/*.a -f
	rm sdio/*.o      sdio/*.a -f
	rm tzasc/*.o     tzasc/*.a -f
