#
# (C) Copyright 2017
# Novatek Microelectronic corp.
#
# SPDX-License-Identifier:	GPL-2.0+
#
ifdef CONFIG_TARGET_NA51055
obj-$(CONFIG_NVT_SPI_NOR) += spinor.o
obj-$(CONFIG_NVT_SPI_NAND) += spinand.o
obj-$(CONFIG_NVT_SPI_NONE) += spinand.o
else
obj-$(CONFIG_NVT_SPI_NOR) += spi020_nor.o
obj-$(CONFIG_NVT_SPI_NAND) += spi020_nand.o
obj-$(CONFIG_NVT_SPI_NONE) += spi020_nand.o
obj-$(CONFIG_NVT_SPI_NOR_NAND) += spi020_nand.o spi020_nor.o
endif
#spi020_nor.o
