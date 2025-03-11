#include "DrawSceneHeatMap.h"
#include "DrawItemSpaceHeatMap.h"
#include "DrawItemTimeHeatMap.h"
#include <QtDebug>

DrawSceneHeatMap::DrawSceneHeatMap(QObject *parent)
    : QGraphicsScene(parent)
{
    m_spaceItem = new DrawItemSpaceHeatMap();
    addItem(m_spaceItem);
    m_spaceItem->hide();

    m_timeItem = new DrawItemTimeHeatMap();
    addItem(m_timeItem);
    m_timeItem->hide();
}

void DrawSceneHeatMap::clearInfo()
{
    m_spaceItem->hide();
    m_timeItem->hide();
}

void DrawSceneHeatMap::showSpaceHeatMap()
{
    m_spaceItem->show();
    m_timeItem->hide();
}

void DrawSceneHeatMap::showSpaceHeatMap(const ImageParameters &imageParameters)
{
    m_spaceItem->setRect(sceneRect());
    m_spaceItem->showHeatMap(imageParameters.max, imageParameters.min, imageParameters.colorImage, imageParameters.heatmapImage);

    m_spaceItem->show();
    m_timeItem->hide();
}

void DrawSceneHeatMap::showTimeHeatMap()
{
    m_spaceItem->hide();
    m_timeItem->show();
}

void DrawSceneHeatMap::showTimeHeatMap(const QString &text, int reportType, const QDateTime &dateTime)
{
    m_timeItem->setRect(sceneRect());
    m_timeItem->showHeatMap(text, reportType, dateTime);

    m_spaceItem->hide();
    m_timeItem->show();
}

bool DrawSceneHeatMap::saveTimeHeatMap(const QString &fileName)
{
    return m_timeItem->saveTimeHeatMap(fileName);
}

bool DrawSceneHeatMap::saveTimeHeatMap(const QString &text, int reportType, const QDateTime &dateTime, const QString &fileName)
{
    return m_timeItem->saveTimeHeatMap(text, reportType,dateTime,fileName);
}
