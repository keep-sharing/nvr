#include "GraphicsMultiRegionItem.h"
#include "GraphicsMultiRegionScene.h"
#include "MyDebug.h"
#include <QGraphicsTextItem>
#include <QPainter>
#include "DynamicDisplayData.h"

extern "C" {
#include "msg.h"
}

const qreal BaseWidth = 1024.0;
const qreal BaseHeight = 1024.0;

GraphicsMultiRegionItem::GraphicsMultiRegionItem(QGraphicsItem *parent)
    : DrawItem_Polygon(parent)
{
    m_textItem = new QGraphicsTextItem(this);
}

void GraphicsMultiRegionItem::updateRegionAlarm(const RegionalAlarmInfo &info)
{
    if (isEmpty()) {
        return;
    }
    if (!isFinished()) {
        return;
    }
    if (isEditting()) {
        return;
    }

    m_alarm = (info.frameAlarm & (0x01 << index())) | (info.stayAlarm & (0x01 << index()));
    showStayValue(info.num[index()], lineColor());
    update();
}

void GraphicsMultiRegionItem::showItem(MsIpcRegionalPeople *info, int index)
{
    setIndex(index);
    m_channel = info->chnid;

    QVector<QPointF> points;
    const auto &regional = info->regional[index];
    QStringList xs = QString(regional.polygonX).split(":", QString::SkipEmptyParts);
    QStringList ys = QString(regional.polygonY).split(":", QString::SkipEmptyParts);
    for (int i = 0; i < xs.size(); ++i) {
        int x = xs.at(i).toInt();
        int y = ys.at(i).toInt();
        if (x < 0 || y < 0) {
            break;
        }
        points.append(physicalPoint(QPoint(x, y)));
    }
    setPoints(points);

    setEditable(false);
    setSelected(false);
    if (!isEmpty()) {
        show();
    }
}

void GraphicsMultiRegionItem::beginEdit()
{
    DrawItem_Polygon::beginEdit();
    //
    if (isEmpty()) {
        return;
    }
    showStayValue(0, lineColor());
}

void GraphicsMultiRegionItem::endEdit()
{
    DrawItem_Polygon::endEdit();
}

QColor GraphicsMultiRegionItem::lineColor()
{
    if (!isAlarmable()) {
        return DrawItem_Polygon::lineColor();;
    }

    if (isFinished() && !isEditting() && m_alarm) {
        return QColor(255, 0, 0);
    } else {
        return DrawItem_Polygon::lineColor();
    }
}

void GraphicsMultiRegionItem::showText()
{
    if (isEmpty()) {
        m_textItem->setPlainText("");
        return;
    }
    if (!isFinished()) {
        m_textItem->setPlainText("");
        return;
    }
    //ipc数据变化才会推，这里主动获取缓存中的stay value
    int value = gDynamicDisplayData.regionStayValue(m_channel, index());
    showStayValue(value, lineColor());
    QRect textRect = m_textItem->boundingRect().toRect();

    auto vecPoint = points();
    QPolygonF polygon(vecPoint);
    QRegion region(polygon.toPolygon());
    QRect regionRect = region.boundingRect();

    //显示在左上角那个点的上方
    QMap<qreal, int> lengthMap;
    for (int i = 0; i < vecPoint.size(); ++i) {
        qreal length = QLineF(regionRect.topLeft(), vecPoint.at(i)).length();
        lengthMap.insert(length, i);
    }
    QPoint topLeftPoint = vecPoint.at(lengthMap.begin().value()).toPoint();
    textRect.moveBottomLeft(topLeftPoint);
    QPointF sp = mapToScene(textRect.topLeft());
    textRect.moveTopLeft(sp.toPoint());
    if (textRect.left() < scene()->sceneRect().left()) {
        textRect.moveLeft(scene()->sceneRect().left());
    }
    if (textRect.top() < scene()->sceneRect().top()) {
        textRect.moveTop(scene()->sceneRect().top());
    }
    if (textRect.right() > scene()->sceneRect().right()) {
        textRect.moveRight(scene()->sceneRect().right());
    }
    if (textRect.bottom() > scene()->sceneRect().bottom()) {
        textRect.moveBottom(scene()->sceneRect().bottom());
    }
    QPointF p = mapFromScene(textRect.topLeft());
    textRect.moveTopLeft(p.toPoint());

    //
    m_textItem->setPos(textRect.topLeft());
}

void GraphicsMultiRegionItem::clearText()
{
    m_textItem->setPlainText("");
}

QPointF GraphicsMultiRegionItem::physicalPoint(const QPoint &p) const
{
    qreal x = p.x() / BaseWidth * scene()->sceneRect().width();
    qreal y = p.y() / BaseHeight * scene()->sceneRect().height();
    return QPointF(x, y);
}

QPoint GraphicsMultiRegionItem::logicalPoint(const QPointF &p) const
{
    int x = qRound(p.x() / scene()->sceneRect().width() * BaseWidth);
    int y = qRound(p.y() / scene()->sceneRect().height() * BaseHeight);
    return QPoint(x, y);
}

void GraphicsMultiRegionItem::showStayValue(int value, QColor color)
{
    Q_UNUSED(color)

    m_stayValue = value;
    QString text;
    if (value < 0) {
        text = QString("Region%1").arg(index() + 1);
    } else {
        text = QString("Region%1 Stay: %2").arg(index() + 1).arg(m_stayValue);
    }
    QFont font(m_textItem->font().family(), 15, QFont::Bold);
    m_textItem->setFont(font);
    m_textItem->setDefaultTextColor(QColor(6, 174, 0));
    m_textItem->setPlainText(text);
    if (!m_textItem->isVisible()) {
        m_textItem->show();
    }
}

void GraphicsMultiRegionItem::hideStayValue()
{
    m_textItem->hide();
}

bool GraphicsMultiRegionItem::alarm() const
{
    return m_alarm;
}

void GraphicsMultiRegionItem::setAlarm(bool alarm)
{
    m_alarm = alarm;
    update();
}
