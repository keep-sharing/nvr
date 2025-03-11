#include "LiveLayout.h"
#include "MsDevice.h"
#include <QtDebug>
#include <qmath.h>

QMap<int, mosaic> LiveLayout::s_mapMosaicData;
QMap<int, camera> LiveLayout::s_mapCameraData;

LiveLayout::LiveLayout(int layoutType, QObject *parent)
    : QObject(parent)
    , m_layoutType(layoutType)
{
    //qDebug() << QString("LiveLayout::LiveLayout, type: %1").arg(m_layoutType);
}

void LiveLayout::initializeMosaicData()
{
    s_mapMosaicData.clear();
    //
    mosaic mosaicArray[LAYOUT_MODE_MAX];
    memset(mosaicArray, 0, sizeof(mosaic) * LAYOUT_MODE_MAX);
    int mosaicCount = 0;
    read_mosaics(SQLITE_FILE_NAME, mosaicArray, &mosaicCount);
    for (int i = 0; i < mosaicCount; ++i) {
        mosaic &m = mosaicArray[i];
        s_mapMosaicData.insert(m.layoutmode, m);
    }
}

QMap<int, mosaic> LiveLayout::mosaicMap()
{
    return s_mapMosaicData;
}

bool LiveLayout::isLayoutEnable(int layout, int channel)
{
    if (s_mapMosaicData.contains(layout)) {
        const mosaic &m = s_mapMosaicData.value(layout);
        if (m.channels[channel] >= 0) {
            return true;
        }
    }
    return false;
}

void LiveLayout::initializeCameraData()
{
    s_mapCameraData.clear();
    //
    camera cameraArray[MAX_CAMERA];
    memset(cameraArray, 0, sizeof(camera) * MAX_CAMERA);
    int cameraCount = 0;
    read_cameras(SQLITE_FILE_NAME, cameraArray, &cameraCount);
    for (int i = 0; i < cameraCount; ++i) {
        const camera &cam = cameraArray[i];
        s_mapCameraData.insert(cam.id, cam);
    }
}

camera LiveLayout::cameraData(int channel)
{
    if (s_mapCameraData.contains(channel)) {
        return s_mapCameraData.value(channel);
    } else {
        camera cam;
        memset(&cam, 0, sizeof(camera));
        cam.id = -1;
        return cam;
    }
}

void LiveLayout::initializeLayout()
{
    initializeCameraData();
    //
    const mosaic &mosaic_data = s_mapMosaicData.value(m_layoutType);
    //
    switch (m_layoutType) {
    case LAYOUTMODE_1:
        setNormalLayout(1, 1, mosaic_data);
        break;
    case LAYOUTMODE_4:
        setNormalLayout(2, 2, mosaic_data);
        break;
    case LAYOUTMODE_8:
        setLayout_8(mosaic_data);
        break;
    case LAYOUTMODE_8_1:
        setLayout_8_1(mosaic_data);
        break;
    case LAYOUTMODE_9:
        setNormalLayout(3, 3, mosaic_data);
        break;
    case LAYOUTMODE_12:
        setNormalLayout(3, 4, mosaic_data);
        break;
    case LAYOUTMODE_12_1:
        setLayout_12_1(mosaic_data);
        break;
    case LAYOUTMODE_14:
        setLayout_14(mosaic_data);
        break;
    case LAYOUTMODE_16:
        setNormalLayout(4, 4, mosaic_data);
        break;
    case LAYOUTMODE_25:
        setNormalLayout(5, 5, mosaic_data);
        break;
    case LAYOUTMODE_32:
        setNormalLayout(4, 8, mosaic_data);
        break;
    case LAYOUTMODE_32_2:
        setLayout_32_2(mosaic_data);
        break;
    case LAYOUTMODE_36:
        setNormalLayout(6, 6, mosaic_data);
        break;
    }
}

VideoPositionList LiveLayout::channelPosition(int page) const
{
    if (!m_validPageMap.contains(page)) {
        qWarning() << QString("LiveLayout::channelPosition, %1 doesn't has page %2.").arg(layoutTypeString()).arg(page);
        return VideoPositionList();
    }
    return m_validPageMap.value(page);
}

QMap<int, VideoPositionList> LiveLayout::allPageMap() const
{
    return m_allPageMap;
}

int LiveLayout::pageCount() const
{
    return m_validPageMap.size();
}

/**
 * @brief LiveLayout::nextPage
 * @param currentPage
 * @return 0-n
 */
int LiveLayout::nextPage(int currentPage) const
{
    auto iter = m_validPageMap.constFind(currentPage);
    if (iter == m_validPageMap.constEnd()) {
        iter = m_validPageMap.constBegin();
    } else {
        iter++;
        if (iter == m_validPageMap.constEnd()) {
            iter = m_validPageMap.constBegin();
        }
    }
    return iter.key();
}

/**
 * @brief LiveLayout::previousPage
 * @param currentPage
 * @return 1-n
 */
int LiveLayout::previousPage(int currentPage) const
{
    auto iter = m_validPageMap.constFind(currentPage);
    if (iter == m_validPageMap.constBegin()) {
        iter = m_validPageMap.constEnd();
    }
    iter--;
    return iter.key();
}

void LiveLayout::swapChannel(int page, int index1, int channel1, int index2, int channel2)
{
    qDebug() << QString("LiveLayout::swapChannel, page: %1, channel1: %2, channel2: %3").arg(page).arg(channel1).arg(channel2);
    if (m_validPageMap.contains(page)) {
        VideoPositionList &list = m_validPageMap[page];

        int pageIndex1 = -1;
        int pageIndex2 = -1;
        for (int i = 0; i < list.size(); ++i) {
            if (list.at(i).channel == channel1) {
                pageIndex1 = i;
            }
            if (list.at(i).channel == channel2) {
                pageIndex2 = i;
            }
        }
        if (pageIndex1 != -1 && pageIndex2 != -1) {
            VideoPosition &position1 = list[pageIndex1];
            VideoPosition &position2 = list[pageIndex2];
            int tempChannel = position1.channel;
            position1.channel = position2.channel;
            position2.channel = tempChannel;
        }
        //
        mosaic &mosaic_data = s_mapMosaicData[m_layoutType];
        mosaic_data.channels[index1] = channel2;
        mosaic_data.channels[index2] = channel1;
    }
}

/**
 * @brief LiveLayout::findVideoIndex
 * @param page
 * @param index
 * @param direction
 * @return -1: 下一页，-2：上一页
 */
int LiveLayout::findVideoIndex(int page, int index, RockerDirection direction)
{
    if (direction < RockerUp || direction > RockerDownRight) {
        return -3;
    }

    qDebug() << "====LiveLayout::findVideoIndex====";
    qDebug() << "----page:" << page;
    qDebug() << "----index:" << index;
    qDebug() << "----direction:" << direction;

    const VideoPositionList &positionList = m_validPageMap.value(page);
    if (index < 0) {
        return positionList.first().index;
    }
    VideoPosition position;
    position.index = index;
    int result = -1;
    switch (m_layoutType) {
    case LAYOUTMODE_1:
        switch (direction) {
        case RockerUp:
        case RockerLeft:
            position.index = index - 1;
            break;
        case RockerDown:
        case RockerRight:
            position.index = index + 1;
            break;
        default:
            break;
        }
        break;
    case LAYOUTMODE_4: {
        int indexInPage = index % 4;
        switch (direction) {
        case RockerUp:
            switch (indexInPage) {
            case 0:
            case 1:
                position.index = index - 4;
                break;
            case 2:
            case 3:
                position.index = index - 2;
                break;
            default:
                break;
            }
            break;
        case RockerDown:
            switch (indexInPage) {
            case 0:
            case 1:
                position.index = index + 2;
                break;
            case 2:
            case 3:
                position.index = index + 4;
                break;
            default:
                break;
            }
            break;
        case RockerLeft:
            switch (indexInPage) {
            case 0:
            case 2:
                position.index = index - 4;
                break;
            case 1:
            case 3:
                position.index = index - 1;
                break;
            default:
                break;
            }
            break;
        case RockerRight:
            switch (indexInPage) {
            case 0:
            case 2:
                position.index = index + 1;
                break;
            case 1:
            case 3:
                position.index = index + 4;
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
        break;
    }
    case LAYOUTMODE_8: {
        int indexInPage = index % 8;
        switch (direction) {
        case RockerUp:
            switch (indexInPage) {
            case 0:
            case 1:
            case 4:
                position.index = index - 8;
                break;
            case 2:
            case 3:
                position.index = index - 2;
                break;
            case 5:
            case 6:
            case 7:
                position.index = index - 1;
                break;
            default:
                break;
            }
            break;
        case RockerDown:
            switch (indexInPage) {
            case 0:
            case 1:
                position.index = index + 2;
                break;
            case 4:
            case 5:
            case 6:
                position.index = index + 1;
                break;
            case 2:
            case 3:
            case 7:
                position.index = index + 8;
                break;
            default:
                break;
            }
            break;
        case RockerLeft:
            switch (indexInPage) {
            case 0:
            case 2:
                position.index = index - 8;
                break;
            case 1:
            case 3:
                position.index = index - 1;
                break;
            case 4:
            case 6:
                position.index = index - 3;
                break;
            case 5:
            case 7:
                position.index = index - 4;
                break;
            default:
                break;
            }
            break;
        case RockerRight:
            switch (indexInPage) {
            case 0:
            case 2:
                position.index = index + 1;
                break;
            case 1:
            case 3:
                position.index = index + 3;
                break;
            case 4:
            case 6:
                position.index = index + 4;
                break;
            case 5:
            case 7:
                position.index = index + 3;
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
        break;
    }
    case LAYOUTMODE_8_1: {
        int indexInPage = index % 8;
        switch (direction) {
        case RockerUp:
            switch (indexInPage) {
            case 0:
            case 1:
                position.index = index - 8;
                break;
            case 2:
            case 3:
                position.index = index - 1;
                break;
            case 4:
            case 5:
            case 6:
                position.index = page * 8;
                break;
            case 7:
                position.index = index - 4;
                break;
            default:
                break;
            }
            break;
        case RockerDown:
            switch (indexInPage) {
            case 0:
                position.index = index + 4;
                break;
            case 1:
            case 2:
                position.index = index + 1;
                break;
            case 3:
                position.index = index + 4;
                break;
            case 4:
            case 5:
            case 6:
            case 7:
                position.index = index + 8;
                break;
            default:
                break;
            }
            break;
        case RockerLeft:
            switch (indexInPage) {
            case 0:
            case 4:
                position.index = index - 8;
                break;
            case 1:
            case 2:
            case 3:
                position.index = page * 8;
                break;
            case 5:
            case 6:
            case 7:
                position.index = index - 1;
                break;
            default:
                break;
            }
            break;
        case RockerRight:
            switch (indexInPage) {
            case 0:
                position.index = index + 1;
                break;
            case 1:
            case 2:
            case 3:
            case 7:
                position.index = index + 8;
                break;
            case 4:
            case 5:
            case 6:
                position.index = index + 1;
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
        break;
    }
    case LAYOUTMODE_9: {
        int indexInPage = index % 9;
        switch (direction) {
        case RockerUp:
            switch (indexInPage) {
            case 0:
            case 1:
            case 2:
                position.index = index - 9;
                break;
            default:
                position.index = index - 3;
                break;
            }
            break;
        case RockerDown:
            switch (indexInPage) {
            case 6:
            case 7:
            case 8:
                position.index = index + 9;
                break;
            default:
                position.index = index + 3;
                break;
            }
            break;
        case RockerLeft:
            switch (indexInPage) {
            case 0:
            case 3:
            case 6:
                position.index = index - 9;
                break;
            default:
                position.index = index - 1;
                break;
            }
            break;
        case RockerRight:
            switch (indexInPage) {
            case 2:
            case 5:
            case 8:
                position.index = index + 9;
                break;
            default:
                position.index = index + 1;
                break;
            }
            break;
        default:
            break;
        }
        break;
    }
    case LAYOUTMODE_12: {
        int indexInPage = index % 12;
        switch (direction) {
        case RockerUp:
            switch (indexInPage) {
            case 0:
            case 1:
            case 2:
            case 3:
                position.index = index - 12;
                break;
            default:
                position.index = index - 4;
                break;
            }
            break;
        case RockerDown:
            switch (indexInPage) {
            case 8:
            case 9:
            case 10:
            case 11:
                position.index = index + 12;
                break;
            default:
                position.index = index + 4;
                break;
            }
            break;
        case RockerLeft:
            switch (indexInPage) {
            case 0:
            case 4:
            case 8:
                position.index = index - 12;
                break;
            default:
                position.index = index - 1;
                break;
            }
            break;
        case RockerRight:
            switch (indexInPage) {
            case 3:
            case 7:
            case 11:
                position.index = index + 12;
                break;
            default:
                position.index = index + 1;
                break;
            }
            break;
        default:
            break;
        }
        break;
    }
    case LAYOUTMODE_12_1: {
        int indexInPage = index % 12;
        switch (direction) {
        case RockerUp:
            switch (indexInPage) {
            case 0:
            case 1:
            case 2:
                position.index = index - 12;
                break;
            case 3:
            case 4:
            case 5:
            case 6:
                position.index = index - 2;
                break;
            case 7:
            case 8:
            case 9:
                position.index = page * 12;
                break;
            case 10:
            case 11:
                position.index = index - 5;
                break;
            default:
                break;
            }
            break;
        case RockerDown:
            switch (indexInPage) {
            case 0:
                position.index = index + 7;
                break;
            case 1:
            case 2:
            case 3:
            case 4:
                position.index = index + 2;
                break;
            case 5:
            case 6:
                position.index = index + 5;
                break;
            default:
                position.index = index + 12;
                break;
            }
            break;
        case RockerLeft:
            switch (indexInPage) {
            case 0:
            case 7:
                position.index = index - 12;
                break;
            case 1:
            case 3:
            case 5:
                position.index = page * 12;
                break;
            default:
                position.index = index - 1;
                break;
            }
            break;
        case RockerRight:
            switch (indexInPage) {
            case 2:
            case 4:
            case 6:
            case 11:
                position.index = index + 12;
                break;
            default:
                position.index = index + 1;
                break;
            }
            break;
        default:
            break;
        }
        break;
    }
    case LAYOUTMODE_14: {
        int indexInPage = index % 14;
        switch (direction) {
        case RockerUp:
            switch (indexInPage) {
            case 0:
            case 1:
            case 10:
                position.index = index - 14;
                break;
            case 2:
            case 3:
                position.index = page * 14;
                break;
            case 4:
            case 5:
                position.index = page * 14 + 1;
                break;
            case 6:
            case 7:
            case 8:
            case 9:
                position.index = index - 4;
                break;
            default:
                position.index = index - 1;
                break;
            }
            break;
        case RockerDown:
            switch (indexInPage) {
            case 0:
                position.index = index + 2;
                break;
            case 1:
                position.index = index + 3;
                break;
            case 2:
            case 3:
            case 4:
            case 5:
                position.index = index + 4;
                break;
            case 10:
            case 11:
            case 12:
                position.index = index + 1;
                break;
            default:
                position.index = index + 14;
                break;
            }
            break;
        case RockerLeft:
            switch (indexInPage) {
            case 0:
            case 2:
            case 6:
                position.index = index - 14;
                break;
            case 1:
            case 3:
            case 4:
            case 5:
            case 7:
            case 8:
            case 9:
                position.index = index - 1;
                break;
            case 10:
            case 11:
                position.index = page * 14 + 1;
                break;
            case 12:
                position.index = index - 7;
                break;
            case 13:
                position.index = index - 4;
                break;
            default:
                break;
            }
            break;
        case RockerRight:
            switch (indexInPage) {
            case 1:
                position.index = index + 9;
                break;
            case 5:
                position.index = index + 7;
                break;
            case 9:
                position.index = index + 4;
                break;
            case 10:
            case 11:
            case 12:
            case 13:
                position.index = index + 14;
                break;
            default:
                position.index = index + 1;
                break;
            }
            break;
        default:
            break;
        }
        break;
    }
    case LAYOUTMODE_16: {
        int indexInPage = index % 16;
        switch (direction) {
        case RockerUp:
            switch (indexInPage) {
            case 0:
            case 1:
            case 2:
            case 3:
                position.index = index - 16;
                break;
            default:
                position.index = index - 4;
                break;
            }
            break;
        case RockerDown:
            switch (indexInPage) {
            case 12:
            case 13:
            case 14:
            case 15:
                position.index = index + 16;
                break;
            default:
                position.index = index + 4;
                break;
            }
            break;
        case RockerLeft:
            switch (indexInPage) {
            case 0:
            case 4:
            case 8:
            case 12:
                position.index = index - 16;
                break;
            default:
                position.index = index - 1;
                break;
            }
            break;
        case RockerRight:
            switch (indexInPage) {
            case 3:
            case 7:
            case 11:
            case 15:
                position.index = index + 16;
                break;
            default:
                position.index = index + 1;
                break;
            }
            break;
        default:
            break;
        }
        break;
    }
    case LAYOUTMODE_25: {
        int indexInPage = index % 25;
        switch (direction) {
        case RockerUp:
            switch (indexInPage) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
                position.index = index - 25;
                break;
            default:
                position.index = index - 5;
                break;
            }
            break;
        case RockerDown:
            switch (indexInPage) {
            case 20:
            case 21:
            case 22:
            case 23:
            case 24:
                position.index = index + 25;
                break;
            default:
                position.index = index + 5;
                break;
            }
            break;
        case RockerLeft:
            switch (indexInPage) {
            case 0:
            case 5:
            case 10:
            case 15:
            case 20:
                position.index = index - 25;
                break;
            default:
                position.index = index - 1;
                break;
            }
            break;
        case RockerRight:
            switch (indexInPage) {
            case 4:
            case 9:
            case 14:
            case 19:
            case 24:
                position.index = index + 25;
                break;
            default:
                position.index = index + 1;
                break;
            }
            break;
        default:
            break;
        }
        break;
    }
    case LAYOUTMODE_32: {
        int indexInPage = index % 32;
        switch (direction) {
        case RockerUp:
            switch (indexInPage) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                position.index = index - 32;
                break;
            default:
                position.index = index - 8;
                break;
            }
            break;
        case RockerDown:
            switch (indexInPage) {
            case 24:
            case 25:
            case 26:
            case 27:
            case 28:
            case 29:
            case 30:
            case 31:
                position.index = index + 32;
                break;
            default:
                position.index = index + 8;
                break;
            }
            break;
        case RockerLeft:
            switch (indexInPage) {
            case 0:
            case 8:
            case 16:
            case 24:
                position.index = index - 32;
                break;
            default:
                position.index = index - 1;
                break;
            }
            break;
        case RockerRight:
            switch (indexInPage) {
            case 7:
            case 15:
            case 23:
            case 31:
                position.index = index + 32;
                break;
            default:
                position.index = index + 1;
                break;
            }
            break;
        default:
            break;
        }
        break;
    }
    case LAYOUTMODE_32_2: {
        int indexInPage = index % 32;
        switch (direction) {
        case RockerUp:
            switch (indexInPage) {
            case 0:
            case 1:
            case 2:
                position.index = index - 32;
                break;
            case 3:
            case 4:
                position.index = page * 32 + 1;
                break;
            case 5:
            case 6:
                position.index = page * 32 + 2;
                break;
            case 7:
            case 8:
                position.index = page * 32;
                break;
            case 9:
            case 10:
            case 11:
            case 12:
                position.index = index - 6;
                break;
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
                position.index = index - 5;
                break;
            case 18:
            case 19:
                position.index = page * 32 + 7;
                break;
            default:
                position.index = index - 7;
                break;
            }
            break;
        case RockerDown:
            switch (indexInPage) {
            case 0:
                position.index = index + 7;
                break;
            case 1:
                position.index = index + 2;
                break;
            case 2:
                position.index = index + 3;
                break;
            case 3:
            case 4:
            case 5:
            case 6:
                position.index = index + 6;
                break;
            case 7:
                position.index = index + 11;
                break;
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
                position.index = index + 5;
                break;
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
                position.index = index + 7;
                break;
            case 18:
            case 19:
            case 20:
            case 21:
            case 22:
            case 23:
            case 24:
                position.index = index + 7;
                break;
            default:
                position.index = index + 32;
                break;
            }
            break;
        case RockerLeft:
            switch (indexInPage) {
            case 0:
            case 7:
            case 18:
            case 25:
                position.index = index - 32;
                break;
            case 3:
                position.index = page * 32;
                break;
            case 13:
                position.index = page * 32 + 7;
                break;
            default:
                position.index = index - 1;
                break;
            }
            break;
        case RockerRight:
            switch (indexInPage) {
            case 2:
            case 6:
            case 12:
            case 17:
            case 24:
            case 31:
                position.index = index + 32;
                break;
            default:
                position.index = index + 1;
                break;
            }
            break;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
    if (positionList.contains(position)) {
        result = position.index;
    } else {
        switch (direction) {
        case RockerUp:
        case RockerLeft:
            result = -2;
            break;
        case RockerDown:
        case RockerRight:
            result = -1;
        default:
            break;
        }
    }
    qDebug() << QString("LiveLayout::findVideoIndex, page: %1, index: %2, result: %3").arg(page).arg(index).arg(result);
    return result;
}

int LiveLayout::pageFromChannel(int channel)
{
    for (auto iter = m_validPageMap.constBegin(); iter != m_validPageMap.constEnd(); ++iter) {
        const int page = iter.key();
        const VideoPositionList &list = iter.value();
        for (int i = 0; i < list.size(); ++i) {
            const VideoPosition &position = list.at(i);
            if (position.channel == channel) {
                return page;
            }
        }
    }
    return -1;
}

void LiveLayout::updateChannelIndexMap(QMap<int, int> &map)
{
    map.clear();

    for (auto iter = m_allPageMap.constBegin(); iter != m_allPageMap.constEnd(); ++iter) {
        const VideoPositionList &list = iter.value();
        for (int i = 0; i < list.size(); ++i) {
            const VideoPosition &position = list.at(i);
            map.insert(position.channel, position.index);
        }
    }
}

bool LiveLayout::isChannelEnable(int channel)
{
    if (s_mapCameraData.contains(channel)) {
        const camera &cam = s_mapCameraData.value(channel);
        return cam.enable;
    } else {
        return false;
    }
}

QString LiveLayout::layoutTypeString(int layout)
{
    QString strType;
    switch (layout) {
    case LAYOUTMODE_1:
        strType = "LAYOUTMODE_1";
        break;
    case LAYOUTMODE_4:
        strType = "LAYOUTMODE_4";
        break;
    case LAYOUTMODE_8:
        strType = "LAYOUTMODE_8";
        break;
    case LAYOUTMODE_8_1:
        strType = "LAYOUTMODE_8_1";
        break;
    case LAYOUTMODE_9:
        strType = "LAYOUTMODE_9";
        break;
    case LAYOUTMODE_12:
        strType = "LAYOUTMODE_12";
        break;
    case LAYOUTMODE_12_1:
        strType = "LAYOUTMODE_12_1";
        break;
    case LAYOUTMODE_14:
        strType = "LAYOUTMODE_14";
        break;
    case LAYOUTMODE_16:
        strType = "LAYOUTMODE_16";
        break;
    case LAYOUTMODE_25:
        strType = "LAYOUTMODE_25";
        break;
    case LAYOUTMODE_32:
        strType = "LAYOUTMODE_32";
        break;
    case LAYOUTMODE_36:
        strType = "LAYOUTMODE_36";
        break;
    default:
        strType = QString("Unknow type (%1)").arg(layout);
        break;
    }
    return strType;
}

QString LiveLayout::layoutTypeString() const
{
    QString strType;
    switch (m_layoutType) {
    case LAYOUTMODE_1:
        strType = "LAYOUTMODE_1";
        break;
    case LAYOUTMODE_4:
        strType = "LAYOUTMODE_4";
        break;
    case LAYOUTMODE_8:
        strType = "LAYOUTMODE_8";
        break;
    case LAYOUTMODE_8_1:
        strType = "LAYOUTMODE_8_1";
        break;
    case LAYOUTMODE_9:
        strType = "LAYOUTMODE_9";
        break;
    case LAYOUTMODE_12:
        strType = "LAYOUTMODE_12";
        break;
    case LAYOUTMODE_12_1:
        strType = "LAYOUTMODE_12_1";
        break;
    case LAYOUTMODE_14:
        strType = "LAYOUTMODE_14";
        break;
    case LAYOUTMODE_16:
        strType = "LAYOUTMODE_16";
        break;
    case LAYOUTMODE_25:
        strType = "LAYOUTMODE_25";
        break;
    case LAYOUTMODE_32:
        strType = "LAYOUTMODE_32";
        break;
    case LAYOUTMODE_36:
        strType = "LAYOUTMODE_36";
        break;
    default:
        strType = QString("Unknow type (%1)").arg(m_layoutType);
        break;
    }
    return strType;
}

void LiveLayout::setNormalLayout(int row, int column, const mosaic &layoutData)
{
    m_allPageMap.clear();
    m_validPageMap.clear();

    //
    int maxDevice = qMsNvr->maxChannel();
    int pageCount = qCeil((qreal)maxDevice / (row * column));
    int videoIndex = 0;
    for (int i = 0; i < pageCount; ++i) {
        VideoPositionList positionList;

        bool validPage = false;
        for (int r = 0; r < row; ++r) {
            for (int c = 0; c < column; ++c) {
                VideoPosition position(r, c);
                position.index = videoIndex;
                if (videoIndex < maxDevice) {
                    position.channel = layoutData.channels[videoIndex];
                    if (isChannelEnable(position.channel)) {
                        validPage = true;
                    }
                } else {
                    position.channel = -1;
                }
                positionList.append(position);
                videoIndex++;
            }
        }
        m_allPageMap.insert(i, positionList);
        if (validPage) {
            int page = m_validPageMap.size();
            m_validPageMap.insert(page, positionList);
        }
    }
    //
    if (m_validPageMap.isEmpty()) {
        m_validPageMap = m_allPageMap;
    }
}

void LiveLayout::setLayout_8(const mosaic &layoutData)
{
    VideoPositionList positionList;
    positionList << VideoPosition(0, 0, 2, 2);
    positionList << VideoPosition(0, 2, 2, 2);
    positionList << VideoPosition(2, 0, 2, 2);
    positionList << VideoPosition(2, 2, 2, 2);
    positionList << VideoPosition(0, 4);
    positionList << VideoPosition(1, 4);
    positionList << VideoPosition(2, 4);
    positionList << VideoPosition(3, 4);
    //
    setSpecificLayout(positionList, layoutData);
}

void LiveLayout::setLayout_8_1(const mosaic &layoutData)
{
    VideoPositionList positionList;
    positionList << VideoPosition(0, 0, 3, 3);
    positionList << VideoPosition(0, 3);
    positionList << VideoPosition(1, 3);
    positionList << VideoPosition(2, 3);
    positionList << VideoPosition(3, 0);
    positionList << VideoPosition(3, 1);
    positionList << VideoPosition(3, 2);
    positionList << VideoPosition(3, 3);
    //
    setSpecificLayout(positionList, layoutData);
}

void LiveLayout::setLayout_12_1(const mosaic &layoutData)
{
    VideoPositionList positionList;
    positionList << VideoPosition(0, 0, 3, 3);
    positionList << VideoPosition(0, 3);
    positionList << VideoPosition(0, 4);
    positionList << VideoPosition(1, 3);
    positionList << VideoPosition(1, 4);
    positionList << VideoPosition(2, 3);
    positionList << VideoPosition(2, 4);
    positionList << VideoPosition(3, 0);
    positionList << VideoPosition(3, 1);
    positionList << VideoPosition(3, 2);
    positionList << VideoPosition(3, 3);
    positionList << VideoPosition(3, 4);
    //
    setSpecificLayout(positionList, layoutData);
}

void LiveLayout::setLayout_14(const mosaic &layoutData)
{
    VideoPositionList positionList;
    positionList << VideoPosition(0, 0, 2, 2);
    positionList << VideoPosition(0, 2, 2, 2);
    positionList << VideoPosition(2, 0);
    positionList << VideoPosition(2, 1);
    positionList << VideoPosition(2, 2);
    positionList << VideoPosition(2, 3);
    positionList << VideoPosition(3, 0);
    positionList << VideoPosition(3, 1);
    positionList << VideoPosition(3, 2);
    positionList << VideoPosition(3, 3);
    positionList << VideoPosition(0, 4);
    positionList << VideoPosition(1, 4);
    positionList << VideoPosition(2, 4);
    positionList << VideoPosition(3, 4);
    //
    setSpecificLayout(positionList, layoutData);
}

void LiveLayout::setLayout_32_2(const mosaic &layoutData)
{
    VideoPositionList positionList;
    positionList << VideoPosition(0, 0, 3, 3);
    positionList << VideoPosition(0, 3, 2, 2);
    positionList << VideoPosition(0, 5, 2, 2);
    positionList << VideoPosition(2, 3);
    positionList << VideoPosition(2, 4);
    positionList << VideoPosition(2, 5);
    positionList << VideoPosition(2, 6);
    positionList << VideoPosition(3, 0, 2, 2);
    positionList << VideoPosition(3, 2);
    positionList << VideoPosition(3, 3);
    positionList << VideoPosition(3, 4);
    positionList << VideoPosition(3, 5);
    positionList << VideoPosition(3, 6);
    positionList << VideoPosition(4, 2);
    positionList << VideoPosition(4, 3);
    positionList << VideoPosition(4, 4);
    positionList << VideoPosition(4, 5);
    positionList << VideoPosition(4, 6);
    positionList << VideoPosition(5, 0);
    positionList << VideoPosition(5, 1);
    positionList << VideoPosition(5, 2);
    positionList << VideoPosition(5, 3);
    positionList << VideoPosition(5, 4);
    positionList << VideoPosition(5, 5);
    positionList << VideoPosition(5, 6);
    positionList << VideoPosition(6, 0);
    positionList << VideoPosition(6, 1);
    positionList << VideoPosition(6, 2);
    positionList << VideoPosition(6, 3);
    positionList << VideoPosition(6, 4);
    positionList << VideoPosition(6, 5);
    positionList << VideoPosition(6, 6);
    //
    setSpecificLayout(positionList, layoutData);
}

void LiveLayout::setSpecificLayout(const VideoPositionList &list, const mosaic &layoutData)
{
    m_allPageMap.clear();
    m_validPageMap.clear();

    int maxDevice = qMsNvr->maxChannel();
    int pageCount = qCeil((qreal)maxDevice / list.size());
    int videoIndex = 0;
    for (int i = 0; i < pageCount; ++i) {
        bool validPage = false;
        VideoPositionList positionList = list;
        for (int i = 0; i < positionList.size(); ++i) {
            VideoPosition &position = positionList[i];
            position.index = videoIndex;
            if (videoIndex < maxDevice) {
                position.channel = layoutData.channels[videoIndex];
                if (isChannelEnable(position.channel)) {
                    validPage = true;
                }
            } else {
                position.channel = -1;
            }
            videoIndex++;
        }
        m_allPageMap.insert(i, positionList);
        if (validPage) {
            int page = m_validPageMap.size();
            m_validPageMap.insert(page, positionList);
        }
    }
    //
    if (m_validPageMap.isEmpty()) {
        m_validPageMap = m_allPageMap;
    }
    qDebug() << QString("LiveLayout::setSpecificLayout, layout: %1, allPageCount: %2, validPageCount: %3").arg(layoutData.layoutmode).arg(m_allPageMap.count()).arg(m_validPageMap.count());
}
