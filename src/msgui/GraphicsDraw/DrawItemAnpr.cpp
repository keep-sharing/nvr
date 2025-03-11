#include "DrawItemAnpr.h"
#include <QtDebug>
#include "DrawSceneAnpr.h"

DrawItemAnpr::DrawItemAnpr(QGraphicsItem *parent) :
    DrawItemMask(parent)
{
    m_textItem = new QGraphicsTextItem(this);
}

void DrawItemAnpr::setIndex(int index)
{
    m_index = index;
}

int DrawItemAnpr::index() const
{
    return m_index;
}

void DrawItemAnpr::setName(const QString &name)
{
    m_name = name;
}

QString DrawItemAnpr::name(bool jsonSupport)
{
    if (m_name.isEmpty() && jsonSupport)
    {
        m_name = QString("ROI_%1").arg(m_index + 1);
    }
    return m_name;
}

void DrawItemAnpr::updateSizeText()
{
    DrawSceneAnpr *anprScene = static_cast<DrawSceneAnpr *>(scene());

    const QRectF &rc = realRect();
    qreal w = rc.width() / anprScene->width() * anprScene->baseRegionWidth();
    qreal h = rc.height() / anprScene->height() * anprScene->baseRegionHeight();
    m_textItem->setHtml(QString("<html><head/><body><p><span style=\" color:#ff0000;\">%1 %2*%3</span></p></body></html>").arg(m_index + 1).arg((int)w).arg((int)h));
    m_textItem->setPos(rect().left() + 2, rect().top());
}

bool DrawItemAnpr::isText()
{
    if (m_textItem) {
        return false;
    }
    return true;
}

void DrawItemAnpr::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    DrawItemMask::mouseMoveEvent(event);

    //
    updateSizeText();
}
