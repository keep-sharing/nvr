INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/AbstractTestItem.h \
    $$PWD/TestHardware.h \
    $$PWD/TestHardwareData.h \
    $$PWD/TestItemCommon.h \
    $$PWD/TestWizard.h

SOURCES += \
    $$PWD/AbstractTestItem.cpp \
    $$PWD/TestHardware.cpp \
    $$PWD/TestHardwareData.cpp \
    $$PWD/TestItemCommon.cpp \
    $$PWD/TestWizard.cpp

FORMS += \
    $$PWD/TestHardware.ui \
    $$PWD/TestItemCommon.ui \
    $$PWD/TestWizard.ui

LIBS += -L$$(PUBLIC_LIB_DIR) -lmshw

QT += widgets \
    widgets \
    widgets
