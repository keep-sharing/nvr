INCLUDEPATH += $$PWD

include($$PWD/ModuleStatus-PacketCapture/ModuleStatus-PacketCapture.pri)
include($$PWD/ModuleStatus-NetworkStatus/ModuleStatus-NetworkStatus.pri)

FORMS += \
    $$PWD/LogDetail.ui \
    $$PWD/ModuleStatus.ui \
    $$PWD/PageCameraStatus.ui \
    $$PWD/PageDeviceInformation.ui \
    $$PWD/PageDiskStatus.ui \
    $$PWD/PageEventStatus.ui \
    $$PWD/PageGroupStatus.ui \
    $$PWD/PageLogStatus.ui \
    $$PWD/PageNetworkStatus.ui \
    $$PWD/PageOnlineUser.ui \
    $$PWD/TabCameraStatus.ui \
    $$PWD/TabDiskHealthDetection.ui \
    $$PWD/TabDiskStatus.ui \
    $$PWD/TabPoeStatus.ui \
    $$PWD/TabSmartStatus.ui

HEADERS += \
    $$PWD/DiskSmartTest.h \
    $$PWD/LogDetail.h \
    $$PWD/ModuleStatus.h \
    $$PWD/PageCameraStatus.h \
    $$PWD/PageDeviceInformation.h \
    $$PWD/PageDiskStatus.h \
    $$PWD/PageEventStatus.h \
    $$PWD/PageGroupStatus.h \
    $$PWD/PageLogStatus.h \
    $$PWD/PageNetworkStatus.h \
    $$PWD/PageOnlineUser.h \
    $$PWD/TabCameraStatus.h \
    $$PWD/TabDiskHealthDetection.h \
    $$PWD/TabDiskStatus.h \
    $$PWD/TabPoeStatus.h \
    $$PWD/TabSmartStatus.h

SOURCES += \
    $$PWD/DiskSmartTest.cpp \
    $$PWD/LogDetail.cpp \
    $$PWD/ModuleStatus.cpp \
    $$PWD/PageCameraStatus.cpp \
    $$PWD/PageDeviceInformation.cpp \
    $$PWD/PageDiskStatus.cpp \
    $$PWD/PageEventStatus.cpp \
    $$PWD/PageGroupStatus.cpp \
    $$PWD/PageLogStatus.cpp \
    $$PWD/PageNetworkStatus.cpp \
    $$PWD/PageOnlineUser.cpp \
    $$PWD/TabCameraStatus.cpp \
    $$PWD/TabDiskHealthDetection.cpp \
    $$PWD/TabDiskStatus.cpp \
    $$PWD/TabPoeStatus.cpp \
    $$PWD/TabSmartStatus.cpp
