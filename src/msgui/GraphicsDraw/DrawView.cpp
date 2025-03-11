#include "DrawView.h"
#include "MyDebug.h"
#include <QPainter>
#include <QResizeEvent>

DrawView::DrawView(QWidget *parent)
    : QGraphicsView(parent)
{
    setStyleSheet("border: 0px; background: transparent");
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
}

void DrawView::setScene(QGraphicsScene *scene)
{
    QGraphicsView::setScene(scene);

    if (scene) {
        scene->setSceneRect(QRectF(QPoint(0, 0), size()));
    }
}

void DrawView::setGeometry(const QRect &rc)
{
    QGraphicsView::setGeometry(rc);

    QGraphicsScene *s = scene();
    if (s) {
        s->setSceneRect(QRectF(QPoint(0, 0), rc.size()));
    }
}

void DrawView::resizeEvent(QResizeEvent *event)
{
    QGraphicsScene *s = scene();
    if (s) {
        s->setSceneRect(QRectF(QPoint(0, 0), event->size()));
    }

    QGraphicsView::resizeEvent(event);
}

void DrawView::showEvent(QShowEvent *event)
{
    QGraphicsScene *s = scene();
    if (s) {
        s->setSceneRect(QRectF(QPoint(0, 0), size()));
    }

    QGraphicsView::showEvent(event);
}
