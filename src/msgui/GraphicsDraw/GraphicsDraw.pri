INCLUDEPATH += $$PWD

include($$PWD/GraphicsDrawPolygon/GraphicsDrawPolygon.pri)
include($$PWD/GraphicsDrawPos/GraphicsDrawPos.pri)
include($$PWD/GraphicsDrawLineCrossing/GraphicsDrawLineCrossing.pri)
include($$PWD/GraphicsDrawGrid/GraphicsDrawGrid.pri)
include($$PWD/GraphicsDrawFace/GraphicsDrawFace.pri)

HEADERS += \
    $$PWD/DrawItemAnpr.h \
    $$PWD/DrawItemDiskHealth.h \
    $$PWD/DrawItemMask.h \
    $$PWD/DrawItemMaskControl.h \
    $$PWD/DrawItemMotion.h \
    $$PWD/DrawItemObjectBox.h \
    $$PWD/DrawItemObjectSize.h \
    $$PWD/DrawItemPTZMaskControl.h \
    $$PWD/DrawItemRoi.h \
    $$PWD/DrawItemSpaceHeatMap.h \
    $$PWD/DrawItemTimeHeatMap.h \
    $$PWD/DrawItemTrack.h \
    $$PWD/DrawSceneAnpr.h \
    $$PWD/DrawSceneDiskHealth.h \
    $$PWD/DrawSceneHeatMap.h \
    $$PWD/DrawSceneMask.h \
    $$PWD/DrawSceneMotion.h \
    $$PWD/DrawSceneObjectSize.h \
    $$PWD/DrawScenePtzMask.h \
    $$PWD/DrawSceneRoi.h \
    $$PWD/DrawVideoBase.h \
    $$PWD/DrawVideoGrid.h \
    $$PWD/DrawVideoLine.h \
    $$PWD/DrawView.h \
    $$PWD/GraphicsItem.h \
    $$PWD/GraphicsScene.h \
    $$PWD/MsGraphicsObject.h \
    $$PWD/VideoScene.h

SOURCES += \
    $$PWD/DrawItemAnpr.cpp \
    $$PWD/DrawItemDiskHealth.cpp \
    $$PWD/DrawItemMask.cpp \
    $$PWD/DrawItemMaskControl.cpp \
    $$PWD/DrawItemMotion.cpp \
    $$PWD/DrawItemObjectBox.cpp \
    $$PWD/DrawItemObjectSize.cpp \
    $$PWD/DrawItemPTZMaskControl.cpp \
    $$PWD/DrawItemRoi.cpp \
    $$PWD/DrawItemSpaceHeatMap.cpp \
    $$PWD/DrawItemTimeHeatMap.cpp \
    $$PWD/DrawItemTrack.cpp \
    $$PWD/DrawSceneAnpr.cpp \
    $$PWD/DrawSceneDiskHealth.cpp \
    $$PWD/DrawSceneHeatMap.cpp \
    $$PWD/DrawSceneMask.cpp \
    $$PWD/DrawSceneMotion.cpp \
    $$PWD/DrawSceneObjectSize.cpp \
    $$PWD/DrawScenePtzMask.cpp \
    $$PWD/DrawSceneRoi.cpp \
    $$PWD/DrawVideoBase.cpp \
    $$PWD/DrawVideoGrid.cpp \
    $$PWD/DrawVideoLine.cpp \
    $$PWD/DrawView.cpp \
    $$PWD/GraphicsItem.cpp \
    $$PWD/GraphicsScene.cpp \
    $$PWD/MsGraphicsObject.cpp \
    $$PWD/VideoScene.cpp

