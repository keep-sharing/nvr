#include "GraphicsScene.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include "MyDebug.h"
#include <QGraphicsItem>

GraphicsScene::GraphicsScene(QObject *parent) :
    QGraphicsScene(parent)
{

}

void GraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mousePressEvent(event);
}

void GraphicsScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(painter)
    Q_UNUSED(rect)

#if 0
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);
    painter->setPen(QPen(Qt::red, 3));
    painter->setBrush(QBrush(QColor(0, 0, 255, 40)));
    painter->drawRect(sceneRect());
    painter->restore();
#endif
}
