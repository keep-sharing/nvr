INCLUDEPATH += \
    $$PWD/../public/include/vapi \
    $$PWD/../public/include/vapi/$$(PLATFORM_TYPE) \
    $$PWD/../public/include/vapi/$$(PLATFORM_TYPE)/svp_npu \

HEADERS += \
    $$PWD/../public/include/vapi/*.h \
    $$PWD/../public/include/vapi/$$(PLATFORM_TYPE)/*.h \
    $$PWD/../public/include/vapi/$$(PLATFORM_TYPE)/svp_npu/*.h \
    $$PWD/../libs/vapi/$$(PLATFORM_TYPE)/*.h

SOURCES += $$PWD/../libs/vapi/$$(PLATFORM_TYPE)/*.c
