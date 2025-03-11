# Copyright (C) 2015-2021 Novatek corp.
#
# SPDX-License-Identifier:	GPL-2.0+
#
KBUILD_CFLAGS := -fno-builtin -Werror -Iboard/$(BOARDDIR)/../common/include -fshort-wchar -Iinclude

obj-$(CONFIG_NVT_PCIE_MAPPING) += nvt_pcie_mapping.o nvt_pcie_fdt.o
obj-$(CONFIG_NVT_PCIE) += nvt_pcie_utils.o
obj-$(CONFIG_NVT_PCIE_DAUGHTER_BRD) += nvt_pcie_ep.o
