HEADERS += \
    $$PWD/../boards/$$(PLATFORM_TYPE)/firmware/env/*.h \
    $$PWD/../boards/$$(PLATFORM_TYPE)/firmware/FWupdate/*.h

SOURCES += \
    $$PWD/../boards/$$(PLATFORM_TYPE)/firmware/env/*.c \
    $$PWD/../boards/$$(PLATFORM_TYPE)/firmware/FWupdate/*.c
