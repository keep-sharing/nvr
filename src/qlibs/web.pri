HEADERS += \
    $$files($$PWD/../web/*.h, true)

SOURCES += \
    $$files($$PWD/../web/*.c, true)

DISTFILES += \
    $$files($$PWD/../web/*.css, true) \
    $$files($$PWD/../web/*.html, true) \
    $$files($$PWD/../web/*.js, true) \
    $$files($$PWD/../web/*.json, true)
