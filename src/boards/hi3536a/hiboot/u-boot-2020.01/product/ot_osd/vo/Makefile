ifeq ($(CONFIG_PRODUCTNAME), "ss524v100")
VO_ARCH_NAME = ss524v100
else ifeq ($(CONFIG_PRODUCTNAME), "ss522v100")
VO_ARCH_NAME = ss524v100
else ifeq ($(CONFIG_PRODUCTNAME), "ss615v100")
VO_ARCH_NAME = ss615v100
else ifeq ($(CONFIG_PRODUCTNAME), "ss528v100")
VO_ARCH_NAME = ss528v100
VO_SUB_ARCH_NAME = ss528v100
else ifeq ($(CONFIG_PRODUCTNAME), "ss625v100")
VO_ARCH_NAME = ss625v100
VO_SUB_ARCH_NAME = ss625v100
else ifeq ($(CONFIG_PRODUCTNAME),$(filter $(CONFIG_PRODUCTNAME), "ss928v100" "ss000v100"))
VO_ARCH_NAME = ss928v100
else ifeq ($(CONFIG_PRODUCTNAME), "ss626v100")
VO_ARCH_NAME = ss626v100
VO_SUB_ARCH_NAME = ss626v100
endif

cflags-y     += -I$(srctree)/product/ot_osd/include
cflags-y     += -I$(srctree)/product/ot_osd/vo/ext_inc
cflags-y     += -I$(srctree)/product/ot_osd/vo/include
cflags-y     += -I$(srctree)/product/ot_osd/vo/mkp/include
cflags-y     += -I$(srctree)/product/ot_osd/vo/mkp/src
cflags-y     += -I$(srctree)/product/ot_osd/vo/mkp/src/dev
cflags-y     += -I$(srctree)/product/ot_osd/vo/mkp/src/intf
cflags-y     += -I$(srctree)/product/ot_osd/vo/mkp/src/layer
cflags-y     += -I$(srctree)/product/ot_osd/vo/mkp/src/common
cflags-y     += -I$(srctree)/product/ot_osd/vo/arch/include
cflags-y     += -I$(srctree)/product/ot_osd/vo/arch/comm/src
cflags-y     += -I$(srctree)/product/ot_osd/vo/arch/comm/include
cflags-y     += -I$(srctree)/product/ot_osd/vo/arch/$(VO_ARCH_NAME)/include
cflags-y     += -I$(srctree)/product/ot_osd/vo/arch/include
cflags-y     += -I$(srctree)/product/ot_osd/vo/arch/$(VO_ARCH_NAME)/include/$(VO_SUB_ARCH_NAME)

ccflags-y  += $(cflags-y)
#HOSTCFLAGS += $(cflags-y)
#CPPFLAGS   += $(cflags-y)

CBB_VO_ARCH_COMM_SRC = arch/comm/src
CBB_VO_ARCH_HAL = arch/$(VO_ARCH_NAME)/hal

#enum mkp src files
obj-y += mkp/src/vo.o
obj-y += mkp/src/mkp_vo_user.o
obj-y += mkp/src/dev/mkp_vo_dev.o
obj-y += mkp/src/intf/mkp_vo_intf.o
obj-y += mkp/src/layer/mkp_vo_video.o
obj-y += mkp/src/layer/mkp_vo_gfx.o
obj-y += mkp/src/common/mkp_vo_comm.o
obj-y += mkp/src/common/mkp_vo_init.o
obj-y += mkp/src/mkp_vo_bmp.o

#enum arch comm src files
obj-y += $(CBB_VO_ARCH_COMM_SRC)/drv_vo_comm.o
obj-y += $(CBB_VO_ARCH_COMM_SRC)/drv_vo_dev_comm.o
obj-y += $(CBB_VO_ARCH_COMM_SRC)/drv_vo_layer_comm.o
obj-y += $(CBB_VO_ARCH_COMM_SRC)/drv_vo_video_comm.o
obj-y += $(CBB_VO_ARCH_COMM_SRC)/drv_vo_gfx_comm.o
obj-y += $(CBB_VO_ARCH_COMM_SRC)/drv_vo_coef_org_comm.o
obj-y += $(CBB_VO_ARCH_COMM_SRC)/hal_vo_comm.o
obj-y += $(CBB_VO_ARCH_COMM_SRC)/hal_vo_dev_comm.o
obj-y += $(CBB_VO_ARCH_COMM_SRC)/hal_vo_layer_comm.o
obj-y += $(CBB_VO_ARCH_COMM_SRC)/hal_vo_video_comm.o
obj-y += $(CBB_VO_ARCH_COMM_SRC)/hal_vo_gfx_comm.o

#enum vo hal src files
obj-y += $(CBB_VO_ARCH_HAL)/drv_vo.o
obj-y += $(CBB_VO_ARCH_HAL)/drv_vo_dev.o
obj-y += $(CBB_VO_ARCH_HAL)/drv_vo_video.o
obj-y += $(CBB_VO_ARCH_HAL)/drv_vo_gfx.o
obj-y += $(CBB_VO_ARCH_HAL)/hal_vo.o
obj-y += $(CBB_VO_ARCH_HAL)/hal_vo_dev.o
obj-y += $(CBB_VO_ARCH_HAL)/hal_vo_video.o
obj-y += $(CBB_VO_ARCH_HAL)/hal_vo_gfx.o

#enum vo sys hal src files
obj-y += $(CBB_VO_ARCH_HAL)/sys_drv.o
obj-y += $(CBB_VO_ARCH_HAL)/sys_hal.o

ifeq ($(CONFIG_PRODUCTNAME), "ss528v100")
ccflags-y += -DCHIP_SS528V100
else ifeq ($(CONFIG_PRODUCTNAME), "ss625v100")
ccflags-y += -DCHIP_SS625V100
else ifeq ($(CONFIG_PRODUCTNAME), "ss524v100")
ccflags-y += -DCHIP_SS524V100
else ifeq ($(CONFIG_PRODUCTNAME), "ss615v100")
ccflags-y += -DCHIP_SS615V100
else ifeq ($(CONFIG_PRODUCTNAME), "ss522v100")
ccflags-y += -DCHIP_SS522V100
else ifeq ($(CONFIG_PRODUCTNAME),$(filter $(CONFIG_PRODUCTNAME), "ss928v100" "ss000v100"))
ccflags-y += -DCHIP_SS928V100
else ifeq ($(CONFIG_PRODUCTNAME), "ss626v100")
ccflags-y += -DCHIP_SS626V100
endif

# module macro
ifeq ($(CONFIG_PRODUCTNAME),$(filter $(CONFIG_PRODUCTNAME), "ss528v100" "ss625v100" "ss524v100" "ss522v100" "ss615v100" "ss928v100" "ss000v100" "ss626v100"))
ccflags-y += -DCONFIG_OT_HDMI_SUPPORT
endif

# function macro
ifeq ($(CONFIG_PRODUCTNAME), "ss528v100")
ccflags-y += -DCONFIG_OT_VO_CVBS
ccflags-y += -DCONFIG_OT_VO_VGA
ccflags-y += -DCONFIG_OT_VO_BT1120
ccflags-y += -DCONFIG_OT_VO_HDMI
ccflags-y += -DCONFIG_OT_VO_BT1120_DUAL_EDGE
# CONFIG_OT_VO_RGB is not set
# CONFIG_OT_VO_MIPI is not set
# CONFIG_OT_VO_HDMI1 is not set
else ifeq ($(CONFIG_PRODUCTNAME), "ss625v100")
ccflags-y += -DCONFIG_OT_VO_CVBS
ccflags-y += -DCONFIG_OT_VO_VGA
ccflags-y += -DCONFIG_OT_VO_BT1120
ccflags-y += -DCONFIG_OT_VO_HDMI
# CONFIG_OT_VO_RGB is not set
# CONFIG_OT_VO_MIPI is not set
# CONFIG_OT_VO_HDMI1 is not set
# CONFIG_OT_VO_BT1120_DUAL_EDGE is not set
else ifeq ($(CONFIG_PRODUCTNAME), "ss524v100")
ccflags-y += -DCONFIG_OT_VO_CVBS
ccflags-y += -DCONFIG_OT_VO_VGA
ccflags-y += -DCONFIG_OT_VO_RGB
ccflags-y += -DCONFIG_OT_VO_BT1120
ccflags-y += -DCONFIG_OT_VO_HDMI
# CONFIG_OT_VO_MIPI is not set
# CONFIG_OT_VO_HDMI1 is not set
# CONFIG_OT_VO_BT1120_DUAL_EDGE is not set
else ifeq ($(CONFIG_PRODUCTNAME), "ss522v100")
ccflags-y += -DCONFIG_OT_VO_CVBS
ccflags-y += -DCONFIG_OT_VO_VGA
ccflags-y += -DCONFIG_OT_VO_RGB
ccflags-y += -DCONFIG_OT_VO_BT1120
ccflags-y += -DCONFIG_OT_VO_HDMI
# CONFIG_OT_VO_MIPI is not set
# CONFIG_OT_VO_HDMI1 is not set
# CONFIG_OT_VO_BT1120_DUAL_EDGE is not set
else ifeq ($(CONFIG_PRODUCTNAME), "ss615v100")
ccflags-y += -DCONFIG_OT_VO_CVBS
ccflags-y += -DCONFIG_OT_VO_VGA
ccflags-y += -DCONFIG_OT_VO_RGB
ccflags-y += -DCONFIG_OT_VO_BT1120
ccflags-y += -DCONFIG_OT_VO_HDMI
# CONFIG_OT_VO_MIPI is not set
# CONFIG_OT_VO_HDMI1 is not set
# CONFIG_OT_VO_BT1120_DUAL_EDGE is not set
else ifeq ($(CONFIG_PRODUCTNAME),$(filter $(CONFIG_PRODUCTNAME), "ss928v100" "ss000v100"))
ccflags-y += -DCONFIG_OT_VO_CVBS
ccflags-y += -DCONFIG_OT_VO_RGB
ccflags-y += -DCONFIG_OT_VO_MIPI
ccflags-y += -DCONFIG_OT_VO_BT1120
ccflags-y += -DCONFIG_OT_VO_HDMI
# CONFIG_OT_VO_VGA is not set
# CONFIG_OT_VO_HDMI1 is not set
# CONFIG_OT_VO_BT1120_DUAL_EDGE is not set
else ifeq ($(CONFIG_PRODUCTNAME), "ss626v100")
ccflags-y += -DCONFIG_OT_VO_CVBS
ccflags-y += -DCONFIG_OT_VO_VGA
ccflags-y += -DCONFIG_OT_VO_BT1120
ccflags-y += -DCONFIG_OT_VO_HDMI
ccflags-y += -DCONFIG_OT_VO_HDMI1
ccflags-y += -DCONFIG_OT_VO_BT1120_DUAL_EDGE
# CONFIG_OT_VO_RGB is not set
# CONFIG_OT_VO_MIPI is not set
endif
