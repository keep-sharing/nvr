#include "GraphicsDrawLineCrossing.h"
#include "MyDebug.h"

GraphicsDrawLineCrossing::GraphicsDrawLineCrossing(QGraphicsItem *parent)
    : MsGraphicsObject(parent)
{
}

GraphicsDrawLineCrossing::~GraphicsDrawLineCrossing()
{
}

void GraphicsDrawLineCrossing::setLineCrossInfo(ms_linecrossing_info *info)
{
    setIpcMaxSize(info->width, info->height);
    for (int i = 0; i < 4; ++i) {
        const linecrossing_info &line = info->line[i];
        GraphicsDrawLineCrossingItem *item = m_lineMap.value(i);
        if (!item) {
            item = new GraphicsDrawLineCrossingItem(this);
            item->setIndex(i);
            item->setRect(boundingRect());
            m_lineMap.insert(i, item);
        }
        item->setBeginPoint(mapToQtPos(line.startX, line.startY));
        item->setEndPoint(mapToQtPos(line.stopX, line.stopY));
        item->setDirection(line.direction);
        item->setIsShowIndex(true);
    }
}

void GraphicsDrawLineCrossing::getLineCrossInfo(ms_linecrossing_info *info)
{
    for (int i = 0; i < 4; ++i) {
        linecrossing_info &line = info->line[i];
        GraphicsDrawLineCrossingItem *item = m_lineMap.value(i);
        if (item) {
            QPoint start = mapToIpcPos(item->beginPoint());
            QPoint stop = mapToIpcPos(item->endPoint());
            line.startX = start.x();
            line.startY = start.y();
            line.stopX = stop.x();
            line.stopY = stop.y();
            line.direction = item->direction();
        } else {
            line.startX = 0;
            line.startY = 0;
            line.stopX = 0;
            line.stopY = 0;
            line.direction = 0;
        }
    }
}

void GraphicsDrawLineCrossing::setLineCrossInfo(ms_linecrossing_info2 *info)
{
    setIpcMaxSize(info->width, info->height);
    for (int i = 0; i < 4; ++i) {
        const linecrossing_info2 &line = info->line[i];
        GraphicsDrawLineCrossingItem *item = m_lineMap.value(i);
        if (!item) {
            item = new GraphicsDrawLineCrossingItem(this);
            item->setIndex(i);
            item->setRect(boundingRect());
            m_lineMap.insert(i, item);
        }
        item->setBeginPoint(mapToQtPos(line.startX, line.startY));
        item->setEndPoint(mapToQtPos(line.stopX, line.stopY));
        item->setDirection(line.direction);
        if (!item->beginPoint().isNull() && !item->endPoint().isNull()) {
            item->setIsShowIndex(true);
        }
    }
}

void GraphicsDrawLineCrossing::getLineCrossInfo(ms_linecrossing_info2 *info)
{
    for (int i = 0; i < 4; ++i) {
        linecrossing_info2 &line = info->line[i];
        GraphicsDrawLineCrossingItem *item = m_lineMap.value(i);
        if (item) {
            QPoint start = mapToIpcPos(item->beginPoint());
            QPoint stop = mapToIpcPos(item->endPoint());
            line.startX = start.x();
            line.startY = start.y();
            line.stopX = stop.x();
            line.stopY = stop.y();
            line.direction = item->direction();
        } else {
            line.startX = 0;
            line.startY = 0;
            line.stopX = 0;
            line.stopY = 0;
            line.direction = 0;
        }
    }
}

void GraphicsDrawLineCrossing::setLineCrossInfo(ms_linecrossing_info2 *info, int index)
{
    setIpcMaxSize(info->width, info->height);
    for (int i = 0; i < 4; ++i) {
        const MS_LINE &line = info->line[i].line[index];
        GraphicsDrawLineCrossingItem *item = m_lineMap.value(i);
        if (!item) {
            item = new GraphicsDrawLineCrossingItem(this);
            item->setIndex(i);
            item->setRect(boundingRect());
            m_lineMap.insert(i, item);
        }
        item->setBeginPoint(mapToQtPos(line.startX, line.startY));
        item->setEndPoint(mapToQtPos(line.stopX, line.stopY));
        item->setDirection(line.direction);
        item->setIsShowIndex(true);
    }
}

void GraphicsDrawLineCrossing::getLineCrossInfo(ms_linecrossing_info2 *info, int index)
{
    for (int i = 0; i < 4; ++i) {
        MS_LINE &line = info->line[i].line[index];
        GraphicsDrawLineCrossingItem *item = m_lineMap.value(i);
        if (item) {
            QPoint start = mapToIpcPos(item->beginPoint());
            QPoint stop = mapToIpcPos(item->endPoint());
            line.startX = start.x();
            line.startY = start.y();
            line.stopX = stop.x();
            line.stopY = stop.y();
            line.direction = item->direction();
        } else {
            line.startX = 0;
            line.startY = 0;
            line.stopX = 0;
            line.stopY = 0;
            line.direction = 0;
        }
    }
}

void GraphicsDrawLineCrossing::setPeopleCountInfo(ms_smart_event_people_cnt *info)
{
    m_info = *info;
    setIpcMaxSize(info->width, info->height);
    int count = info->lineNum;
    for (int i = 0; i < count; ++i) {
        GraphicsDrawLineCrossingItem *item = m_lineMap.value(i);
        if (!item) {
            item = new GraphicsDrawLineCrossingItem(this);
            item->setIndex(i);
            item->setRect(boundingRect());
            m_lineMap.insert(i, item);
        }
        item->setDirection(info->lines[i].direction);
        item->setBeginPoint(mapToQtPos(info->lines[i].startX, info->lines[i].startY));
        item->setEndPoint(mapToQtPos(info->lines[i].stopX, info->lines[i].stopY));
        if (count > 1) {
            item->setIsShowIndex(true);
            item->setIsShowLineInfo(info->lines[i].showCntEnable & info->lines[i].lineEnable);
        } else {
            item->setIsShowIndex(false);
            item->setIsShowLineInfo(false);
        }
    }
}

void GraphicsDrawLineCrossing::getPeopleCountInfo(ms_smart_event_people_cnt *info)
{
    int count = info->lineNum;
    for (int i = 0; i < count; ++i) {
        GraphicsDrawLineCrossingItem *item = m_lineMap.value(i);
        if (item) {
            QPoint start = mapToIpcPos(item->beginPoint());
            QPoint stop = mapToIpcPos(item->endPoint());
            info->lines[i].direction = item->direction();
            info->lines[i].startX = start.x();
            info->lines[i].startY = start.y();
            info->lines[i].stopX = stop.x();
            info->lines[i].stopY = stop.y();
        } else {
            info->lines[i].startX = 0;
            info->lines[i].startY = 0;
            info->lines[i].stopX = 0;
            info->lines[i].stopY = 0;
            info->lines[i].direction = 0;
        }
    }
}

void GraphicsDrawLineCrossing::setCurrentLine(int index)
{
    m_currentLineIndex = index;
    for (auto iter = m_lineMap.constBegin(); iter != m_lineMap.constEnd(); ++iter) {
        auto item = iter.value();
        if (iter.key() == m_currentLineIndex) {
            item->setZValue(1);
        } else {
            item->setZValue(0);
        }
    }
}

void GraphicsDrawLineCrossing::setLineDirection(int direction)
{
    auto item = m_lineMap.value(m_currentLineIndex);
    if (item) {
        item->setDirection(direction);
    }
}

int GraphicsDrawLineCrossing::lineDirection(int index)
{
    auto item = m_lineMap.value(index);
    if (item) {
        return item->direction();
    } else {
        qMsCritical() << "item is null";
        return 0;
    }
}

void GraphicsDrawLineCrossing::clearLine(int index)
{
    auto item = m_lineMap.value(index);
    if (item) {
        item->clear();
    }
}

void GraphicsDrawLineCrossing::clearAllLine()
{
    for (auto iter = m_lineMap.constBegin(); iter != m_lineMap.constEnd(); ++iter) {
        auto item = iter.value();
        item->clear();
    }
}

void GraphicsDrawLineCrossing::setEnabled(bool enabled)
{
    MsGraphicsObject::setEnabled(enabled);

    for (auto iter = m_lineMap.constBegin(); iter != m_lineMap.constEnd(); ++iter) {
        auto item = iter.value();
        item->unsetCursor();
    }
}

void GraphicsDrawLineCrossing::setIsShowLineInfo(bool enabled, int index)
{
    GraphicsDrawLineCrossingItem *item = m_lineMap.value(index);
    item->setIsShowLineInfo(enabled);
}

void GraphicsDrawLineCrossing::setLineCntOsdType(int osdType)
{
    for (int i = 0; i < 4; ++i) {
        GraphicsDrawLineCrossingItem *item = m_lineMap.value(i);
        item->setOsdType(osdType);
    }
}

void GraphicsDrawLineCrossing::setLineCntData(MS_PEOPLECNT_DATA *info)
{
    for (int i = 0; i < m_lineMap.count(); ++i) {
        GraphicsDrawLineCrossingItem *item = m_lineMap.value(i);
        item->setLineInfo(info->lineInCnt[i], info->lineOutCnt[i], info->lineSumCnt[i], info->lineCapacityCnt[i]);
    }
}

void GraphicsDrawLineCrossing::setRect(const QRectF &rect)
{
    if (m_lineMap.count() < 1) {
        return;
    }
    for (auto iter = m_lineMap.constBegin(); iter != m_lineMap.constEnd(); ++iter) {
        GraphicsDrawLineCrossingItem *item = iter.value();
        item->setRect(rect);
    }
    setPeopleCountInfo(&m_info);
}

void GraphicsDrawLineCrossing::setLineCrossState(int state, int line)
{
    if (m_lineMap.contains(line)) {
        auto *item = m_lineMap.value(line);
        item->setAlarmState(state);
    }
}
