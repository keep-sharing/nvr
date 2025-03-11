CHIP_V3_CIPHER := "ss928v100" "ss000v100" "ss626v100"

ifneq ($(filter $(CONFIG_PRODUCTNAME), $(CHIP_V3_CIPHER)), )
include $(srctree)/product/security_subsys/cipher/v3/src/build.mak
else
include $(srctree)/product/security_subsys/cipher/v2/build.mak
endif
