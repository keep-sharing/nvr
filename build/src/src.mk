
#src build目录
BUILD_SRC_DIR	:= $(BUILD_DIR)/src

#src通用mk文件.导出,为src下的具体makefile文件所使用
BUILD_COMMON_MK	:= $(BUILD_DIR)/src/common.mk
BUILD_RULES_MK	:= $(BUILD_DIR)/src/rules.mk

export BUILD_COMMON_MK
export BUILD_RULES_MK

include $(BUILD_SRC_DIR)/rules.mk
include $(BUILD_SRC_DIR)/arch.mk
#include $(BUILD_SRC_DIR)/definition.mk

#include $(BUILD_SRC_DIR)/backup/*.mk
include $(BUILD_SRC_DIR)/boards/*.mk
include $(BUILD_SRC_DIR)/daemon/*.mk
include $(BUILD_SRC_DIR)/dbtools/*.mk
include $(BUILD_SRC_DIR)/dhcpcd/*.mk
include $(BUILD_SRC_DIR)/gui/*.mk
include $(BUILD_SRC_DIR)/libs/*.mk
include $(BUILD_SRC_DIR)/mscli/*.mk
include $(BUILD_SRC_DIR)/msmail/*.mk
include $(BUILD_SRC_DIR)/mssp/*.mk
include $(BUILD_SRC_DIR)/profile/*.mk
#include $(BUILD_SRC_DIR)/showlogo/*.mk
include $(BUILD_SRC_DIR)/smtp/*.mk
include $(BUILD_SRC_DIR)/sysconf/*.mk
include $(BUILD_SRC_DIR)/test/*.mk
include $(BUILD_SRC_DIR)/tools/*.mk
include $(BUILD_SRC_DIR)/update/*.mk
#include $(BUILD_SRC_DIR)/demo/*.mk
include $(BUILD_SRC_DIR)/fio/*.mk
include $(BUILD_SRC_DIR)/mskb/*.mk
include $(BUILD_SRC_DIR)/msjs/*.mk
include $(BUILD_SRC_DIR)/snmp/*.mk
include $(BUILD_SRC_DIR)/dhcpv6/*.mk
include $(BUILD_SRC_DIR)/pppd/*.mk
include $(BUILD_SRC_DIR)/rp-pppoe/*.mk
include $(BUILD_SRC_DIR)/dbtar/*.mk
include $(BUILD_SRC_DIR)/tsar/*.mk
include $(BUILD_SRC_DIR)/msop/*.mk
include $(BUILD_SRC_DIR)/miniqt/*.mk


