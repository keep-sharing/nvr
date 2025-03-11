#include "DrawItemSpaceHeatMap.h"
#include <QPainter>
#include <QElapsedTimer>
#include <QtDebug>

DrawItemSpaceHeatMap::DrawItemSpaceHeatMap(QGraphicsItem *parent) :
    QGraphicsRectItem(parent)
{

}

void DrawItemSpaceHeatMap::showHeatMap(int max, int min, QImage colorImage, QImage heatmapImage)
{
    m_max = max;
    m_min = min;
    m_colorImage = colorImage;
    m_heatMapImage = heatmapImage;

    update();
}

void DrawItemSpaceHeatMap::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();
    painter->setRenderHint(QPainter::SmoothPixmapTransform);

    QRectF rc = rect();

    //
    QRectF heatMapRect = rc;
    heatMapRect.setWidth(rc.height() - 50);
    heatMapRect.setHeight(rc.height() - 50);
    heatMapRect.moveCenter(QPointF(rc.center().x(), rc.center().y() - 25));
    QRectF backgroundRect = heatMapRect;
    backgroundRect.setWidth((qreal)m_heatMapImage.width() / m_heatMapImage.height() * backgroundRect.height());
    backgroundRect.moveCenter(heatMapRect.center());
    painter->drawImage(backgroundRect, m_heatMapImage);

    //
    QRectF colorRect(rc.center().x() - 150, rc.bottom() - 30, 300, 30);
    painter->drawImage(colorRect, m_colorImage);

    //
    QRectF leftTextRect = colorRect;
    leftTextRect.setLeft(rc.left());
    leftTextRect.setRight(colorRect.left() - 10);
    painter->drawText(leftTextRect, Qt::AlignVCenter | Qt::AlignRight, "0");

    //
    QString rightText;
    int d = m_max / 86400;
    if (d > 0)
    {
        rightText.append(QString("%1d.").arg(d));
    }
    int hr = m_max % 86400 / 3600;
    if (hr > 0)
    {
        rightText.append(QString("%1hr.").arg(hr));
    }
    int min = m_max % 3600 / 60;
    if (min > 0)
    {
        rightText.append(QString("%1min.").arg(min));
    }
    int sec = m_max % 3600 % 60;
    rightText.append(QString("%1sec.").arg(sec));
    QRectF rightTextRect = colorRect;
    rightTextRect.setRight(rc.right());
    rightTextRect.setLeft(colorRect.right() + 10);
    painter->drawText(rightTextRect, Qt::AlignVCenter | Qt::AlignLeft, rightText);

    painter->restore();
}
