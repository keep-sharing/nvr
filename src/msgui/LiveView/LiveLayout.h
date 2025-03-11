#ifndef LIVELAYOUT_H
#define LIVELAYOUT_H

#include "BaseWidget.h"
#include "VideoPosition.h"

#include <QMap>
#include <QObject>

extern "C" {
#include "msdb.h"
}

typedef QList<VideoPosition> VideoPositionList;
typedef QMap<int, VideoPosition> ChannelPositionMap;
typedef QMap<int, VideoPosition>::const_iterator ChannelPositionMapConstIter;

class LiveLayout : public QObject {
    Q_OBJECT
public:
    explicit LiveLayout(int layoutType, QObject *parent);

    static void initializeMosaicData();
    static QMap<int, mosaic> mosaicMap();
    static bool isLayoutEnable(int layout, int channel);

    static void initializeCameraData();
    static camera cameraData(int channel);
    static bool isChannelEnable(int channel);

    static QString layoutTypeString(int layout);
    QString layoutTypeString() const;

    void initializeLayout();
    VideoPositionList channelPosition(int page) const;
    QMap<int, VideoPositionList> allPageMap() const;

    int pageCount() const;
    int nextPage(int currentPage) const;
    int previousPage(int currentPage) const;

    void swapChannel(int page, int index1, int channel1, int index2, int channel2);

    int findVideoIndex(int page, int index, RockerDirection direction);

    int pageFromChannel(int channel);

    void updateChannelIndexMap(QMap<int, int> &map);

private:
    void setNormalLayout(int row, int column, const mosaic &layoutData);
    void setLayout_8(const mosaic &layoutData);
    void setLayout_8_1(const mosaic &layoutData);
    void setLayout_12_1(const mosaic &layoutData);
    void setLayout_14(const mosaic &layoutData);
    void setLayout_32_2(const mosaic &layoutData);
    void setSpecificLayout(const VideoPositionList &list, const mosaic &layoutData);

signals:

public slots:

private:
    static QMap<int, mosaic> s_mapMosaicData;
    static QMap<int, camera> s_mapCameraData;

    int m_layoutType;

    QMap<int, VideoPositionList> m_allPageMap;
    QMap<int, VideoPositionList> m_validPageMap; //key: page
};

#endif // LIVELAYOUT_H
