#include "DrawItemDiskHealth.h"
#include "MsDevice.h"
#include "centralmessage.h"
#include "msfs_disk.h"
#include <QDateTime>
#include <QElapsedTimer>
#include <QFile>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <QtDebug>
#include <qmath.h>

DrawItemDiskHealth::DrawItemDiskHealth(QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
{
    setAcceptHoverEvents(true);
    memset(m_values, 0, sizeof(struct disk_temperature) * MAX_DISK_HM_TEMPERATURE);
}

void DrawItemDiskHealth::showDiskHealthMap(struct disk_temperature *temperatureList)
{
    if (temperatureList != NULL) {
        memcpy(m_values, temperatureList, sizeof(struct disk_temperature) * MAX_DISK_HM_TEMPERATURE);
    } else {
        memset(m_values, 0, sizeof(struct disk_temperature) * MAX_DISK_HM_TEMPERATURE);
    }
    update();
}

void DrawItemDiskHealth::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    drawGrid(painter);
    drawLine(painter);
}

void DrawItemDiskHealth::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    int tempIndex = -1;

    for (int i = 0; i < m_valueRects.size(); ++i) {
        QRectF rc = m_valueRects.at(i);
        if (rc.contains(event->pos())) {
            tempIndex = i;
            break;
        }
    }

    if (m_hoverIndex != tempIndex) {
        m_hoverIndex = tempIndex;
        update();
    }

    QGraphicsRectItem::hoverMoveEvent(event);
}

void DrawItemDiskHealth::drawGrid(QPainter *painter)
{
    painter->save();

    m_left = rect().left() + 70;
    m_right = rect().right() - 80;
    m_top = rect().top() + 10;
    m_bottom = rect().bottom() - 30;

    int yCount = 8;
    qreal yStep = qreal(m_bottom - m_top) / (yCount - 1);
    qreal y = m_top;
    for (int i = 0; i < yCount; ++i) {
        painter->setPen(QPen(QColor("#bdbdbd"), 1));
        if (i != 0) {
            painter->drawLine(QPointF(m_left, y), QPointF(m_right, y));
        }
        //
        QString text = QString("%1").arg(100 - (i * 20));

        QRectF textRect;
        textRect.setLeft(rect().left());
        textRect.setRight(m_left - 15);
        textRect.setTop(y - 10);
        textRect.setBottom(y + 10);
        painter->setPen(QPen(QColor("#575757")));
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignRight, text);

        //
        y += yStep;
    }

    int xCount = 4;
    int xStart = 0;

    m_xStep = qreal(m_right - m_left) / (xCount - 1);
    qreal x = m_left;
    int xValue = xStart;
    for (int i = 0; i < xCount; ++i) {
        //
        QString text;
        switch (i) {
        case 0:
            text = QString("30 days ago");
            break;
        case 1:
            text = QString("20 days ago");
            break;
        case 2:
            text = QString("10 days ago");
            break;
        case 3:
            text = QString("Now");
            break;
        }
        xValue++;
        if (xValue > xCount) {
            xValue = 1;
        }
        QRectF textRect;
        textRect.setLeft(x - 60);
        textRect.setRight(x + 60);
        textRect.setTop(m_bottom + 5);
        textRect.setBottom(m_bottom + 25);
        painter->setPen(QPen(QColor("#575757")));
        painter->drawText(textRect, Qt::AlignCenter, text);
        //
        x += m_xStep;
    }
    QRectF colorRect(m_left, m_top, m_right - 70, 2*yStep);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor("#ffe3a8"));
    painter->drawRect(colorRect);

    QRectF colorRect2(m_left, m_bottom - 2*yStep, m_right - 70, 2*yStep);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor("#beebff"));
    painter->drawRect(colorRect2);

    QRectF textRect;
    textRect.setLeft(m_right );
    textRect.setRight(m_right + 40);
    textRect.setTop(m_bottom - 10);
    textRect.setBottom(m_bottom + 10);
    painter->setPen(QPen(QColor("#575757")));
    painter->drawText(textRect, Qt::AlignCenter, "Time");

    painter->restore();
}

void DrawItemDiskHealth::drawLine(QPainter *painter)
{
    if (m_values[0].timestamp == 0) {
        return;
    }
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    m_valueRects.clear();
    m_xStep = qreal(m_right - m_left) / (MAX_DISK_HM_TEMPERATURE - 1);

    QVector<QPointF> points;
    qreal x = m_left;
    qreal y = 0;
    for (int i = 0; i < MAX_DISK_HM_TEMPERATURE; ++i) {
        if (m_values[i].temperature == -300) {
            x += m_xStep;
            continue;
        }
        qreal yValue = m_values[i].temperature;
        if (m_valueRects.count() == m_hoverIndex) {
            m_hoverValue = yValue;
            m_hoverTime = (uint)m_values[i].timestamp;
        }
        qreal tempValue = yValue;
        if (tempValue < -40) {
            tempValue = -40;
        }
        if (tempValue > 100) {
            tempValue = 100;
        }
        tempValue += 40;

        y = m_bottom - (qreal)(m_bottom - m_top) / 140 * tempValue;
        points.append(QPointF(x, y));
        m_valueRects.append(QRectF(x - 10, y - 10, 20, 20));
        x += m_xStep;
    }
    painter->setPen(QPen(QColor("#51aafe"), 2));
    painter->drawPolyline(points);

    //
    for (int i = 0; i < points.size(); ++i) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QBrush(QColor("#51aafe")));

        const QPointF &p = points.at(i);

        if (i == m_hoverIndex) {
            QColor color("#51aafe");
            color.setAlpha(120);
            painter->setBrush(QBrush(color));
            painter->drawEllipse(p, 10, 10);

            QString strLeft("MM-dd HH:mm | ");
            QString strRight = QString("%1℃").arg(m_hoverValue);
            QFontMetrics fm(painter->font());
            int strWidth = fm.width(strLeft + strRight) + 22;

            QRectF textRect(p.x() - strWidth / 2, p.y() - 50, strWidth, 40);
            if (textRect.top() < rect().top()) {
                textRect.moveTop(p.y() + 20);
            }
            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor("#b2b2b2"));
            painter->drawRoundedRect(textRect, 3, 3);

            QRectF textRealRect = textRect;
            textRealRect.setLeft(textRect.left() + 15);
            textRealRect.setRight(textRect.right() - 15);
            painter->setPen(Qt::white);
            QDateTime timesamp = QDateTime::fromTime_t(m_hoverTime);
            QString textStr = timesamp.toString("MM-dd HH:mm") + QString(" |");

            QFont boldFont = painter->font();
            boldFont.setBold(true);
            painter->setFont(boldFont);
            painter->drawText(textRealRect, Qt::AlignVCenter | Qt::AlignLeft, textStr);
            painter->setPen(Qt::white);
            painter->drawText(textRealRect, Qt::AlignVCenter | Qt::AlignRight, QString("%1℃").arg(m_hoverValue));
        }
    }

    painter->restore();
}
