INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/AbstractKeyboard.h \
    $$PWD/KeyButton.h \
    $$PWD/Keyboard.h \
    $$PWD/KeyboardData.h \
    $$PWD/KeyboardPolski.h \
    $$PWD/KeyboardWidget.h \
    $$PWD/MyInputMethod.h \
    $$PWD/key.h

SOURCES += \
    $$PWD/AbstractKeyboard.cpp \
    $$PWD/KeyButton.cpp \
    $$PWD/Keyboard.cpp \
    $$PWD/KeyboardData.cpp \
    $$PWD/KeyboardPolski.cpp \
    $$PWD/KeyboardWidget.cpp \
    $$PWD/MyInputMethod.cpp \
    $$PWD/key.cpp

FORMS += \
    $$PWD/Keyboard.ui \
    $$PWD/KeyboardPolski.ui \
    $$PWD/KeyboardWidget.ui

QT += widgets \
    widgets
