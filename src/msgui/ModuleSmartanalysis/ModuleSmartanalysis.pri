include($$PWD/ModuleSmartanalysis-AnprSearch/ModuleSmartanalysis-AnprSearch.pri)
include($$PWD/ModuleSmartAnalysis-AnprSettings/ModuleSmartAnalysis-AnprSettings.pri)
include($$PWD/ModuleSmartAnalysis-HeatMapSearch/ModuleSmartAnalysis-HeatMapSearch.pri)
include($$PWD/ModuleSmartAnalysis-HeatMapSettings/ModuleSmartAnalysis-HeatMapSettings.pri)
include($$PWD/ModuleSmartAnalysis-PeopleCountingSearch/ModuleSmartAnalysis-PeopleCountingSearch.pri)
include($$PWD/ModuleSmartAnalysis-PeopleCountingSettings/ModuleSmartAnalysis-PeopleCountingSettings.pri)
include($$PWD/ModuleSmartAnalysis-PosSearch/ModuleSmartAnalysis-PosSearch.pri)
include($$PWD/ModuleSmartAnalysis-PosSettings/ModuleSmartAnalysis-PosSettings.pri)
include($$PWD/ModuleSmartAnalysis-FaceDetectionSettings/ModuleSmartAnalysis-FaceDetectionSettings.pri)
include($$PWD/ModuleSmartAnalysis-FaceDetectionSearch/ModuleSmartAnalysis-FaceDetectionSearch.pri)

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/ModuleSmartAnalysis.h

SOURCES += \
    $$PWD/ModuleSmartAnalysis.cpp

FORMS += \
    $$PWD/ModuleSmartAnalysis.ui

