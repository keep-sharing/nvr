#include "GraphicsMultiRegionScene.h"
#include "MyDebug.h"

extern "C" {
#include "msg.h"
}

GraphicsMultiRegionScene::GraphicsMultiRegionScene(QObject *parent)
    : DrawScenePolygon(parent)
{
}

void GraphicsMultiRegionScene::setCurrentIndex(int index)
{
    qMsDebug() << index;
    m_currentIndex = index;

    for (auto iter = m_itemMap.constBegin(); iter != m_itemMap.constEnd(); ++iter) {
        GraphicsMultiRegionItem *item = iter.value();
        if (!item->isFinished()) {
            item->clear();
        }
        item->setSelected(false);
        item->setEditable(false);
        item->setZValue(0);
    }
}

void GraphicsMultiRegionScene::setEditable(bool enable)
{
    GraphicsMultiRegionItem *item = currentItem();
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

void GraphicsMultiRegionScene::clearAllEditState()
{
    for (auto iter = m_itemMap.constBegin(); iter != m_itemMap.constEnd(); ++iter) {
        GraphicsMultiRegionItem *item = iter.value();
        item->endEdit();
    }
}

void GraphicsMultiRegionScene::setRegions(MsIpcRegionalArea *regions, int size)
{
    int backupIndex = m_currentIndex;
    for (int i = size - 1; i > -1; --i) {
        m_currentIndex = i;
        const MsIpcRegionalArea &region = regions[i];
        setPolygon(region.polygonX, region.polygonY);
    }
    m_currentIndex = backupIndex;
}

void GraphicsMultiRegionScene::getRegions(MsIpcRegionalArea *regions, int size)
{
    int backupIndex = m_currentIndex;
    for (int i = 0; i < size; ++i) {
        m_currentIndex = i;
        MsIpcRegionalArea &region = regions[i];
        getPolygon(region.polygonX, sizeof(region.polygonX), region.polygonY, sizeof(region.polygonY));
    }
    m_currentIndex = backupIndex;
}

void GraphicsMultiRegionScene::getRegion(MsIpcRegionalArea *regions, int index)
{
    QMS_ASSERT(index == m_currentIndex);

    MsIpcRegionalArea &region = regions[index];
    getPolygon(region.polygonX, sizeof(region.polygonX), region.polygonY, sizeof(region.polygonY));
}

void GraphicsMultiRegionScene::clearAll()
{
    int backupIndex = m_currentIndex;
    for (auto iter = m_itemMap.constBegin(); iter != m_itemMap.constEnd(); ++iter) {
        m_currentIndex = iter.key();
        clearPolygon();
    }
    m_currentIndex = backupIndex;
}

void GraphicsMultiRegionScene::setAlarmable(bool enable)
{
    m_isAlarmable = enable;
}

void GraphicsMultiRegionScene::showRegionAlarm(const RegionalAlarmInfo &info)
{
    for (auto iter = m_itemMap.constBegin(); iter != m_itemMap.constEnd(); ++iter) {
        GraphicsMultiRegionItem *item = iter.value();
        item->updateRegionAlarm(info);
    }
}

GraphicsMultiRegionItem *GraphicsMultiRegionScene::currentItem()
{
    GraphicsMultiRegionItem *item = m_itemMap.value(m_currentIndex);
    if (!item) {
        item = new GraphicsMultiRegionItem();
        item->setIndex(m_currentIndex);
        item->setAlarmable(m_isAlarmable);
        addItem(item);
        m_itemMap.insert(m_currentIndex, item);
    }
    return item;
}
