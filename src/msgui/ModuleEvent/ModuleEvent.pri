include($$PWD/ModuleEvent-AlarmInput/ModuleEvent-AlarmInput.pri)
include($$PWD/ModuleEvent-AlarmOutput/ModuleEvent-AlarmOutput.pri)
include($$PWD/ModuleEvent-Exception/ModuleEvent-Exception.pri)
include($$PWD/ModuleEvent-MotionDetection/ModuleEvent-MotionDetection.pri)
include($$PWD/ModuleEvent-VCA/ModuleEvent-VCA.pri)
include($$PWD/ModuleEvent-VideoLoss/ModuleEvent-VideoLoss.pri)
include($$PWD/ModuleEvent-AudioAlarm/ModuleEvent-AudioAlarm.pri)

INCLUDEPATH += $$PWD

FORMS += \
    $$PWD/ModuleEvent.ui

HEADERS += \
    $$PWD/ModuleEvent.h

SOURCES += \
    $$PWD/ModuleEvent.cpp
