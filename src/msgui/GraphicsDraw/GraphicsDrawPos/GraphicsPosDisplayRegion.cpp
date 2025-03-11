#include "GraphicsPosDisplayRegion.h"
#include "MyDebug.h"
#include <QGraphicsScene>
#include <QGraphicsView>

GraphicsPosDisplayRegion::GraphicsPosDisplayRegion(QGraphicsItem *parent)
    : MsGraphicsObject(parent)
{
}

GraphicsPosDisplayRegion::~GraphicsPosDisplayRegion()
{
}

void GraphicsPosDisplayRegion::clearAll()
{
    for (auto iter = m_itemMap.constBegin(); iter != m_itemMap.constEnd(); ++iter) {
        auto *value = iter.value();
        delete value;
    }
    m_itemMap.clear();
}

void GraphicsPosDisplayRegion::setCurrentPos(int id)
{
    auto *item = m_itemMap.value(id);
    if (!item) {
        qMsWarning() << "item is nullptr, id:" << id;
        return;
    }

    for (auto iter = m_itemMap.constBegin(); iter != m_itemMap.constEnd(); ++iter) {
        auto *value = iter.value();
        if (value->getIndex() == id) {
            value->setZValue(1);
            value->setEnabled(true);
            value->setSelected(true);
        } else {
            value->setZValue(0);
            value->setEnabled(false);
            value->setSelected(false);
        }
    }

    //
    update(sceneRect());
    emit mouseDragging();
}

void GraphicsPosDisplayRegion::setPosItemRegion(int id, int x, int y, int w, int h)
{
    auto *item = m_itemMap.value(id, nullptr);
    if (!item) {
        item = new GraphicsPosDisplayRegionItem(this);
        item->setIndex(id);
        m_itemMap.insert(id, item);
    }

    QRectF rc = sceneRect();

    qreal _x = (qreal)x / 1000 * rc.width();
    qreal _y = (qreal)y / 1000 * rc.height();
    qreal _w = (qreal)w / 1000 * rc.width();
    qreal _h = (qreal)h / 1000 * rc.height();

    item->setRealRect(QRectF(_x, _y, _w, _h));
}

void GraphicsPosDisplayRegion::getPosItemRegion(int id, int &x, int &y, int &w, int &h)
{
    auto *item = m_itemMap.value(id, nullptr);
    if (item) {
        auto src = sceneRect();
        auto rc  = item->realRect();
        x        = qRound(rc.x() / src.width() * 1000);
        y        = qRound(rc.y() / src.height() * 1000);
        w        = qRound(rc.width() / src.width() * 1000);
        h        = qRound(rc.height() / src.height() * 1000);
    }
}

void GraphicsPosDisplayRegion::setEnabled(int current, bool enabled)
{
    for (auto iter = m_itemMap.constBegin(); iter != m_itemMap.constEnd(); ++iter) {
        auto *item = iter.value();
        item->unsetCursor();
        if (item->getIndex() == current) {
            item->setEnabled(enabled);
        } else {
            item->setEnabled(false);
        }
    }
    MsGraphicsObject::setEnabled(enabled);
}

QRect GraphicsPosDisplayRegion::posGlobalGeometry() const
{
    QRect rc;

    if (!scene()) {
        qMsWarning() << "scene is nullptr";
        return rc;
    }

    auto *item = m_itemMap.value(m_currentPos, nullptr);
    if (item) {
        rc            = item->realRect().toRect();
        auto viewList = scene()->views();
        if (!viewList.isEmpty()) {
            QPoint p = viewList.first()->mapToGlobal(rc.topLeft());
            rc.moveTopLeft(p);
        }
    }
    return rc;
}
