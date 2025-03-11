#include "LineScene.h"
#include "LegendItem.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "pointlineitem.h"
#include <QPainter>
#include <qmath.h>

LineScene::LineScene(QObject *parent)
    : BaseChartScene(parent)
{
}

void LineScene::showLine(const QList<int> &channels, int groupFilter)
{
    clear();

    m_channels = channels;
    m_groupFilter = groupFilter;

    m_colorMap.clear();
    int colorIndex = -1;
    for (int i = 0; i < channels.size(); ++i) {
        int channel = channels.at(i);
        m_colorMap.insert(channel, gPeopleCountingData.color(colorIndex));
        colorIndex++;
    }

    showLegends(channels);

    m_gridData = gPeopleCountingData.peopleLineGridData(m_groupFilter);
    const PeopleLineData &lineData = gPeopleCountingData.peopleLineData(m_groupFilter);

    QRectF gridRc = gridRect();
    m_pointsMap.clear();
    for (int i = 0; i < lineData.lines.size(); ++i) {
        int lineIndex = lineData.lines.at(i);
        if (!isChannelVisible(lineIndex)) {
            continue;
        }
        const QMap<int, int> &valueMap = lineData.lineData.value(lineIndex);
        //color
        QColor color = m_colorMap.value(lineIndex);
        //line
        QVector<QPointF> &points = m_pointsMap[lineIndex];
        for (int j = 0; j < m_gridData.xLineCount; ++j) {
            qreal xValue = j;
            qreal yValue = valueMap.value(j, 0);
            qreal x = xValue / m_gridData.xMaxValue * gridRc.width() + gridRc.left();
            qreal y = gridRc.bottom() - yValue / m_gridData.yMaxValue * gridRc.height();
            points.append(QPointF(x, y));
            //
            PointLineItem *item = new PointLineItem(color, yValue);
            addItem(item);
            QRectF rc(0, 0, 9, 9);
            rc.moveCenter(QPointF(x, y));
            item->setRect(rc);
        }
        //超过10个通道时仅显示total, 11 = 10 channel + total
        if (m_channels.size() > 11) {
            break;
        }
    }

    update();
}

void LineScene::setChannelVisible(int channel, bool visible)
{
    if (m_currentGroup < 0) {
        return;
    }
    m_visibleChannels[m_currentGroup][channel] = visible;
    showLine(m_channels, m_groupFilter);
}

void LineScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect)

    //MsDebug() << "scene rect:" << sceneRect() << ", update rect:" << rect;

    //background
    drawBackgroundColor(painter);

    //title
    drawTitle(painter);

    //
    drawYAxis(painter, sceneRect());
    drawXAxis(painter, sceneRect());

    //
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);
    for (auto iter = m_pointsMap.constBegin(); iter != m_pointsMap.constEnd(); ++iter) {
        int lineIndex = iter.key();
        const QVector<QPointF> &points = iter.value();
        QColor color = m_colorMap.value(lineIndex);
#if 0
        painter->setPen(QPen(color, 3));
        painter->drawPolyline(points);
#else
        int alpha1 = 200;
        int alpha2 = 50;
        int width = 2;
        qreal offset1 = 0.5;
        qreal offset2 = 1;
        for (int i = 0; i < points.size() - 1; ++i) {
            QPointF p1 = points.at(i);
            QPointF p2 = points.at(i + 1);
            QLineF line(p1, p2);
            qreal k = qAbs(line.dy() / line.dx());
            if (k >= 0 && k < 1) {
                painter->setPen(QPen(color, width));
                painter->drawLine(p1, p2);

                painter->setPen(QPen(QColor(color.red(), color.green(), color.blue(), alpha1), width));
                painter->drawLine(QPoint(p1.x(), p1.y() - offset1), QPoint(p2.x(), p2.y() - offset1));
                painter->setPen(QPen(QColor(color.red(), color.green(), color.blue(), alpha1), width));
                painter->drawLine(QPoint(p1.x(), p1.y() + offset1), QPoint(p2.x(), p2.y() + offset1));

                painter->setPen(QPen(QColor(color.red(), color.green(), color.blue(), alpha2), width));
                painter->drawLine(QPoint(p1.x(), p1.y() - offset2), QPoint(p2.x(), p2.y() - offset2));
                painter->setPen(QPen(QColor(color.red(), color.green(), color.blue(), alpha2), width));
                painter->drawLine(QPoint(p1.x(), p1.y() + offset2), QPoint(p2.x(), p2.y() + offset2));
            } else if (k >= 1) {
                painter->setPen(QPen(color, width));
                painter->drawLine(p1, p2);

                painter->setPen(QPen(QColor(color.red(), color.green(), color.blue(), alpha1), width));
                painter->drawLine(QPoint(p1.x() - offset1, p1.y()), QPoint(p2.x() - offset1, p2.y()));
                painter->setPen(QPen(QColor(color.red(), color.green(), color.blue(), alpha1), width));
                painter->drawLine(QPoint(p1.x() + offset1, p1.y()), QPoint(p2.x() + offset1, p2.y()));

                painter->setPen(QPen(QColor(color.red(), color.green(), color.blue(), alpha2), width));
                painter->drawLine(QPoint(p1.x() - offset2, p1.y()), QPoint(p2.x() - offset2, p2.y()));
                painter->setPen(QPen(QColor(color.red(), color.green(), color.blue(), alpha2), width));
                painter->drawLine(QPoint(p1.x() + offset2, p1.y()), QPoint(p2.x() + offset2, p2.y()));
            }
        }
#endif
        //超过10个通道时仅显示total
        if (m_channels.size() > 11) {
            break;
        }
    }
    painter->restore();
}
