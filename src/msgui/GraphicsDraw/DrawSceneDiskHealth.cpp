#include "DrawSceneDiskHealth.h"
#include "DrawItemDiskHealth.h"
#include <QtDebug>

DrawSceneDiskHealth::DrawSceneDiskHealth(QObject *parent)
    : QGraphicsScene(parent)
{

    m_diskItem = new DrawItemDiskHealth();
    addItem(m_diskItem);
}

void DrawSceneDiskHealth::showDiskHealthMap(struct disk_temperature *temperatureList)
{
    m_diskItem->setRect(sceneRect());
    m_diskItem->showDiskHealthMap(temperatureList);
}
