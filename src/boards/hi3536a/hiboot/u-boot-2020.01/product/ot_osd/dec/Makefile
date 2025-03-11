obj-y += jpegd_entry.o
obj-y += jpegd.o
obj-y += jpegd_drv.o
obj-y += jpegd_image.o
ccflags-y  += -I$(srctree)/product/ot_osd/include
ifeq ($(CONFIG_PRODUCTNAME), "ss919v100")
ccflags-y  += -DCHIP_SS919V100
else ifeq ($(CONFIG_PRODUCTNAME), "ss015v100")
ccflags-y  += -DCHIP_SS015V100
else ifeq ($(CONFIG_PRODUCTNAME), "ss918v100")
ccflags-y  += -DCHIP_SS918V100
else ifeq ($(CONFIG_PRODUCTNAME), "ss013v100")
ccflags-y  += -DCHIP_SS918V100
else ifeq ($(CONFIG_PRODUCTNAME), "ss813v100")
ccflags-y  += -DCHIP_SS812V100
else ifeq ($(CONFIG_PRODUCTNAME), "ss313v100")
ccflags-y  += -DCHIP_SS812V100
else ifeq ($(CONFIG_PRODUCTNAME), "ss312v100")
ccflags-y  += -DCHIP_SS812V100
else ifeq ($(CONFIG_PRODUCTNAME), "ss815v100")
ccflags-y  += -DCHIP_SS812V100
else ifeq ($(CONFIG_PRODUCTNAME), "ss528v100")
ccflags-y  += -DCHIP_SS528V100
else ifeq ($(CONFIG_PRODUCTNAME), "ss625v100")
ccflags-y  += -DCHIP_SS625V100
else ifeq ($(CONFIG_PRODUCTNAME), "ss524v100")
ccflags-y  += -DCHIP_SS524V100
else ifeq ($(CONFIG_PRODUCTNAME), "ss615v100")
ccflags-y  += -DCHIP_SS615V100
else ifeq ($(CONFIG_PRODUCTNAME), "ss522v100")
ccflags-y  += -DCHIP_SS522V100
else ifeq ($(CONFIG_PRODUCTNAME),$(filter $(CONFIG_PRODUCTNAME), "ss928v100" "ss000v100"))
ccflags-y  += -DCHIP_SS928V100
else ifeq ($(CONFIG_PRODUCTNAME), "ss626v100")
ccflags-y  += -DCHIP_SS626V100
endif
