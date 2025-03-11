#include "DrawItemObjectBox.h"
#include "MyDebug.h"
#include <QGraphicsScene>
#include <QPainter>

extern "C" {
#include "msg.h"
}

DrawItemObjectBox::DrawItemObjectBox(QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
{

}

DrawItemObjectBox::~DrawItemObjectBox()
{

}

void DrawItemObjectBox::showBox(int id, int type, int alarm)
{
    m_mode = ModeVca;
    m_id = id;
    m_class = type;
    m_alarm = alarm;
    prepareGeometryChange();
}

void DrawItemObjectBox::showBox(const RegionalRectBox &box)
{
    m_mode = ModeRegionPeopleCounting;
    memcpy(&m_regionRectInfo, &box, sizeof(RegionalRectBox));
}

QRectF DrawItemObjectBox::boundingRect() const
{
    QRectF rc = rect();
    rc.adjust(-200, -200, 200, 200);
    return rc;
}

void DrawItemObjectBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    switch (m_mode) {
    case ModeVca:
        painter->setClipRect(rect());
        drawVca(painter);
        break;
    case ModeRegionPeopleCounting:
        drawRegionalPeopleCounting(painter);
        break;
    default:
        break;
    }
}

void DrawItemObjectBox::drawVca(QPainter *painter)
{
    painter->save();
    //
    painter->setRenderHints(QPainter::Antialiasing);
    switch (m_class) {
    case ObjectBox::VCA_OBJECT_Person: //人
        painter->setPen(QPen(QColor(255, 205, 51), 2));
        break;
    case ObjectBox::VCA_OBJECT_Vehicle: //车
        painter->setPen(QPen(QColor(65, 160, 13), 2));
        break;
    }
    if (m_alarm) {
        painter->setPen(QPen(QColor(255, 0, 0), 2));
    }
    if (m_alarm || m_class) {
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(rect());
    //
        painter->drawText(rect(), Qt::AlignLeft | Qt::AlignTop, QString("%1").arg(m_id));
    //
    }
    painter->restore();
}

void DrawItemObjectBox::drawRegionalPeopleCounting(QPainter *painter)
{
    painter->save();
    //
    painter->setRenderHints(QPainter::Antialiasing);
    painter->setPen(QPen(QColor(255, 205, 51), 2));
    if (m_regionRectInfo.alarm) {
        painter->setPen(QPen(QColor(255, 0, 0), 2));
    }
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rect());
    //
    QRectF textRc = rect();
    textRc.adjust(2, 0, -4, 0);
    painter->drawText(textRc, Qt::AlignLeft | Qt::AlignTop, QString("%1").arg(m_regionRectInfo.id));
    //
    int time = 0;
    for (int i = 0; i < 4; ++i) {
        int t = m_regionRectInfo.timeValue[i];
        time = qMax(time, t);
    }
    painter->drawText(textRc, Qt::AlignRight | Qt::AlignTop, QString("%1s").arg(time));
    //
    painter->restore();
}
