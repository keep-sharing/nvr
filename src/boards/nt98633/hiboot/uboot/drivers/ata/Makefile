# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-$(CONFIG_DWC_AHCI) += dwc_ahci.o
obj-$(CONFIG_AHCI) += ahci-uclass.o
obj-$(CONFIG_AHCI_PCI) += ahci-pci.o
obj-$(CONFIG_SCSI_AHCI) += ahci.o
obj-$(CONFIG_DWC_AHSATA) += dwc_ahsata.o
obj-$(CONFIG_FSL_SATA) += fsl_sata.o
obj-$(CONFIG_LIBATA) += libata.o
obj-$(CONFIG_MVSATA_IDE) += mvsata_ide.o
obj-$(CONFIG_SATA) += sata.o
obj-$(CONFIG_SATA_CEVA) += sata_ceva.o
obj-$(CONFIG_SATA_MV) += sata_mv.o
obj-$(CONFIG_SATA_SIL3114) += sata_sil3114.o
obj-$(CONFIG_SATA_SIL) += sata_sil.o
obj-$(CONFIG_SANDBOX) += sata_sandbox.o
obj-$(CONFIG_AHCI_MVEBU) += ahci_mvebu.o
ifneq ($(CONFIG_TARGET_NA51103)$(CONFIG_TARGET_NA51103_A64),)
obj-$(CONFIG_AHCI_NVT) += ahci_nvt.o ahci_nvt_na51103.o
else ifneq ($(CONFIG_TARGET_NA51090)$(CONFIG_TARGET_NA51090_A64),)
obj-$(CONFIG_AHCI_NVT) += ahci_nvt.o ahci_nvt_na51090.o
else ifneq ($(CONFIG_TARGET_NA51102)$(CONFIG_TARGET_NA51102_A64),)
obj-$(CONFIG_AHCI_NVT) += ahci_nvt.o ahci_nvt_na51102.o
endif
