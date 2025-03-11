INCLUDEPATH += \
    $$PWD/../public/include/msrtsp \
    $$PWD/../public/include/rtsp

HEADERS += \
    $$files($$PWD/../public/include/rtsp/*.h, true) \
    $$files($$PWD/../libs/recortsp/*.h, true)

SOURCES += \
    $$files($$PWD/../libs/recortsp/*.c, true)
