INCLUDEPATH += $$PWD

FORMS += \
    $$PWD/EffectiveTimeAbstract.ui

HEADERS += \
    $$PWD/EffectiveTimeAbstract.h \
    $$PWD/EffectiveTimeAnpr.h \
    $$PWD/EffectiveTimeAudioAlarm.h \
    $$PWD/EffectiveTimeCameraAlarmInput.h \
    $$PWD/EffectiveTimeCameraAlarmOutput.h \
    $$PWD/EffectiveTimeLineCrossing.h \
    $$PWD/EffectiveTimeFace.h \
    $$PWD/EffectiveTimeMotionDetection.h \
    $$PWD/EffectiveTimeNvrAlarmInput.h \
    $$PWD/EffectiveTimeNvrAlarmOutput.h \
    $$PWD/EffectiveTimePeopleCounting.h \
    $$PWD/EffectiveTimePos.h \
    $$PWD/EffectiveTimePtzAutoTracking.h \
    $$PWD/EffectiveTimeVCA.h

SOURCES += \
    $$PWD/EffectiveTimeAbstract.cpp \
    $$PWD/EffectiveTimeAnpr.cpp \
    $$PWD/EffectiveTimeAudioAlarm.cpp \
    $$PWD/EffectiveTimeCameraAlarmInput.cpp \
    $$PWD/EffectiveTimeCameraAlarmOutput.cpp \
    $$PWD/EffectiveTimeLineCrossing.cpp \
    $$PWD/EffectiveTimeFace.cpp \
    $$PWD/EffectiveTimeMotionDetection.cpp \
    $$PWD/EffectiveTimeNvrAlarmInput.cpp \
    $$PWD/EffectiveTimeNvrAlarmOutput.cpp \
    $$PWD/EffectiveTimePeopleCounting.cpp \
    $$PWD/EffectiveTimePos.cpp \
    $$PWD/EffectiveTimePtzAutoTracking.cpp \
    $$PWD/EffectiveTimeVCA.cpp

QT += widgets
