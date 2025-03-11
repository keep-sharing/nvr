#DRV_PATH := $(srctree)/product/ot_osd/hdmi

HDMI_VER := hdmi_2_0
ifeq ($(CONFIG_PRODUCTNAME), "ss626v100")
VO_ARCH_NAME = ss626v100
VO_SUB_ARCH_NAME = ss626v100
CHIP_TYPE := ss626v100
CTRL      := v300
PHY       := v300
cflags-y  += -DHDMI_PRODUCT_SS626V100
cflags-y  += -DCHIP_TYPE_ss626v100
cflags-y  += -DHDMI_PLATFORM_FMT
cflags-y  += -DHDMI_SUPPORT_DUAL_CHANNEL
endif

CTRL_DIR    := $(srctree)/hal/ctrl/$(CTRL)
PHY_DIR     := $(srctree)/hal/phy/$(PHY)
PRODUCT_DIR := $(srctree)/product/$(CHIP_TYPE)

cflags-y     += -I$(srctree)/product/ot_osd/hdmi/$(HDMI_VER)
cflags-y     += -I$(srctree)/product/ot_osd/hdmi/$(HDMI_VER)/drv
cflags-y     += -I$(srctree)/product/ot_osd/hdmi/$(HDMI_VER)/drv/hal
cflags-y     += -I$(srctree)/product/ot_osd/hdmi/$(HDMI_VER)/drv/hal/ctrl/$(CTRL)
cflags-y     += -I$(srctree)/product/ot_osd/hdmi/$(HDMI_VER)/drv/hal/ctrl/$(CTRL)/regs
cflags-y     += -I$(srctree)/product/ot_osd/hdmi/$(HDMI_VER)/drv/hal/emi
cflags-y     += -I$(srctree)/product/ot_osd/hdmi/$(HDMI_VER)/drv/hal/phy/$(PHY)
cflags-y     += -I$(srctree)/product/ot_osd/hdmi/$(HDMI_VER)/drv/hal/phy/$(PHY)/regs
cflags-y     += -I$(srctree)/product/ot_osd/hdmi/$(HDMI_VER)/drv/product/$(CHIP_TYPE)
cflags-y     += -I$(srctree)/product/ot_osd/hdmi/$(HDMI_VER)/drv/product/$(CHIP_TYPE)/regs
cflags-y     += -I$(srctree)/include
cflags-y     += -I$(PRODUCT_DIR)
cflags-y     += -I$(PRODUCT_DIR)/regs
cflags-y     += -I$(srctree)/product/ot_osd/vo/arch/$(VO_ARCH_NAME)/include
cflags-y     += -I$(srctree)/product/ot_osd/vo/arch/include
cflags-y     += -I$(srctree)/product/ot_osd/vo/include
cflags-y     += -I$(srctree)/product/ot_osd/vo/arch/$(VO_ARCH_NAME)/include/$(VO_SUB_ARCH_NAME)
cflags-y     += -I$(srctree)/product/ot_osd/include

cflags-y += -DHDMI_SUPPORT_LOGIC_VENDORV100
cflags-y += -DCONFIG_HDMI_VENDOR_SDK -DDVI_SUPPORT
cflags-y += -DDVI_SUPPORT
cflags-y += -DHDMI_SCDC_SUPPORT

ifeq ($(FPGA),y)
cflags-y  += -DHDMI_FPGA_SUPPORT
endif

ccflags-y  += $(cflags-y)
HOSTCFLAGS += $(cflags-y)
CPPFLAGS   += $(cflags-y)

#HDMI_EMI_ENABLE

OBJ    := ../boot_hdmi_intf.o
OBJ    += drv_hdmi_common.o
OBJ    += drv_hdmi_infoframe.o
OBJ    += $(CTRL_DIR)/hdmi_hal_intf.o

OBJ    +=   $(CTRL_DIR)/regs/hdmi_reg_aon.o \
            $(CTRL_DIR)/regs/hdmi_reg_ctrl.o \
            $(CTRL_DIR)/regs/hdmi_reg_tx.o \
            $(CTRL_DIR)/regs/hdmi_reg_video_path.o

OBJ += $(PRODUCT_DIR)/regs/hdmi_reg_crg.o

OBJ    +=   $(CTRL_DIR)/hdmi_hal_intf.o \
            $(CTRL_DIR)/hdmi_hal_ddc.o \
            $(CTRL_DIR)/hdmi_hal_ctrl.o \
            $(CTRL_DIR)/hdmi_hal_scdc.o

OBJ += $(PHY_DIR)/hdmi_hal_phy.o
OBJ += $(PHY_DIR)/regs/hdmi_reg_dphy.o
OBJ += $(PRODUCT_DIR)/hdmi_product_define.o

obj-y += $(OBJ)

#########################################################################
