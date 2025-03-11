#include "DrawMultiRegionItem.h"
#include "DynamicDisplayData.h"
#include "GraphicsMultiRegionScene.h"
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
DrawMultiRegionItem::DrawMultiRegionItem(QGraphicsItem *parent)
    : GraphicsMultiRegionItem(parent)
{
}

void DrawMultiRegionItem::updateRegionAlarm(const RegionalAlarmInfo &info)
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

void DrawMultiRegionItem::beginEdit()
{
    DrawItem_Polygon::beginEdit();
    //
    if (isEmpty()) {
        return;
    }
    showStayValue(0, lineColor());
}

void DrawMultiRegionItem::showText()
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

void DrawMultiRegionItem::showStayValue(int value, QColor color)
{
    Q_UNUSED(value)
    Q_UNUSED(color)

    QString text;
    text = QString("Region%1").arg(index() + 1);
    QFont font(m_textItem->font().family(), 15, QFont::Bold);
    m_textItem->setFont(font);
    m_textItem->setDefaultTextColor(QColor(6, 174, 0));
    m_textItem->setPlainText(text);
    if (!m_textItem->isVisible()) {
        m_textItem->show();
    }
    if (!m_isShowText) {
        m_textItem->hide();
    }
}

void DrawMultiRegionItem::setIsShowText(bool isShowText)
{
    m_isShowText = isShowText;
    m_textItem->setVisible(isShowText);
}
