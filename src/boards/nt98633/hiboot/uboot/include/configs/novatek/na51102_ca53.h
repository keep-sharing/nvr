/* NVT board configuration */
#define CONFIG_MEM_SIZE				_BOARD_DRAM_SIZE_
/* kernel image and all-in-one image size */
#if (CONFIG_MEM_SIZE == (SZ_1M * 256))
#define CONFIG_NVT_UIMAGE_SIZE			(SZ_1M * 25)
#define CONFIG_NVT_ALL_IN_ONE_IMG_SIZE		(SZ_1M * 120)
#elif (CONFIG_MEM_SIZE == (SZ_1M * 128))
#define CONFIG_NVT_UIMAGE_SIZE			(SZ_1M * 8)
#define CONFIG_NVT_ALL_IN_ONE_IMG_SIZE		(SZ_1M * 32)
#elif (CONFIG_MEM_SIZE == (SZ_1M * 64))
#define CONFIG_NVT_UIMAGE_SIZE			(SZ_1M * 4)
#define CONFIG_NVT_ALL_IN_ONE_IMG_SIZE		(SZ_1M * 20)
#else
#define CONFIG_NVT_UIMAGE_SIZE			(SZ_1M * 25)
#define CONFIG_NVT_ALL_IN_ONE_IMG_SIZE		(SZ_1M * 330)
#endif

/* temp buffer for all-in-one T.bin boot requirement */
#define CONFIG_UBOOT_SDRAM_BASE			_BOARD_UBOOT_ADDR_
#define CONFIG_UBOOT_SDRAM_SIZE			_BOARD_UBOOT_SIZE_
#define CONFIG_LINUX_SDRAM_BASE			_BOARD_LINUXTMP_ADDR_
#define CONFIG_LINUX_SDRAM_SIZE			_BOARD_LINUXTMP_SIZE_
#define CONFIG_LINUX_SDRAM_START		(CONFIG_UBOOT_SDRAM_BASE - CONFIG_NVT_UIMAGE_SIZE)
#define CONFIG_NVT_RUNFW_SDRAM_BASE		(CONFIG_LINUX_SDRAM_START - CONFIG_NVT_ALL_IN_ONE_IMG_SIZE)
#define CONFIG_RAMDISK_SDRAM_SIZE		(SZ_1M * 30)		/* To define maximum ramdisk size */

#define CONFIG_SMEM_SDRAM_BASE			_BOARD_SHMEM_ADDR_
#define CONFIG_SMEM_SDRAM_SIZE			_BOARD_SHMEM_SIZE_
#define CONFIG_FDT_SDRAM_BASE			_BOARD_FDT_ADDR_
#define CONFIG_FDT_SDRAM_SIZE			_BOARD_FDT_SIZE_

#if defined(CONFIG_NVT_SPI_NAND)
#if (_EMBMEM_BLK_SIZE_ == 0x40000)
#define CONFIG_MODELEXT_FLASH_BASE		0x80000
#else
#define CONFIG_MODELEXT_FLASH_BASE		0x40000
#endif
#elif defined(CONFIG_NVT_IVOT_EMMC)
#define CONFIG_MODELEXT_FLASH_BASE		(0x40000/512)
#else
#define CONFIG_MODELEXT_FLASH_BASE		0x10000
#endif

/* FLASH        FileSystem */
#if defined(CONFIG_NVT_SPI_NAND) || defined(CONFIG_NVT_SPI_NONE)
#define CONFIG_SYS_MAX_NAND_DEVICE              1
#define CONFIG_SYS_NAND_MAX_CHIPS               1
#endif

#ifdef CONFIG_NVT_FPGA_EMULATION
#define CONFIG_SYS_HZ_CLOCK			24000000
#else /* !CONFIG_NVT_FPGA_EMULATION */
#define CONFIG_SYS_HZ_CLOCK			12000000
#endif /* CONFIG_NVT_FPGA_EMULATION */


/*
 * Serial driver init.
 *
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_MEM32
#define CONFIG_SYS_NS16550_REG_SIZE			-1
#define CONFIG_SYS_NS16550_COM1                         0xf0290000
#define CONFIG_SYS_NS16550_COM2                         0xf0300000
#define CONFIG_SYS_NS16550_COM3                         0xf0310000
#define CONFIG_SYS_NS16550_COM4                         0xf0380000
#define CONFIG_SYS_NS16550_COM5                         0xf03E0000
#ifdef CONFIG_NVT_FPGA_EMULATION
#define CONFIG_SYS_NS16550_CLK                          24000000        /* COM1: 24M, other COM port 48M */
#else
#define CONFIG_SYS_NS16550_CLK                          48000000        /* COM1: 24M, other COM port 48M */
#endif
#define CONFIG_SYS_NS16550_HSCLK                        48000000        /* COM1: 24M, other COM port 48M */
