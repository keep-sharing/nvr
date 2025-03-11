#include "MyLayout.h"
#include "MsDevice.h"
#include <qmath.h>

extern "C" {
#include "msdb.h"
}

MyLayout::MyLayout(QObject *parent)
    : QObject(parent)
{
}

QMap<int, ChannelPositionList> MyLayout::layoutPageMap(const mosaic &layout)
{
    QMap<int, ChannelPositionList> map;

    switch (layout.layoutmode) {
    case LAYOUTMODE_1: {
        map = makeNormalLayout(1, 1, layout);
        break;
    }
    case LAYOUTMODE_4: {
        map = makeNormalLayout(2, 2, layout);
        break;
    }
    case LAYOUTMODE_8: {
        ChannelPositionList list;
        list << ChannelPosition(0, 0, 2, 2);
        list << ChannelPosition(0, 2, 2, 2);
        list << ChannelPosition(2, 0, 2, 2);
        list << ChannelPosition(2, 2, 2, 2);
        list << ChannelPosition(0, 4);
        list << ChannelPosition(1, 4);
        list << ChannelPosition(2, 4);
        list << ChannelPosition(3, 4);
        map = makeSpecificLayout(list, layout);
        break;
    }
    case LAYOUTMODE_8_1: {
        ChannelPositionList list;
        list << ChannelPosition(0, 0, 3, 3);
        list << ChannelPosition(0, 3);
        list << ChannelPosition(1, 3);
        list << ChannelPosition(2, 3);
        list << ChannelPosition(3, 0);
        list << ChannelPosition(3, 1);
        list << ChannelPosition(3, 2);
        list << ChannelPosition(3, 3);
        map = makeSpecificLayout(list, layout);
        break;
    }
    case LAYOUTMODE_9: {
        map = makeNormalLayout(3, 3, layout);
        break;
    }
    case LAYOUTMODE_12: {
        map = makeNormalLayout(3, 4, layout);
        break;
    }
    case LAYOUTMODE_12_1: {
        ChannelPositionList list;
        list << ChannelPosition(0, 0, 3, 3);
        list << ChannelPosition(0, 3);
        list << ChannelPosition(0, 4);
        list << ChannelPosition(1, 3);
        list << ChannelPosition(1, 4);
        list << ChannelPosition(2, 3);
        list << ChannelPosition(2, 4);
        list << ChannelPosition(3, 0);
        list << ChannelPosition(3, 1);
        list << ChannelPosition(3, 2);
        list << ChannelPosition(3, 3);
        list << ChannelPosition(3, 4);
        map = makeSpecificLayout(list, layout);
        break;
    }
    case LAYOUTMODE_14: {
        ChannelPositionList list;
        list << ChannelPosition(0, 0, 2, 2);
        list << ChannelPosition(0, 2, 2, 2);
        list << ChannelPosition(2, 0);
        list << ChannelPosition(2, 1);
        list << ChannelPosition(2, 2);
        list << ChannelPosition(2, 3);
        list << ChannelPosition(3, 0);
        list << ChannelPosition(3, 1);
        list << ChannelPosition(3, 2);
        list << ChannelPosition(3, 3);
        list << ChannelPosition(0, 4);
        list << ChannelPosition(1, 4);
        list << ChannelPosition(2, 4);
        list << ChannelPosition(3, 4);
        map = makeSpecificLayout(list, layout);
        break;
    }
    case LAYOUTMODE_16: {
        map = makeNormalLayout(4, 4, layout);
        break;
    }
    case LAYOUTMODE_25: {
        map = makeNormalLayout(5, 5, layout);
        break;
    }
    case LAYOUTMODE_32: {
        map = makeNormalLayout(4, 8, layout);
        break;
    }
    case LAYOUTMODE_32_2: {
        ChannelPositionList list;
        list << ChannelPosition(0, 0, 3, 3);
        list << ChannelPosition(0, 3, 2, 2);
        list << ChannelPosition(0, 5, 2, 2);
        list << ChannelPosition(2, 3);
        list << ChannelPosition(2, 4);
        list << ChannelPosition(2, 5);
        list << ChannelPosition(2, 6);
        list << ChannelPosition(3, 0, 2, 2);
        list << ChannelPosition(3, 2);
        list << ChannelPosition(3, 3);
        list << ChannelPosition(3, 4);
        list << ChannelPosition(3, 5);
        list << ChannelPosition(3, 6);
        list << ChannelPosition(4, 2);
        list << ChannelPosition(4, 3);
        list << ChannelPosition(4, 4);
        list << ChannelPosition(4, 5);
        list << ChannelPosition(4, 6);
        list << ChannelPosition(5, 0);
        list << ChannelPosition(5, 1);
        list << ChannelPosition(5, 2);
        list << ChannelPosition(5, 3);
        list << ChannelPosition(5, 4);
        list << ChannelPosition(5, 5);
        list << ChannelPosition(5, 6);
        list << ChannelPosition(6, 0);
        list << ChannelPosition(6, 1);
        list << ChannelPosition(6, 2);
        list << ChannelPosition(6, 3);
        list << ChannelPosition(6, 4);
        list << ChannelPosition(6, 5);
        list << ChannelPosition(6, 6);
        //
        map = makeSpecificLayout(list, layout);
        break;
    }
    case LAYOUTMODE_36: {
        map = makeNormalLayout(6, 6, layout);
        break;
    }
    case LAYOUTMODE_64: {
      map = makeNormalLayout(8, 8, layout);
      break;
    }
    default:
        break;
    }
    return map;
}

QMap<int, ChannelPositionList> MyLayout::makeNormalLayout(int row, int column, const mosaic &layoutData)
{
    QMap<int, ChannelPositionList> pageMap;

    int maxDevice = qMsNvr->maxChannel();
    int pageCount = qCeil((qreal)maxDevice / (row * column));
    int index = 0;
    for (int page = 0; page < pageCount; ++page) {
        ChannelPositionList positionList;

        for (int r = 0; r < row; ++r) {
            for (int c = 0; c < column; ++c) {
                ChannelPosition position(r, c);
                position.index = index;
                if (index < MAX_CAMERA) {
                    position.channel = layoutData.channels[index];
                } else {
                    position.channel = -1;
                }
                positionList.append(position);
                index++;
            }
        }
        pageMap.insert(page, positionList);
    }
    return pageMap;
}

QMap<int, ChannelPositionList> MyLayout::makeSpecificLayout(const QList<ChannelPosition> &list, const mosaic &layoutData)
{
    QMap<int, ChannelPositionList> pageMap;

    int maxDevice = qMsNvr->maxChannel();
    int pageCount = qCeil((qreal)maxDevice / list.count());
    int index = 0;
    for (int page = 0; page < pageCount; ++page) {
        ChannelPositionList positionList = list;
        for (int i = 0; i < positionList.size(); ++i) {
            ChannelPosition &position = positionList[i];
            position.index = index;
            if (index < MAX_CAMERA) {
                position.channel = layoutData.channels[index];
            } else {
                position.channel = -1;
            }
            index++;
        }
        pageMap.insert(page, positionList);
    }
    return pageMap;
}
