head = \
    $$PWD/../public/include/msrtsp/*.h \
    $$files($$PWD/../libs/msrtsp/*.h, true)

HEADERS += \
    $$head

SOURCES += \
    $$files($$PWD/../libs/msrtsp/*.c, true)

incpath = $$dirname(head)
INCLUDEPATH += $$incpath
