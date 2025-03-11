# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2021 Novateck, Inc
#
ifeq ($(CONFIG_TARGET_NA51090_A64),y)
obj-$(CONFIG_VIDEO_LOGO) += drv_videoout/
else ifeq ($(CONFIG_TARGET_NA51103)$(CONFIG_TARGET_NA51103_A64),y)
obj-$(CONFIG_VIDEO_LOGO) += drv_videoout/
else
obj-$(CONFIG_VIDEO_LOGO) += drv_videoout/ nvt_panel/
endif
