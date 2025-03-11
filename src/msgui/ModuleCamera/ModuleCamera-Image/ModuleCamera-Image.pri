include($$PWD/exposure_schedule/exposure_schedule.pri)

INCLUDEPATH += $$PWD

FORMS += \
    $$PWD/DayNightSchedule.ui \
    $$PWD/EditImageRegionType.ui \
    $$PWD/ImagePageDisplayMulti.ui \
    $$PWD/ImagePageEnhancementMulti.ui \
    $$PWD/imagepagedaynight.ui \
    $$PWD/ImagePageDayNightMulti.ui \
    $$PWD/imagepagedisplay.ui \
    $$PWD/imagepageenhancement.ui \
    $$PWD/imagepagemask.ui \
    $$PWD/imagepageosd.ui \
    $$PWD/imagepageroi.ui \
    $$PWD/imagesetting.ui \
    $$PWD/manualmodesettings.ui \
    $$PWD/whitebalanceschedule.ui \
    $$PWD/daynightscheedit.ui \
    $$PWD/daynightscheadd.ui \
    $$PWD/bwhschedule.ui \
    $$PWD/addmanualmode.ui \
    $$PWD/editmanualmode.ui

HEADERS += \
    $$PWD/DayNightScheTimeEdit.h \
    $$PWD/DayNightSchedule.h \
    $$PWD/EditImageRegionType.h \
    $$PWD/ImagePageDayNightMulti.h \
    $$PWD/ImagePageDisplayMulti.h \
    $$PWD/ImagePageEnhancementMulti.h \
    $$PWD/abstractimagepage.h \
    $$PWD/blcwidget.h \
    $$PWD/drawenhancement.h \
    $$PWD/drawroi.h \
    $$PWD/imagepagedaynight.h \
    $$PWD/imagepagedisplay.h \
    $$PWD/imagepageenhancement.h \
    $$PWD/imagepagemask.h \
    $$PWD/imagepageosd.h \
    $$PWD/imagepageroi.h \
    $$PWD/imagescheduleedit.h \
    $$PWD/imagesetting.h \
    $$PWD/manualmodesettings.h \
    $$PWD/roiwidget.h \
    $$PWD/whitebalanceschedule.h \
    $$PWD/daynightscheedit.h \
    $$PWD/daynightscheadd.h \
    $$PWD/bwhschedule.h \
    $$PWD/addmanualmode.h \
    $$PWD/editmanualmode.h

SOURCES += \
    $$PWD/DayNightScheTimeEdit.cpp \
    $$PWD/DayNightSchedule.cpp \
    $$PWD/EditImageRegionType.cpp \
    $$PWD/ImagePageDayNightMulti.cpp \
    $$PWD/ImagePageDisplayMulti.cpp \
    $$PWD/ImagePageEnhancementMulti.cpp \
    $$PWD/abstractimagepage.cpp \
    $$PWD/blcwidget.cpp \
    $$PWD/drawenhancement.cpp \
    $$PWD/drawroi.cpp \
    $$PWD/imagepagedaynight.cpp \
    $$PWD/imagepagedisplay.cpp \
    $$PWD/imagepageenhancement.cpp \
    $$PWD/imagepagemask.cpp \
    $$PWD/imagepageosd.cpp \
    $$PWD/imagepageroi.cpp \
    $$PWD/imagescheduleedit.cpp \
    $$PWD/imagesetting.cpp \
    $$PWD/manualmodesettings.cpp \
    $$PWD/roiwidget.cpp \
    $$PWD/whitebalanceschedule.cpp \
    $$PWD/daynightscheedit.cpp \
    $$PWD/daynightscheadd.cpp \
    $$PWD/bwhschedule.cpp \
    $$PWD/addmanualmode.cpp \
    $$PWD/editmanualmode.cpp
