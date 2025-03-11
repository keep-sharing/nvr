#include "GraphicsDrawLineCrossingItem.h"
#include "MyDebug.h"
#include <QCursor>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>
GraphicsDrawLineCrossingItem::GraphicsDrawLineCrossingItem(QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
{
    setAcceptHoverEvents(true);
    setFiltersChildEvents(true);
    m_textIndexItem = new QGraphicsTextItem(this);
    m_textInfoItem = new QGraphicsTextItem(this);
}

int GraphicsDrawLineCrossingItem::index() const
{
    return m_index;
}

void GraphicsDrawLineCrossingItem::setIndex(int newIndex)
{
    m_index = newIndex;
}

int GraphicsDrawLineCrossingItem::direction() const
{
    return m_direction;
}

void GraphicsDrawLineCrossingItem::setDirection(int newDirection)
{
    m_direction = newDirection;
    update();
}

QPointF GraphicsDrawLineCrossingItem::endPoint() const
{
    return m_endPoint;
}

void GraphicsDrawLineCrossingItem::setEndPoint(QPointF newEndPoint)
{
    m_endPoint = newEndPoint;
    update();
    showText();
}

QPointF GraphicsDrawLineCrossingItem::beginPoint() const
{
    return m_beginPoint;
}

void GraphicsDrawLineCrossingItem::setBeginPoint(QPointF newBeginPoint)
{
    m_beginPoint = newBeginPoint;
    update();
}

bool GraphicsDrawLineCrossingItem::isComplete() const
{
    if (m_beginPoint.isNull() && m_endPoint.isNull()) {
        return false;
    }

    switch (m_currentMode) {
    case ModeDraw:
        return false;
    default:
        return true;
    }
}

void GraphicsDrawLineCrossingItem::clear()
{
    m_beginPoint = QPointF(0, 0);
    m_endPoint = QPointF(0, 0);
    m_currentMode = ModeNone;
    m_textIndexItem->hide();
    m_textInfoItem->hide();
    update();
}

void GraphicsDrawLineCrossingItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (!isComplete()) {
        return;
    }

    painter->setRenderHint(QPainter::Antialiasing);

    //主黄线
    QLineF mainLine(m_beginPoint, m_endPoint);
    painter->save();
    painter->setPen(QPen(m_lineColor, 2));
    painter->drawLine(mainLine);
    painter->restore();
    //短垂直线
    switch (m_direction) {
    case DIRECTION_A_B: {
        drawArrow(painter, lineCenter(mainLine), m_beginPoint);
        break;
    }
    case DIRECTION_B_A: {
        drawArrow(painter, lineCenter(mainLine), m_endPoint);
        break;
    }
    case DIRECTION_AB: {
        drawArrow(painter, lineCenter(mainLine), m_beginPoint);
        drawArrow(painter, lineCenter(mainLine), m_endPoint);
        break;
    }
    default:
        break;
    }
    //两端点
    painter->save();
    painter->setPen(QPen(m_pointColor, 8));
    painter->drawPoint(m_beginPoint);
    painter->drawPoint(m_endPoint);
    painter->restore();

    //AB Text
    painter->save();
    painter->setPen(QPen(m_textColor));
    QFont font = painter->font();
    font.setBold(true);
    font.setPixelSize(14);
    painter->setFont(font);

    //用垂直线确定AB文字位置
    QLineF textLine = QLineF(lineCenter(mainLine), m_endPoint);
    textLine.setLength(30);
    textLine = QLineF(textLine.p2(), textLine.p1()).normalVector();
    textLine = QLineF(textLine.p2(), textLine.p1());
    textLine.setLength(60);
    painter->drawText(textLine.p1(), "B");
    painter->drawText(textLine.p2(), "A");
    painter->restore();
}

void GraphicsDrawLineCrossingItem::showText()
{
    QFont font(m_textInfoItem->font().family(), 15, QFont::Bold);
    m_textIndexItem->setFont(font);
    m_textInfoItem->setFont(font);
    if (m_isShowIndex && isComplete()) {
        m_textIndexItem->setDefaultTextColor(m_textColor);
        m_textIndexItem->setPlainText(QString("#%1#").arg(m_index + 1));
        m_textIndexItem->show();
    } else {
        m_textIndexItem->setPlainText("");
    }

    //画出线条信息
    if (m_isShowLineInfo && isComplete()) {
        m_textInfoItem->setDefaultTextColor(m_lineInfoColor);
        m_textInfoItem->setPlainText(getLineInfoText());
        m_textInfoItem->show();
    } else {
        m_textInfoItem->setPlainText("");
    }

    //调整坐标，显示完整文字
    m_textIndexItem->setPos(removeTextRect(m_textIndexItem->boundingRect().toRect(), m_beginPoint).topLeft());
    m_textInfoItem->setPos(removeTextRect(m_textInfoItem->boundingRect().toRect(), m_endPoint).topLeft());
}

QRect GraphicsDrawLineCrossingItem::removeTextRect(QRect rect, QPointF point)
{
    rect.moveBottomLeft(point.toPoint());
    QPointF sp = mapToScene(rect.topLeft());
    rect.moveTopLeft(sp.toPoint());
    if (rect.left() < scene()->sceneRect().left()) {
        rect.moveLeft(scene()->sceneRect().left());
    }
    if (rect.top() < scene()->sceneRect().top()) {
        rect.moveTop(scene()->sceneRect().top());
    }
    if (rect.right() > scene()->sceneRect().right()) {
        rect.moveRight(scene()->sceneRect().right());
    }
    if (rect.bottom() > scene()->sceneRect().bottom()) {
        rect.moveBottom(scene()->sceneRect().bottom());
    }
    QPointF p = mapFromScene(rect.topLeft());
    rect.moveTopLeft(p.toPoint());
    return rect;
}

void GraphicsDrawLineCrossingItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (isComplete()) {
        if (beginRect().contains(event->scenePos())) {
            setCursor(Qt::OpenHandCursor);
        } else if (endRect().contains(event->scenePos())) {
            setCursor(Qt::OpenHandCursor);
        } else if (contentRect().contains(event->scenePos())) {
            setCursor(Qt::SizeAllCursor);
        } else {
            unsetCursor();
        }
    } else {
    }
}

void GraphicsDrawLineCrossingItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!isEnabled()) {
        return;
    }

    if (event->button() == Qt::LeftButton) {
        m_pressPoint = event->scenePos();
        if (isComplete()) {
            if (beginRect().contains(event->scenePos())) {
                setCursor(Qt::ClosedHandCursor);
                m_currentMode = ModeResizeBegin;
            } else if (endRect().contains(event->scenePos())) {
                setCursor(Qt::ClosedHandCursor);
                m_currentMode = ModeResizeEnd;
            } else if (contentRect().contains(event->scenePos())) {
                setCursor(Qt::SizeAllCursor);
                m_currentMode = ModeMove;
                m_tempBeginPoint = m_beginPoint;
                m_tempEndPoint = m_endPoint;
                showText();
            } else {
                unsetCursor();
                m_currentMode = ModeNone;
            }
        } else {
            m_beginPoint = event->scenePos();
            setCursor(Qt::ClosedHandCursor);
            m_currentMode = ModeDraw;
        }
    }
}

void GraphicsDrawLineCrossingItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!isEnabled()) {
        return;
    }

    switch (m_currentMode) {
    case ModeDraw: {
        m_endPoint = event->scenePos();
        m_currentMode = ModeResizeEnd;
        break;
    }
    case ModeMove: {
        m_beginPoint = m_tempBeginPoint + (event->scenePos() - m_pressPoint);
        m_endPoint = m_tempEndPoint + (event->scenePos() - m_pressPoint);
        break;
    }
    case ModeResizeBegin: {
        m_beginPoint = event->scenePos();
        break;
    }
    case ModeResizeEnd: {
        m_endPoint = event->scenePos();
        break;
    }
    default:
        return;
    }

    correctScenePos(m_beginPoint);
    correctScenePos(m_endPoint);
    update();
    showText();
}

void GraphicsDrawLineCrossingItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
}

void GraphicsDrawLineCrossingItem::correctScenePos(QPointF &p)
{
    if (scene()) {
        QRectF sceneRc = scene()->sceneRect();
        if (p.x() < sceneRc.left()) {
            p.setX(sceneRc.left());
        }
        if (p.x() > sceneRc.right()) {
            p.setX(sceneRc.right());
        }
        if (p.y() < sceneRc.top()) {
            p.setY(sceneRc.top());
        }
        if (p.y() > sceneRc.bottom()) {
            p.setY(sceneRc.bottom());
        }
    }
}

QPointF GraphicsDrawLineCrossingItem::lineCenter(const QLineF &line)
{
    return 0.5 * line.p1() + 0.5 * line.p2();
}

QRectF GraphicsDrawLineCrossingItem::beginRect() const
{
    QRectF rc(0, 0, 8, 8);
    rc.moveCenter(m_beginPoint);
    return rc;
}

QRectF GraphicsDrawLineCrossingItem::endRect() const
{
    QRectF rc(0, 0, 8, 8);
    rc.moveCenter(m_endPoint);
    return rc;
}

QRectF GraphicsDrawLineCrossingItem::contentRect() const
{
    QRectF rc(m_beginPoint, m_endPoint);
    if (rc.width() < 3) {
        rc.adjust(-2, 0, 2, 0);
    }
    if (rc.height() < 3) {
        rc.adjust(0, -2, 0, 2);
    }
    return rc;
}

void GraphicsDrawLineCrossingItem::drawArrow(QPainter *painter, QPointF p1, QPointF p2)
{
    painter->save();
    painter->setPen(QPen(m_lineColor, 2));

    QLineF shortLine = QLineF(p1, p2).normalVector();
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

    painter->restore();
}

void GraphicsDrawLineCrossingItem::setOsdType(int osdType)
{
    m_osdType = osdType;
    showText();
}

void GraphicsDrawLineCrossingItem::setAlarmState(int alarm)
{
    if (alarm) {
        m_lineColor = QColor(255, 0, 0);
    } else {
        m_lineColor = QColor(254, 254, 0);
    }
    update();
}

bool GraphicsDrawLineCrossingItem::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    if (!isEnabled()) {
        return false;
    }
    switch (event->type()) {
    case QEvent::GraphicsSceneHoverMove: {
        if (isComplete()) {
            QGraphicsSceneHoverEvent *hoverEvent = static_cast<QGraphicsSceneHoverEvent *>(event);
            hoverMoveEvent(hoverEvent);
        }
        break;
    }
    default:
        break;
    }
    return QGraphicsRectItem::sceneEventFilter(watched, event);
}

void GraphicsDrawLineCrossingItem::setIsShowLineInfo(bool isShowLineInfo)
{
    m_isShowLineInfo = isShowLineInfo;
    showText();
}

void GraphicsDrawLineCrossingItem::setLineInfo(int in, int out, int sum, int capacity)
{
    m_peopleCntInfo.in = in;
    m_peopleCntInfo.out = out;
    m_peopleCntInfo.sum = sum;
    m_peopleCntInfo.capacity = capacity;
    showText();
}

QString GraphicsDrawLineCrossingItem::getLineInfoText()
{
    QString lineInfo = "";
    if (m_osdType & 1 << 0) {
        lineInfo += QString("In %1 ").arg(m_peopleCntInfo.in);
    }
    if (m_osdType & 1 << 1) {
        lineInfo += QString("Out %1 ").arg(m_peopleCntInfo.out);
    }
    if (m_osdType & 1 << 3) {
        lineInfo += QString("Sum %1 ").arg(m_peopleCntInfo.sum);
    }
    if (m_osdType & 1 << 2) {
        lineInfo += QString("Capacity %1 ").arg(m_peopleCntInfo.capacity);
    }
    return lineInfo;
}

void GraphicsDrawLineCrossingItem::setIsShowIndex(bool isShowIndex)
{
    m_isShowIndex = isShowIndex;
    showText();
}
