#include "LegendItem.h"
#include "BaseChartScene.h"
#include <QFontMetrics>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include "MsColor.h"

LegendItem::LegendItem(QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
{
    setAcceptHoverEvents(true);
}

void LegendItem::setData(int channel, const QString &text, bool checked, const QImage &image)
{
    m_channel = channel;
    m_text = text;
    m_checked = checked;
    if (m_channel == -1) {
        m_image = QImage(QString(":/checkbox/checkbox/unchecked0.png"));
    } else {
        m_image = image;
    }

    //TODO: LiuHuanyu 2021-07-16, about show ...
#if 1
    QFontMetrics fm(scene()->font());
    m_displayText = fm.elidedText(m_text, Qt::ElideRight, rect().width() - rect().height() - 10);
#else
    if (m_text.size() > 10) {
        m_displayText = QString("%1...").arg(m_text.left(10));
    } else {
        m_displayText = m_text;
    }
#endif
}

void LegendItem::setRect(const QRectF &rect)
{
    QGraphicsRectItem::setRect(rect);
}

void LegendItem::setToolTip(ToolTipItem *toolTip)
{
    m_toolTip = toolTip;
    m_toolTip->setText(m_text);
}

void LegendItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QRectF imageRc = rect();
    imageRc.setRight(rect().left() + rect().height());
    painter->drawImage(imageRc.toRect(), m_image);

    QRectF textRc = rect();
    textRc.setLeft(imageRc.right() + 10);
    painter->setPen(QPen(gMsColor.settings.text));
    painter->drawText(textRc, Qt::AlignVCenter | Qt::AlignLeft, m_displayText);
    painter->restore();
}

void LegendItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)

    if (m_channel == -1) {
        return;
    }
    if (!checkable()) {
        return;
    }
    BaseChartScene *chartScene = static_cast<BaseChartScene *>(scene());
    chartScene->setChannelVisible(m_channel, !m_checked);
}

void LegendItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    QPointF p = event->scenePos();
    p.setY(p.y() + 10);
    if (p.x() + m_toolTip->rect().width() + 10 > scene()->sceneRect().right()) {
        p.setX(scene()->sceneRect().right() - m_toolTip->rect().width() - 10);
    }
    m_toolTip->setPos(p);
    m_toolTip->show();
}

void LegendItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)

    m_toolTip->hide();
}

bool LegendItem::checkable() const
{
    return m_checkable;
}

void LegendItem::setCheckable(bool newCheckable)
{
    m_checkable = newCheckable;
}

/**
 * @brief ToolTipItem::ToolTipItem
 * @param parent
 */
ToolTipItem::ToolTipItem(QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
{
}

void ToolTipItem::setText(const QString &text)
{
    m_text = text;
    QFontMetrics metrics(scene()->font());
    int width = metrics.width(m_text) + 4;
    setRect(0, 0, width, 20);
}

void ToolTipItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();
    painter->setPen(QPen(QColor(1, 2, 0), 1));
    painter->setBrush(QColor(254, 254, 218));
    painter->drawRect(rect());
    painter->restore();
    painter->save();
    QRectF textRc = rect();
    textRc.setLeft(rect().left() + 2);
    textRc.setRight(rect().right() - 2);
    painter->setPen(QPen(gMsColor.settings.text));
    painter->drawText(textRc, Qt::AlignCenter, m_text);
    painter->restore();
}
