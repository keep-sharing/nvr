HEADERS += \
    $$PWD/include/parser.h \
    $$PWD/include/serializer.h \
    $$PWD/include/qjson_export.h

LIBS += -L$$PWD/lib/$$(PLATFORM_TYPE) -lQJson
