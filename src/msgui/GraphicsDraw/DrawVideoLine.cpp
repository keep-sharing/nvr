#include "DrawVideoLine.h"
#include "MyDebug.h"
#include <QPainter>

#define DIRECTION_A_B 0
#define DIRECTION_B_A 1
#define DIRECTION_AB 2

DrawVideoLine::DrawVideoLine(QGraphicsItem *parent)
    : DrawVideoBase(parent)
{
}

DrawVideoLine::~DrawVideoLine()
{
    if (m_linecrossing_info) {
        delete m_linecrossing_info;
        m_linecrossing_info = nullptr;
    }
    if (m_linecrossing_info2) {
        delete m_linecrossing_info2;
        m_linecrossing_info2 = nullptr;
    }
    if (m_smart_event_people_cnt) {
        delete m_smart_event_people_cnt;
        m_smart_event_people_cnt = nullptr;
    }
}

void DrawVideoLine::setLineCrossInfo(ms_linecrossing_info *info)
{
    m_lineType = TypeLineCross;

    m_realWidth = info->width;
    m_realHeight = info->height;

    if (!m_linecrossing_info) {
        m_linecrossing_info = new ms_linecrossing_info;
    }
    memcpy(m_linecrossing_info, info, sizeof(ms_linecrossing_info));
    drawLineCrossInfo(m_linecrossing_info);
}

void DrawVideoLine::setLineCrossInfo2(ms_linecrossing_info2 *info)
{
    m_lineType = TypeLineCross2;

    m_realWidth = info->width;
    m_realHeight = info->height;

    if (!m_linecrossing_info2) {
        m_linecrossing_info2 = new ms_linecrossing_info2;
    }
    memcpy(m_linecrossing_info2, info, sizeof(ms_linecrossing_info2));
    if (m_linecrossing_info2->lineType == REGION_PRESET) {
        drawLineCrossInfo2(m_linecrossing_info2, m_linecrossing_info2->lineScene);
    } else {
        drawLineCrossInfo2(m_linecrossing_info2);
    }
}

/**
 * @brief DrawVideoLine::setLineCrossState
 * @param state
 * @param line: 1-4
 */
void DrawVideoLine::setLineCrossState(int state, int line)
{
    m_lineStatus[line - 1] = state;
    update();
}

void DrawVideoLine::setPeopleCountState(int state)
{
    for (int i = 0; i < 4; i++) {
        m_lineStatus[i] = state;
    }
    update();
}

void DrawVideoLine::clearState()
{
    memset(&m_lineStatus, 0, sizeof(m_lineStatus));
}

void DrawVideoLine::setRect(const QRectF &rect)
{
    DrawVideoBase::setRect(rect);
    switch (m_lineType) {
    case TypeLineCross:
        drawLineCrossInfo(m_linecrossing_info);
        break;
    case TypeLineCross2:
        if (m_linecrossing_info2->lineType == REGION_PRESET) {
            drawLineCrossInfo2(m_linecrossing_info2, m_linecrossing_info2->lineScene);
        } else {
            drawLineCrossInfo2(m_linecrossing_info2);
        }
        break;
    default:
        break;
    }
}

void DrawVideoLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setRenderHints(QPainter::Antialiasing);
    switch (m_lineType) {
    case TypeLineCross:
        break;
    case TypeLineCross2:
        for (auto iter = m_crossLineMap.constBegin(); iter != m_crossLineMap.constEnd(); ++iter) {
            int lineIndex = iter.key();
            const CrossLine &crossLine = iter.value();
            drawLine(painter, crossLine, m_lineStatus[lineIndex]);
        }
        break;
    default:
        break;
    }
}

void DrawVideoLine::drawLineCrossInfo(ms_linecrossing_info *info)
{
    m_crossLineMap.clear();
    for (int i = 0; i < 4; ++i) {
        const linecrossing_info &line = info->line[i];

        CrossLine crossLine;
        crossLine.index = i;
        crossLine.beginPoint = physicalPos(line.startX, line.startY);
        crossLine.endPoint = physicalPos(line.stopX, line.stopY);
        crossLine.direction = line.direction;
        if (crossLine.beginPoint.isNull() && crossLine.endPoint.isNull()) {
            crossLine.beginComplete = false;
            crossLine.endComplete = false;
        } else {
            crossLine.beginComplete = true;
            crossLine.endComplete = true;
        }
        m_crossLineMap.insert(i, crossLine);
    }

    update();
}

void DrawVideoLine::drawLineCrossInfo2(ms_linecrossing_info2 *info)
{
    m_crossLineMap.clear();
    for (int i = 0; i < 4; ++i) {
        const linecrossing_info2 &line = info->line[i];

        CrossLine crossLine;
        crossLine.index = i;
        crossLine.beginPoint = physicalPos(line.startX, line.startY);
        crossLine.endPoint = physicalPos(line.stopX, line.stopY);
        crossLine.direction = line.direction;
        if (crossLine.beginPoint.isNull() && crossLine.endPoint.isNull()) {
            crossLine.beginComplete = false;
            crossLine.endComplete = false;
        } else {
            crossLine.beginComplete = true;
            crossLine.endComplete = true;
        }
        m_crossLineMap.insert(i, crossLine);
    }

    update();
}

void DrawVideoLine::drawLineCrossInfo2(ms_linecrossing_info2 *info, int index)
{
    m_crossLineMap.clear();
    for (int i = 0; i < 4; ++i) {
        const MS_LINE &line = info->line[i].line[index];

        CrossLine crossLine;
        crossLine.index = i;
        crossLine.beginPoint = physicalPos(line.startX, line.startY);
        crossLine.endPoint = physicalPos(line.stopX, line.stopY);
        crossLine.direction = line.direction;
        if (crossLine.beginPoint.isNull() && crossLine.endPoint.isNull()) {
            crossLine.beginComplete = false;
            crossLine.endComplete = false;
        } else {
            crossLine.beginComplete = true;
            crossLine.endComplete = true;
        }
        m_crossLineMap.insert(i, crossLine);
    }

    update();
}

void DrawVideoLine::drawLine(QPainter *painter, const CrossLine &line, bool alarm)
{
    if (!line.endComplete) {
        return;
    }

    //主黄线
    painter->save();
    if (alarm) {
        painter->setPen(QPen(QColor("#FF0000"), 2));
    } else {
        painter->setPen(QPen(QColor("#FEFE00"), 2));
    }
    painter->drawLine(line.beginPoint, line.endPoint);
    //短垂直线
    QLineF mainLine(line.beginPoint, line.endPoint);
    QLineF shortLine;
    switch (line.direction) {
    case DIRECTION_A_B: {
        shortLine = QLineF(lineCenter(mainLine), line.beginPoint).normalVector();
        shortLine.setLength(30);
        painter->drawLine(shortLine);
        //三角，利用垂直线确定三个点
        QLineF halfTriangleLine = QLineF(shortLine.p2(), shortLine.p1()).normalVector();
        halfTriangleLine.setLength(10);
        QLineF triangleLine(halfTriangleLine.p2(), halfTriangleLine.p1());
        triangleLine.setLength(20);
        QLineF triangleLine2(shortLine);
        triangleLine2.setLength(40);
        QVector<QPointF> trianglePoint;
        trianglePoint.append(triangleLine.p1());
        trianglePoint.append(triangleLine.p2());
        trianglePoint.append(triangleLine2.p2());
        trianglePoint.append(triangleLine.p1());
        QPolygonF triangle(trianglePoint);
        painter->drawPolyline(triangle);
        break;
    }
    case DIRECTION_B_A: {
        shortLine = QLineF(lineCenter(mainLine), line.endPoint).normalVector();
        shortLine.setLength(30);
        painter->drawLine(shortLine);
        //三角，利用垂直线确定三个点
        QLineF halfTriangleLine = QLineF(shortLine.p2(), shortLine.p1()).normalVector();
        halfTriangleLine.setLength(10);
        QLineF triangleLine(halfTriangleLine.p2(), halfTriangleLine.p1());
        triangleLine.setLength(20);
        QLineF triangleLine2(shortLine);
        triangleLine2.setLength(40);
        QVector<QPointF> trianglePoint;
        trianglePoint.append(triangleLine.p1());
        trianglePoint.append(triangleLine.p2());
        trianglePoint.append(triangleLine2.p2());
        trianglePoint.append(triangleLine.p1());
        QPolygonF triangle(trianglePoint);
        painter->drawPolyline(triangle);
        break;
    }
    case DIRECTION_AB: {
        shortLine = QLineF(lineCenter(mainLine), line.beginPoint).normalVector();
        shortLine.setLength(30);
        shortLine = QLineF(shortLine.p2(), shortLine.p1());
        shortLine.setLength(60);
        painter->drawLine(shortLine);
        //三角，利用垂直线确定三个点
        {
            QLineF halfTriangleLine = QLineF(shortLine.p1(), lineCenter(shortLine)).normalVector();
            halfTriangleLine.setLength(10);
            QLineF triangleLine(halfTriangleLine.p2(), halfTriangleLine.p1());
            triangleLine.setLength(20);
            QLineF triangleLine2(lineCenter(shortLine), shortLine.p1());
            triangleLine2.setLength(40);
            QVector<QPointF> trianglePoint;
            trianglePoint.append(triangleLine.p1());
            trianglePoint.append(triangleLine.p2());
            trianglePoint.append(triangleLine2.p2());
            trianglePoint.append(triangleLine.p1());
            QPolygonF triangle(trianglePoint);
            painter->drawPolyline(triangle);
        }
        {
            QLineF halfTriangleLine = QLineF(shortLine.p2(), lineCenter(shortLine)).normalVector();
            halfTriangleLine.setLength(10);
            QLineF triangleLine(halfTriangleLine.p2(), halfTriangleLine.p1());
            triangleLine.setLength(20);
            QLineF triangleLine2(lineCenter(shortLine), shortLine.p2());
            triangleLine2.setLength(40);
            QVector<QPointF> trianglePoint;
            trianglePoint.append(triangleLine.p1());
            trianglePoint.append(triangleLine.p2());
            trianglePoint.append(triangleLine2.p2());
            trianglePoint.append(triangleLine.p1());
            QPolygonF triangle(trianglePoint);
            painter->drawPolyline(triangle);
        }
        break;
    }
    default:
        break;
    }
    painter->restore();

    //两端点
    painter->save();
    painter->setPen(QPen(QColor("#FF0000"), 8));
    painter->drawPoint(line.beginPoint);
    painter->drawPoint(line.endPoint);
    //文字
    QFont font = painter->font();
    font.setPixelSize(14);
    painter->setFont(font);
    if (line.showIndex) {
        painter->drawText(line.beginPoint.x() + 5, line.beginPoint.y() + 5, QString("#%1#").arg(line.index + 1));
    }
    //用垂直线确定AB文字位置
    QLineF textLine = QLineF(lineCenter(mainLine), line.endPoint);
    textLine.setLength(30);
    textLine = QLineF(textLine.p2(), textLine.p1()).normalVector();
    textLine = QLineF(textLine.p2(), textLine.p1());
    textLine.setLength(60);
    painter->drawText(textLine.p1(), "B");
    painter->drawText(textLine.p2(), "A");
    painter->restore();
}

QPointF DrawVideoLine::lineCenter(const QLineF &line)
{
    return 0.5 * line.p1() + 0.5 * line.p2();
}

QPoint DrawVideoLine::physicalPos(int x, int y)
{
    return QPoint((qreal)x / m_realWidth * rect().width(), (qreal)y / m_realHeight * rect().height());
}

QPoint DrawVideoLine::logicalPos(const QPoint &pos)
{
    return QPoint((qreal)pos.x() / rect().width() * m_realWidth, (qreal)pos.y() / rect().height() * m_realHeight);
}
