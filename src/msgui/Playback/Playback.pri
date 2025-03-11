include($$PWD/PlaybackFileManagement/PlaybackFileManagement.pri)
include($$PWD/PlaybackSmartSearch/PlaybackSmartSearch.pri)

INCLUDEPATH += $$PWD

FORMS += \
    $$PWD/AnimateToast.ui \
    $$PWD/AnimateToastItem.ui \
    $$PWD/CustomTag.ui \
    $$PWD/FilterEventPanel.ui \
    $$PWD/PicturePlay.ui \
    $$PWD/PlaybackBar.ui \
    $$PWD/PlaybackCut.ui \
    $$PWD/PlaybackEvent.ui \
    $$PWD/PlaybackGeneral.ui \
    $$PWD/PlaybackLayout.ui \
    $$PWD/PlaybackLayoutSplit.ui \
    $$PWD/PlaybackList.ui \
    $$PWD/PlaybackMode.ui \
    $$PWD/PlaybackPicture.ui \
    $$PWD/PlaybackSplit.ui \
    $$PWD/PlaybackTag.ui \
    $$PWD/PlaybackVideo.ui \
    $$PWD/PlaybackVideoBar.ui \
    $$PWD/PlaybackWindow.ui \
    $$PWD/PlaybackZoom.ui \
    $$PWD/SmartSpeedDebug.ui \
    $$PWD/SmartSpeedPanel.ui \
    $$PWD/playbackcontrol.ui

HEADERS += \
    $$PWD/AnimateToast.h \
    $$PWD/AnimateToastItem.h \
    $$PWD/BasePlayback.h \
    $$PWD/CloseCommonBackup.h \
    $$PWD/CustomTag.h \
    $$PWD/DateTimeRange.h \
    $$PWD/FilterEventPanel.h \
    $$PWD/PicturePlay.h \
    $$PWD/PlaybackBar.h \
    $$PWD/PlaybackChannelInfo.h \
    $$PWD/PlaybackCut.h \
    $$PWD/PlaybackData.h \
    $$PWD/PlaybackEvent.h \
    $$PWD/PlaybackEventData.h \
    $$PWD/PlaybackEventList.h \
    $$PWD/PlaybackGeneral.h \
    $$PWD/PlaybackLayout.h \
    $$PWD/PlaybackLayoutSplit.h \
    $$PWD/PlaybackList.h \
    $$PWD/PlaybackMode.h \
    $$PWD/PlaybackPicture.h \
    $$PWD/PlaybackPictureList.h \
    $$PWD/PlaybackRealTimeThread.h \
    $$PWD/PlaybackSpeedSlider.h \
    $$PWD/PlaybackSplit.h \
    $$PWD/PlaybackSplitInfo.h \
    $$PWD/PlaybackTag.h \
    $$PWD/PlaybackTagData.h \
    $$PWD/PlaybackTagList.h \
    $$PWD/PlaybackTimeLine.h \
    $$PWD/PlaybackVideo.h \
    $$PWD/PlaybackVideoBar.h \
    $$PWD/PlaybackWindow.h \
    $$PWD/PlaybackZoom.h \
    $$PWD/SearchAbstract.h \
    $$PWD/SearchCommonBackup.h \
    $$PWD/SearchEventBackup.h \
    $$PWD/SmartSearchControl.h \
    $$PWD/SmartSpeedDebug.h \
    $$PWD/SmartSpeedPanel.h \
    $$PWD/StartPlayback.h \
    $$PWD/StopPlayback.h \
    $$PWD/ThumbWidget.h \
    $$PWD/TimeLineThread.h

SOURCES += \
    $$PWD/AnimateToast.cpp \
    $$PWD/AnimateToastItem.cpp \
    $$PWD/BasePlayback.cpp \
    $$PWD/CloseCommonBackup.cpp \
    $$PWD/CustomTag.cpp \
    $$PWD/DateTimeRange.cpp \
    $$PWD/FilterEventPanel.cpp \
    $$PWD/PicturePlay.cpp \
    $$PWD/PlaybackBar.cpp \
    $$PWD/PlaybackChannelInfo.cpp \
    $$PWD/PlaybackCut.cpp \
    $$PWD/PlaybackData.cpp \
    $$PWD/PlaybackEvent.cpp \
    $$PWD/PlaybackEventData.cpp \
    $$PWD/PlaybackEventList.cpp \
    $$PWD/PlaybackGeneral.cpp \
    $$PWD/PlaybackLayout.cpp \
    $$PWD/PlaybackLayoutSplit.cpp \
    $$PWD/PlaybackList.cpp \
    $$PWD/PlaybackMode.cpp \
    $$PWD/PlaybackPicture.cpp \
    $$PWD/PlaybackPictureList.cpp \
    $$PWD/PlaybackRealTimeThread.cpp \
    $$PWD/PlaybackSpeedSlider.cpp \
    $$PWD/PlaybackSplit.cpp \
    $$PWD/PlaybackSplitInfo.cpp \
    $$PWD/PlaybackTag.cpp \
    $$PWD/PlaybackTagData.cpp \
    $$PWD/PlaybackTagList.cpp \
    $$PWD/PlaybackTimeLine.cpp \
    $$PWD/PlaybackVideo.cpp \
    $$PWD/PlaybackVideoBar.cpp \
    $$PWD/PlaybackWindow.cpp \
    $$PWD/PlaybackZoom.cpp \
    $$PWD/SearchAbstract.cpp \
    $$PWD/SearchCommonBackup.cpp \
    $$PWD/SearchEventBackup.cpp \
    $$PWD/SmartSearchControl.cpp \
    $$PWD/SmartSpeedDebug.cpp \
    $$PWD/SmartSpeedPanel.cpp \
    $$PWD/StartPlayback.cpp \
    $$PWD/StopPlayback.cpp \
    $$PWD/ThumbWidget.cpp \
    $$PWD/TimeLineThread.cpp
