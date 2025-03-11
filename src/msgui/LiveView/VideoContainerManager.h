#ifndef VIDEOCONTAINERMANAGER_H
#define VIDEOCONTAINERMANAGER_H

#include "MsWidget.h"

class CustomLayoutKey;
class CustomLayoutInfo;
class VideoContainer;
class VideoPosition;
class LiveVideo;
class QGridLayout;

class VideoContainerManager : public MsWidget {
    Q_OBJECT

public:
    enum LogicScreen {
        LogicMain,
        LogicSub
    };

    explicit VideoContainerManager(QWidget *parent = nullptr);

    QList<VideoContainer *> containers() const;

    void setLogicScreen(VideoContainerManager::LogicScreen screen);
    int physicalScreen();

    void setLayoutMode(const CustomLayoutInfo &info, int page);

    void swapChannel(int index1, int channel1, int index2, int channel2);

    LiveVideo *videoFromIndex(int index);

    void hideAllLayoutButton();

private:
    void dealDesktopSize();
    QRect nvrVideoGeometry(int enView, const VideoPosition &p);

    LiveVideo *liveVideo(int channel);

signals:

private:
    LogicScreen m_logicScreen = LogicMain;

    QList<VideoContainer *> m_containers;
    QGridLayout *m_layout = nullptr;
    int m_rowCount = 0;
    int m_columnCount = 0;
    int m_dw = 0;
    int m_dh = 0;

    QSize m_nvrScreenSize;
    QSize m_qtScreenSize;
};

#endif // VIDEOCONTAINERMANAGER_H
