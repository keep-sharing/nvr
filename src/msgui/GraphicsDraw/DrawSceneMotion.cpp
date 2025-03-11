#include "DrawSceneMotion.h"

DrawSceneMotion::DrawSceneMotion(QObject *parent) :
    QGraphicsScene(parent)
{
    m_motionItem = new DrawItemMotion();
    addItem(m_motionItem);

    connect(this, SIGNAL(sceneRectChanged(QRectF)), this, SLOT(onSceneRectChanged(QRectF)));
}

void DrawSceneMotion::clearAll()
{
    m_motionItem->clearAll();
}

void DrawSceneMotion::selectAll()
{
    m_motionItem->selectAll();
}

void DrawSceneMotion::setRegion(char *region)
{
    m_motionItem->setRegion(region);
}

void DrawSceneMotion::getRegion(char *region)
{
    m_motionItem->getRegion(region);
}

void DrawSceneMotion::setObjectSize(int value)
{
    m_motionItem->setObjectSize(value);
}

void DrawSceneMotion::onSceneRectChanged(const QRectF &rect)
{
    m_motionItem->setRect(rect);
}
