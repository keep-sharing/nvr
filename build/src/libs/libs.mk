
LIBS_BUILD_DIR = $(BUILD_SRC_DIR)/libs

#include $(LIBS_BUILD_DIR)/*/*.mk
#shared lib
include $(LIBS_BUILD_DIR)/msstd/*.mk
include $(LIBS_BUILD_DIR)/vapi/*.mk
include $(LIBS_BUILD_DIR)/osa/*.mk
include $(LIBS_BUILD_DIR)/msdb/*.mk
include $(LIBS_BUILD_DIR)/mssocket/*.mk
include $(LIBS_BUILD_DIR)/msptz/*.mk
include $(LIBS_BUILD_DIR)/avilib/*.mk
#include $(LIBS_BUILD_DIR)/cellular/*.mk
include $(LIBS_BUILD_DIR)/encrypt/*.mk
include $(LIBS_BUILD_DIR)/log/*.mk
#include $(LIBS_BUILD_DIR)/json/*.mk
#include $(LIBS_BUILD_DIR)/curl-7.46.0/*.mk
include $(LIBS_BUILD_DIR)/gpio/*.mk
include $(LIBS_BUILD_DIR)/tde/*.mk
include $(LIBS_BUILD_DIR)/poe/*.mk
include $(LIBS_BUILD_DIR)/qrencode/*.mk
include $(LIBS_BUILD_DIR)/msfs/*.mk
include $(LIBS_BUILD_DIR)/mscrypt/*.mk
include $(LIBS_BUILD_DIR)/tbox/*.mk
include $(LIBS_BUILD_DIR)/sdkp2p/*.mk
include $(LIBS_BUILD_DIR)/msini/*.mk
include $(LIBS_BUILD_DIR)/msmov/*.mk
#static lib
#include $(LIBS_BUILD_DIR)/gpio/*.mk
#include $(LIBS_BUILD_DIR)/stepper/*.mk
#include $(LIBS_BUILD_DIR)/lens/*.mk
#include $(LIBS_BUILD_DIR)/msonvif/*.mk
#include $(LIBS_BUILD_DIR)/upnp/*.mk
#include $(LIBS_BUILD_DIR)/miniupnp/*.mk
#include $(LIBS_BUILD_DIR)/osip/*.mk
#include $(LIBS_BUILD_DIR)/ortp/*.mk
#include $(LIBS_BUILD_DIR)/eXosip/*.mk
include $(LIBS_BUILD_DIR)/chipinfo/*.mk
include $(LIBS_BUILD_DIR)/libnl/*.mk
include $(LIBS_BUILD_DIR)/libuv/*.mk
include $(LIBS_BUILD_DIR)/fisheye/*.mk
#milesight rtsp
include $(LIBS_BUILD_DIR)/msrtsp/*.mk
include $(LIBS_BUILD_DIR)/recortsp/*.mk
#
include $(LIBS_BUILD_DIR)/mshw/*.mk
include $(LIBS_BUILD_DIR)/sds/*.mk
