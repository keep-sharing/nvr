include($$PWD/ModuleCamera-Management/ModuleCamera-Management.pri)
include($$PWD/ModuleCamera-DeviceSearch/ModuleCamera-DeviceSearch.pri)
include($$PWD/ModuleCamera-PtzConfiguration/ModuleCamera-PtzConfiguration.pri)
include($$PWD/ModuleCamera-Image/ModuleCamera-Image.pri)
include($$PWD/ModuleCamera-Audio/ModuleCamera-Audio.pri)
include($$PWD/ModuleCamera-Advanced/ModuleCamera-Advanced.pri)
include($$PWD/ModuleCamera-Maintenance/ModuleCamera-Maintenance.pri)

INCLUDEPATH += $$PWD

FORMS += \
    $$PWD/ModuleCamera.ui \
    $$PWD/camerachanneladd.ui \
    $$PWD/authentication.ui

HEADERS += \
    $$PWD/ModuleCamera.h \
    $$PWD/camerachanneladd.h \
    $$PWD/abstractcamerapage.h \
    $$PWD/authentication.h

SOURCES += \
    $$PWD/ModuleCamera.cpp \
    $$PWD/camerachanneladd.cpp \
    $$PWD/abstractcamerapage.cpp \
    $$PWD/authentication.cpp
