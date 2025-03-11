include($$PWD/ModuleSettings-General/ModuleSettings-General.pri)
include($$PWD/ModuleSettings-Layout/ModuleSettings-Layout.pri)
include($$PWD/ModuleSettings-Network/ModuleSettings-Network.pri)
include($$PWD/ModuleSettings-Audio/ModuleSettings-Audio.pri)
include($$PWD/ModuleSettings-Holiday/ModuleSettings-Holiday.pri)
include($$PWD/ModuleSettings-User/ModuleSettings-User.pri)
include($$PWD/ModuleSettings-AccessFilter/ModuleSettings-AccessFilter.pri)
include($$PWD/ModuleSettings-Maintenance/ModuleSettings-Maintenance.pri)
include($$PWD/ModuleSettings-HotSpare/ModuleSettings-HotSpare.pri)

INCLUDEPATH += $$PWD

FORMS += \
    $$PWD/ModuleSettings.ui

HEADERS += \
    $$PWD/ModuleSettings.h

SOURCES += \
    $$PWD/ModuleSettings.cpp
