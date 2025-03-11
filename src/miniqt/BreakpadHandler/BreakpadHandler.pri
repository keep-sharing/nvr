INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/BreakpadHandler.h

SOURCES += \
    $$PWD/BreakpadHandler.cpp


unix:!macx: LIBS += -L$$PWD/../../msgui/third_party/breakpad-build/lib/$$(PLATFORM_TYPE)/ -lqBreakpad

INCLUDEPATH += $$PWD/../../msgui/third_party/breakpad-build/include/breakpad
DEPENDPATH += $$PWD/../../msgui/third_party/breakpad-build/include/breakpad

unix:!macx: PRE_TARGETDEPS += $$PWD/../../msgui/third_party/breakpad-build/lib/$$(PLATFORM_TYPE)/libqBreakpad.a
