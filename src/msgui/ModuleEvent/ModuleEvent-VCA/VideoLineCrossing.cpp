#include "VideoLineCrossing.h"
#include <QMouseEvent>
#include <QPainter>
#include <QtDebug>

#define DIRECTION_A_B 0
#define DIRECTION_B_A 1
#define DIRECTION_AB 2

VideoLineCrossing::VideoLineCrossing(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
}

VideoLineCrossing::~VideoLineCrossing()
{
}

void VideoLineCrossing::setEnable(bool enable)
{
    m_enable = enable;
}

void VideoLineCrossing::clearLine(int index)
{
    m_tempCrossLine.clear();
    m_crossLineMap.insert(index, m_tempCrossLine);

    update();
}

void VideoLineCrossing::clearAll()
{
    QMap<int, CrossLine>::iterator iter = m_crossLineMap.begin();
    for (; iter != m_crossLineMap.end(); ++iter) {
        CrossLine &line = iter.value();
        line.clear();
    }

    m_tempCrossLine.clear();

    update();
}

void VideoLineCrossing::setCurrentLine(int index)
{
    m_currentLineIndex = index;
    m_tempCrossLine = m_crossLineMap.value(index);
    update();
}

void VideoLineCrossing::setLineDirection(int direction)
{
    m_tempCrossLine.direction = direction;
    update();
}

int VideoLineCrossing::lineDirection(int index)
{
    return m_crossLineMap.value(index).direction;
}

void VideoLineCrossing::saveCurrentLine()
{
    m_crossLineMap.insert(m_currentLineIndex, m_tempCrossLine);
}

void VideoLineCrossing::setLineCrossInfo(ms_linecrossing_info *info)
{
    m_isPeopleCount = false;
    m_realWidth = info->width;
    m_realHeight = info->height;

    m_crossLineMap.clear();
    for (int i = 0; i < 4; ++i) {
        const linecrossing_info &line = info->line[i];

        CrossLine crossLine;
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

void VideoLineCrossing::getLineCrossInfo(ms_linecrossing_info *info)
{
    for (int i = 0; i < 4; ++i) {
        const CrossLine &crossLine = m_crossLineMap.value(i);
        QPoint start = logicalPos(crossLine.beginPoint);
        QPoint end = logicalPos(crossLine.endPoint);

        linecrossing_info &line = info->line[i];
        if (crossLine.beginPoint.isNull() && crossLine.endPoint.isNull()) {
            line.startX = 0;
            line.startY = 0;
            line.stopX = 0;
            line.stopY = 0;
            line.direction = crossLine.direction;
        } else {
            line.startX = start.x();
            line.startY = start.y();
            line.stopX = end.x();
            line.stopY = end.y();
            line.direction = crossLine.direction;
        }
    }
}

void VideoLineCrossing::setLineCrossInfo(ms_linecrossing_info2 *info)
{
    m_isPeopleCount = false;
    m_realWidth = info->width;
    m_realHeight = info->height;

    m_crossLineMap.clear();
    for (int i = 0; i < 4; ++i) {
        const linecrossing_info2 &line = info->line[i];

        CrossLine crossLine;
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

void VideoLineCrossing::getLineCrossInfo(ms_linecrossing_info2 *info)
{
    for (int i = 0; i < 4; ++i) {
        const CrossLine &crossLine = m_crossLineMap.value(i);
        QPoint start = logicalPos(crossLine.beginPoint);
        QPoint end = logicalPos(crossLine.endPoint);

        linecrossing_info2 &line = info->line[i];
        if (crossLine.beginPoint.isNull() && crossLine.endPoint.isNull()) {
            line.startX = 0;
            line.startY = 0;
            line.stopX = 0;
            line.stopY = 0;
            line.direction = crossLine.direction;
        } else {
            line.startX = start.x();
            line.startY = start.y();
            line.stopX = end.x();
            line.stopY = end.y();
            line.direction = crossLine.direction;
        }
    }
}

void VideoLineCrossing::paintEvent(QPaintEvent *)
{
    const CrossLine &currentLine = m_tempCrossLine;
    if (!currentLine.endComplete) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    //主黄线
    painter.save();
    painter.setPen(QPen(QColor("#FEFE00"), 2));
    painter.drawLine(currentLine.beginPoint, currentLine.endPoint);
    //短垂直线
    QLineF mainLine(currentLine.beginPoint, currentLine.endPoint);
    QLineF shortLine;
    switch (currentLine.direction) {
    case DIRECTION_A_B: {
        shortLine = QLineF(lineCenter(mainLine), currentLine.beginPoint).normalVector();
        shortLine.setLength(30);
        painter.drawLine(shortLine);
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
        painter.drawPolyline(triangle);
        break;
    }
    case DIRECTION_B_A: {
        shortLine = QLineF(lineCenter(mainLine), currentLine.endPoint).normalVector();
        shortLine.setLength(30);
        painter.drawLine(shortLine);
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
        painter.drawPolyline(triangle);
        break;
    }
    case DIRECTION_AB: {
        shortLine = QLineF(lineCenter(mainLine), currentLine.beginPoint).normalVector();
        shortLine.setLength(30);
        shortLine = QLineF(shortLine.p2(), shortLine.p1());
        shortLine.setLength(60);
        painter.drawLine(shortLine);
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
            painter.drawPolyline(triangle);
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
            painter.drawPolyline(triangle);
        }
        break;
    }
    default:
        break;
    }
    painter.restore();

    //两端点
    painter.save();
    painter.setPen(QPen(QColor("#FF0000"), 8));
    painter.drawPoint(currentLine.beginPoint);
    painter.drawPoint(currentLine.endPoint);
    //文字
    QFont font = painter.font();
    font.setPixelSize(14);
    painter.setFont(font);
    if (currentLine.showIndex) {
        painter.drawText(currentLine.beginPoint.x() + 5, currentLine.beginPoint.y() + 5, QString("#%1#").arg(m_currentLineIndex + 1));
    }
    //用垂直线确定AB文字位置
    QLineF textLine = QLineF(lineCenter(mainLine), currentLine.endPoint);
    textLine.setLength(30);
    textLine = QLineF(textLine.p2(), textLine.p1()).normalVector();
    textLine = QLineF(textLine.p2(), textLine.p1());
    textLine.setLength(60);
    painter.drawText(textLine.p1(), "B");
    painter.drawText(textLine.p2(), "A");
    painter.restore();
}

void VideoLineCrossing::mousePressEvent(QMouseEvent *event)
{
    if (!m_enable) {
        if (event->button() == Qt::RightButton) {
            QWidget::mousePressEvent(event);
        }
        return;
    }

    m_isPressed = true;
    m_pressPoint = event->pos();

    if (m_tempCrossLine.endComplete) {
        if (m_tempCrossLine.beginRect().contains(event->pos())) {
            setCursor(Qt::ClosedHandCursor);
            m_currentMode = ModeResizeBegin;
        } else if (m_tempCrossLine.endRect().contains(event->pos())) {
            setCursor(Qt::ClosedHandCursor);
            m_currentMode = ModeResizeEnd;
        } else if (m_tempCrossLine.boundingRect().contains(event->pos())) {
            setCursor(Qt::SizeAllCursor);
            m_currentMode = ModeMove;
            m_tempBeginPoint = m_tempCrossLine.beginPoint;
            m_tempEndPoint = m_tempCrossLine.endPoint;
        } else {
            unsetCursor();
            m_currentMode = ModeNone;
        }
    } else {
        if (!m_tempCrossLine.beginComplete) {
            m_tempCrossLine.beginPoint = event->pos();
            m_tempCrossLine.beginComplete = true;
            setCursor(Qt::ClosedHandCursor);
            m_currentMode = ModeResizeEnd;
        } else if (!m_tempCrossLine.endComplete) {
            m_tempCrossLine.endPoint = event->pos();
            m_tempCrossLine.endComplete = true;
            setCursor(Qt::ClosedHandCursor);
            m_currentMode = ModeResizeEnd;
            update();
        }
    }
}

void VideoLineCrossing::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_enable) {
        return;
    }

    m_isPressed = false;

    if (m_tempCrossLine.beginRect().contains(event->pos())) {
        setCursor(Qt::OpenHandCursor);
    } else if (m_tempCrossLine.endRect().contains(event->pos())) {
        setCursor(Qt::OpenHandCursor);
    } else if (m_tempCrossLine.boundingRect().contains(event->pos())) {
        setCursor(Qt::SizeAllCursor);
    } else {
        unsetCursor();
    }
}

void VideoLineCrossing::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_enable) {
        return;
    }

    if (m_isPressed) {
        switch (m_currentMode) {
        case ModeMove: {
            m_tempCrossLine.beginPoint = checkPoint(m_tempBeginPoint + (event->pos() - m_pressPoint));
            m_tempCrossLine.endPoint = checkPoint(m_tempEndPoint + (event->pos() - m_pressPoint));
            break;
        }
        case ModeResizeBegin: {
            m_tempCrossLine.beginPoint = checkPoint(event->pos());
            break;
        }
        case ModeResizeEnd: {
            m_tempCrossLine.endPoint = checkPoint(event->pos());
            m_tempCrossLine.endComplete = true;
            break;
        }
        default:
            break;
        }

        update();
    } else {
        if (!m_tempCrossLine.endComplete) {
            return;
        }
        if (m_tempCrossLine.beginRect().contains(event->pos())) {
            setCursor(Qt::OpenHandCursor);
        } else if (m_tempCrossLine.endRect().contains(event->pos())) {
            setCursor(Qt::OpenHandCursor);
        } else if (m_tempCrossLine.boundingRect().contains(event->pos())) {
            setCursor(Qt::SizeAllCursor);
        } else {
            unsetCursor();
        }
    }
}

QPointF VideoLineCrossing::lineCenter(const QLineF &line)
{
    return 0.5 * line.p1() + 0.5 * line.p2();
}

QPoint VideoLineCrossing::physicalPos(int x, int y)
{
    return QPoint((qreal)x / m_realWidth * width(), (qreal)y / m_realHeight * height());
}

QPoint VideoLineCrossing::logicalPos(const QPoint &pos)
{
    return QPoint((qreal)pos.x() / width() * m_realWidth, (qreal)pos.y() / height() * m_realHeight);
}

QPoint VideoLineCrossing::checkPoint(const QPoint &point)
{
    QPoint tempPoint = point;
    if (tempPoint.x() < 0) {
        tempPoint.setX(0);
    }
    if (tempPoint.y() < 0) {
        tempPoint.setY(0);
    }
    if (tempPoint.x() > width()) {
        tempPoint.setX(width());
    }
    if (tempPoint.y() > height()) {
        tempPoint.setY(height());
    }
    return tempPoint;
}
