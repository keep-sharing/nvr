#include "DrawItemTimeHeatMap.h"
#include "MsDevice.h"
#include "centralmessage.h"
#include <QElapsedTimer>
#include <QFile>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <QtDebug>
#include <qmath.h>

DrawItemTimeHeatMap::DrawItemTimeHeatMap(QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
{
    setAcceptHoverEvents(true);
}

void DrawItemTimeHeatMap::showHeatMap(const QString &text, int reportType, const QDateTime &dateTime)
{
    m_reportType = reportType;
    m_dateTime = dateTime;
    m_values.clear();

    QRegExp rx(R"("max":(\d+),"min":(\d+),"data":\[(.*)\])");
    if (rx.indexIn(text) != -1) {
        QString strMax = rx.cap(1);
        QString strMin = rx.cap(2);
        QString strData = rx.cap(3);

        m_max = strMax.toInt();
        m_min = strMin.toInt();

        QStringList strDataList = strData.split(",");
        for (int i = 0; i < strDataList.size(); ++i) {
            m_values.append(strDataList.at(i).toInt());
        }
    }

    update();
}

bool DrawItemTimeHeatMap::saveTimeHeatMap(const QString &filePath)
{
    qDebug() << "----DrawTimeHeatMapItem::saveTimeHeatMap----";
    qDebug() << "----filePath:" << filePath;

    return saveTimeHeatMap(m_values, m_reportType, m_dateTime, filePath);
}

bool DrawItemTimeHeatMap::saveTimeHeatMap(const QString &text, int reportType, const QDateTime &dateTime, const QString &fileName)
{
    QList<int> valuesList;
    QRegExp rx(R"("max":(\d+),"min":(\d+),"data":\[(.*)\])");
    if (rx.indexIn(text) != -1) {
        QString strData = rx.cap(3);
        QStringList strDataList = strData.split(",");
        for (int i = 0; i < strDataList.size(); ++i) {
            valuesList.append(strDataList.at(i).toInt());
        }
    }
    return saveTimeHeatMap(valuesList, reportType, dateTime, fileName);
}

bool DrawItemTimeHeatMap::saveTimeHeatMap(const QList<int> &valuesList, int reportType, const QDateTime &dateTime, const QString &fileName)
{
    QElapsedTimer timer;
    timer.start();

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        qWarning() << "----error:" << file.errorString();
        return false;
    }
    QDateTime startDateTime = dateTime;
    startDateTime.setTime(QTime(dateTime.time().hour(), 0, 0));
    QDateTime endDateTime;
    QTextStream ts(&file);
    ts << QString("StartTime,EndTime,Value(s)\r\n");
    for (int i = 0; i < valuesList.size(); ++i) {
        switch (reportType) {
        case DailyReport:
            //2020-03-12 02:00:00,2020-03-12 02:59:59,0
            if (i > 0) {
                startDateTime = startDateTime.addSecs(3600);
            }
            endDateTime = startDateTime;
            endDateTime.setTime(QTime(startDateTime.time().hour(), 59, 59));
            break;
        case WeeklyReport:
            //2020-02-01 00:00:00,2020-02-01 23:59:59,0
            if (i > 0) {
                startDateTime = startDateTime.addDays(1);
            }
            endDateTime = startDateTime.addDays(1);
            endDateTime = endDateTime.addSecs(-60);
            break;
        case MonthlyReport:
            //2020-02-01 00:00:00,2020-02-01 23:59:59,0
            if (i > 0) {
                startDateTime = startDateTime.addDays(1);
            }
            endDateTime = startDateTime.addDays(1);
            endDateTime = endDateTime.addSecs(-60);
            break;
        case AnnualReport:
            //2020-03-12 02:00:00,2020-03-31 23:59:59,0
            if (i > 0) {
                startDateTime = startDateTime.addMonths(1);
                startDateTime = QDateTime(QDate(startDateTime.date().year(), startDateTime.date().month(), 1), QTime(0, 0, 0));
            }
            endDateTime = QDateTime(QDate(startDateTime.date().year(), startDateTime.date().month(), startDateTime.date().daysInMonth()), QTime(23, 59, 59));
            break;
        }
        ts << QString("%1,%2,%3\r\n").arg(startDateTime.toString("yyyy-MM-dd HH:mm:ss")).arg(endDateTime.toString("yyyy-MM-dd HH:mm:ss")).arg(valuesList.at(i));
    }
    file.close();
    qDebug() << "----save file took" << timer.elapsed() << "ms";

    gMsMessage.syncFile();
    return true;
}

void DrawItemTimeHeatMap::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    drawGrid(painter);
    drawLine(painter);
}

void DrawItemTimeHeatMap::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
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

void DrawItemTimeHeatMap::drawGrid(QPainter *painter)
{
    painter->save();

    int days = m_max / 86400;
    int hours = m_max % 86400 / 3600;
    int minutes = m_max % 3600 / 60;
    //int seconds = m_max % 60;
    if (days > 10) {
        m_yScaleBase = yDay;
        m_yMax = qCeil(days / 10.0) * 10;
    } else if (days > 0 || hours > 10) {
        m_yScaleBase = yHour;
        m_yMax = qCeil(m_max / 3600.0 / 10.0) * 10;
    } else if (hours > 0 || minutes > 10) {
        m_yScaleBase = yMinute;
        m_yMax = qCeil(m_max / 60.0 / 10.0) * 10;
    } else {
        m_yScaleBase = ySecond;
        m_yMax = qCeil(m_max / 10.0) * 10;
    }
    if (m_yMax == 0) {
        m_yMax = 10;
    }

    m_left = rect().center().x() - rect().height() / 2;
    m_right = rect().center().x() + rect().height() / 2;
    m_top = rect().top() + 10;
    m_bottom = rect().bottom() - 30;

    int yCount = 11;
    qreal yStep = qreal(m_bottom - m_top) / (yCount - 1);
    qreal y = m_top;
    for (int i = 0; i < yCount; ++i) {
        if (i == 0 || i == yCount - 1) {
            painter->setPen(QPen(QColor("#999999"), 2));
        } else {
            painter->setPen(QPen(QColor("#CCCCCC"), 1));
        }
        painter->drawLine(QPointF(m_left, y), QPointF(m_right, y));

        //
        QString text;
        switch (m_yScaleBase) {
        case yDay:
            text = QString("%1 d.").arg((yCount - 1 - i) * (m_yMax / 10));
            break;
        case yHour:
            text = QString("%1 hr.").arg((yCount - 1 - i) * (m_yMax / 10));
            break;
        case yMinute:
            text = QString("%1 min.").arg((yCount - 1 - i) * (m_yMax / 10));
            break;
        case ySecond:
            text = QString("%1 sec.").arg((yCount - 1 - i) * (m_yMax / 10));
            break;
        }
        QRectF textRect;
        textRect.setLeft(rect().left());
        textRect.setRight(m_left - 10);
        textRect.setTop(y - 10);
        textRect.setBottom(y + 10);
        painter->setPen(QPen(QColor("#575757")));
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignRight, text);

        //
        y += yStep;
    }

    int xCount = 24;
    int xStart = 0;
    switch (m_reportType) {
    case DailyReport:
        xCount = 24;
        xStart = m_dateTime.time().hour();
        break;
    case WeeklyReport:
        xCount = 7;
        xStart = m_dateTime.date().dayOfWeek();
        break;
    case MonthlyReport:
        xCount = m_dateTime.date().daysInMonth();
        xStart = m_dateTime.date().day();
        break;
    case AnnualReport:
        xCount = 12;
        xStart = m_dateTime.date().month();
        break;
    }
    m_xStep = qreal(m_right - m_left) / (xCount - 1);
    qreal x = m_left;
    int xValue = xStart;
    for (int i = 0; i < xCount; ++i) {
        if (i == 0 || i == xCount - 1) {
            painter->setPen(QPen(QColor("#999999"), 2));
        } else {
            painter->setPen(QPen(QColor("#CCCCCC"), 1));
        }
        painter->drawLine(QPointF(x, m_top), QPointF(x, m_bottom));

        //
        QString text;
        switch (m_reportType) {
        case DailyReport:
            if (xValue == 0) {
                text = QString("24");
            } else {
                text = QString("%1").arg(xValue);
            }
            break;
        case WeeklyReport:
            text = intoWeek(xValue);
            break;
        case MonthlyReport:
            text = QString("%1").arg(xValue);
            break;
        case AnnualReport:
            text = QString("%1").arg(xValue);
            break;
        }
        xValue++;
        if (xValue > xCount) {
            xValue = 1;
        }
        QRectF textRect;
        textRect.setLeft(x - 17);
        textRect.setRight(x + 17);
        textRect.setTop(m_bottom + 10);
        textRect.setBottom(m_bottom + 20);
        painter->setPen(QPen(QColor("#575757")));
        painter->drawText(textRect, Qt::AlignCenter, text);

        //
        x += m_xStep;
    }

    painter->restore();
}

void DrawItemTimeHeatMap::drawLine(QPainter *painter)
{
    if (m_values.isEmpty()) {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    m_valueRects.clear();

    QVector<QPointF> points;
    qreal x = m_left;
    qreal y = 0;
    for (int i = 0; i < m_values.size(); ++i) {
        int seconds = m_values.at(i);

        qreal yValue = 0;
        switch (m_yScaleBase) {
        case yDay:
            yValue = seconds / 86400.0;
            break;
        case yHour:
            yValue = seconds / 3600.0;
            break;
        case yMinute:
            yValue = seconds / 60.0;
            break;
        case ySecond:
            yValue = seconds;
            break;
        }

        if (i == m_hoverIndex) {
            m_hoverValue = yValue;
        }

        if (m_yMax == 0) {
            y = m_bottom;
        } else {
            y = m_bottom - (qreal)(m_bottom - m_top) / m_yMax * yValue;
        }
        points.append(QPointF(x, y));
        m_valueRects.append(QRectF(x - 10, y - 10, 20, 20));
        x += m_xStep;
    }
    painter->setPen(QPen(QColor("#086ABB"), 2));
    painter->drawPolyline(points);

    //
    for (int i = 0; i < points.size(); ++i) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QBrush(QColor("#086ABB")));

        const QPointF &p = points.at(i);
        painter->drawEllipse(p, 4, 4);

        if (i == m_hoverIndex) {
            QColor color("#086ABB");
            color.setAlpha(120);
            painter->setBrush(QBrush(color));
            painter->drawEllipse(p, 10, 10);

            QString strLeft("heat value");
            QString strRight = QString("%1").arg(m_hoverValue, 0, 'f', 2);
            QFontMetrics fm(painter->font());
            int strWidth = fm.width(strLeft + strRight) + 6;

            QRectF textRect(p.x() - strWidth / 2, p.y() - 35, strWidth, 20);
            if (textRect.top() < rect().top()) {
                textRect.moveTop(p.y() + 15);
            }
            painter->setPen(QColor(0, 0, 0, 60));
            painter->setBrush(QColor(0, 0, 0, 40));
            painter->drawRect(textRect);

            QRectF textRealRect = textRect;
            textRealRect.setLeft(textRect.left() + 2);
            textRealRect.setRight(textRect.right() - 2);
            painter->setPen(Qt::red);
            painter->drawText(textRealRect, Qt::AlignVCenter | Qt::AlignLeft, "heat value");
            painter->setPen(QColor("#575757"));
            painter->drawText(textRealRect, Qt::AlignVCenter | Qt::AlignRight, QString("%1").arg(m_hoverValue, 0, 'f', 2));
        }
    }

    painter->restore();
}

QString DrawItemTimeHeatMap::intoWeek(int day)
{
    QString week;
    switch (day) {
    case 1:
        week = "Mon";
        break;
    case 2:
        week = "Tue";
        break;
    case 3:
        week = "Wed";
        break;
    case 4:
        week = "Thu";
        break;
    case 5:
        week = "Fri";
        break;
    case 6:
        week = "Sat";
        break;
    case 7:
        week = "Sun";
        break;
    default:
        week = QString("%1").arg(day);
        break;
    }
    return week;
}
