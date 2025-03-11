#include "DrawMultiControlPolygon.h"
#include "MyDebug.h"
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
extern "C" {
#include "msg.h"
}

DrawMultiControlPolygon::DrawMultiControlPolygon(QGraphicsItem *parent)
    : DrawControlPolygon(parent)
{
}

void DrawMultiControlPolygon::setCurrentIndex(int index)
{
    qMsDebug() << index;
    m_currentIndex = index;

    for (auto iter = m_itemMap.constBegin(); iter != m_itemMap.constEnd(); ++iter) {
        DrawMultiRegionItem *item = iter.value();
        if (!item->isFinished()) {
            item->clear();
        }
        item->setSelected(false);
        item->setEditable(false);
        item->setZValue(1);
    }
}

void DrawMultiControlPolygon::setEditable(bool enable)
{
    DrawMultiRegionItem *item = currentItem();
    if (!item->isFinished()) {
        item->clear();
    }
    item->setZValue(999);
    item->setSelected(true);
    item->setEditable(enable);
    //点击编辑认为是开始编辑，保存到IPC后认为编辑结束
    if (enable) {
        item->beginEdit();
    }
}

void DrawMultiControlPolygon::clearAllEditState()
{
    for (auto iter = m_itemMap.constBegin(); iter != m_itemMap.constEnd(); ++iter) {
        DrawMultiRegionItem *item = iter.value();
        item->endEdit();
    }
}

void DrawMultiControlPolygon::setPolygon(const QString &xList, const QString &yList)
{
    DrawControlPolygon::setPolygon(xList, yList);
    currentItem()->setIsShowText(false);
    currentItem()->setSelected(true);
    currentItem()->setEditable(true);
}

void DrawMultiControlPolygon::setRegions(SMART_REGION_INFO *regions,int index, int size)
{
    m_size = size;
    m_scene = index;
    int backupIndex = m_currentIndex;
    for (int i = size - 1; i > -1; --i) {
        m_currentIndex = i;
        const MS_POLYGON &region = regions[i].region[index];
        sprintf(m_regions[i].region[index].polygonX, "%s", region.polygonX);
        sprintf(m_regions[i].region[index].polygonY, "%s", region.polygonY);
        //qDebug()<<region.polygonX<<region.polygonY<<i;
        DrawControlPolygon::setPolygon(region.polygonX, region.polygonY);
        currentItem()->setIsShowText(true);
        currentItem()->setSelected(false);
        currentItem()->setEditable(false);
    }
    m_currentIndex = backupIndex;
}

void DrawMultiControlPolygon::getRegions(SMART_REGION_INFO *regions,int index, int size)
{
    int backupIndex = m_currentIndex;
    for (int i = 0; i < size; ++i) {
        m_currentIndex = i;
        MS_POLYGON &region = regions[i].region[index];
        getPolygon(region.polygonX, sizeof(region.polygonX), region.polygonY, sizeof(region.polygonY));
    }
    m_currentIndex = backupIndex;
}

void DrawMultiControlPolygon::getRegion(MsIpcRegionalArea *regions, int index)
{
    QMS_ASSERT(index == m_currentIndex);

    MsIpcRegionalArea &region = regions[index];
    getPolygon(region.polygonX, sizeof(region.polygonX), region.polygonY, sizeof(region.polygonY));
}

void DrawMultiControlPolygon::clearAll()
{
    int backupIndex = m_currentIndex;
    for (auto iter = m_itemMap.constBegin(); iter != m_itemMap.constEnd(); ++iter) {
        m_currentIndex = iter.key();
        clearPolygon();
    }
    m_currentIndex = backupIndex;
}

void DrawMultiControlPolygon::setRect(const QRectF &rect)
{
    if (m_itemMap.count() < 1) {
        return;
    }
    for (auto iter = m_itemMap.constBegin(); iter != m_itemMap.constEnd(); ++iter) {
        DrawMultiRegionItem *item = iter.value();
        item->setRect(rect);
    }
    setRegions(m_regions, m_scene, m_size);
}

void DrawMultiControlPolygon::setAlarmable(bool enable)
{
    m_isAlarmable = enable;
}

void DrawMultiControlPolygon::showRegionAlarm(const RegionalAlarmInfo &info)
{
    for (auto iter = m_itemMap.constBegin(); iter != m_itemMap.constEnd(); ++iter) {
        DrawMultiRegionItem *item = iter.value();
        item->updateRegionAlarm(info);
    }
}

void DrawMultiControlPolygon::setAlarmState(int state, int index)
{
    for (auto iter = m_itemMap.constBegin(); iter != m_itemMap.constEnd(); ++iter) {
        DrawMultiRegionItem *item = iter.value();
        if (item->index() == index && item->alarm() != state) {
            item->setAlarm(state);
        }
    }
}

void DrawMultiControlPolygon::clearAlarmState()
{
    for (auto iter = m_itemMap.constBegin(); iter != m_itemMap.constEnd(); ++iter) {
        DrawMultiRegionItem *item = iter.value();
        item->setAlarm(0);
    }
}

DrawMultiRegionItem *DrawMultiControlPolygon::currentItem()
{
    DrawMultiRegionItem *item = m_itemMap.value(m_currentIndex);
    if (!item) {
        item = new DrawMultiRegionItem();
        item->setIndex(m_currentIndex);
        item->setAlarmable(m_isAlarmable);
        item->setParentItem(this);
        m_itemMap.insert(m_currentIndex, item);
    }
    return item;
}
