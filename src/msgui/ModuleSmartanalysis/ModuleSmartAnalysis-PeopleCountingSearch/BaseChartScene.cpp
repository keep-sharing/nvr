#include "BaseChartScene.h"
#include "LegendItem.h"
#include "MsColor.h"
#include "MsLanguage.h"
#include "MsDevice.h"
#include <QPainter>

BaseChartScene::BaseChartScene(QObject *parent)
    : QGraphicsScene(parent)
{
    connect(this, SIGNAL(sceneRectChanged(QRectF)), this, SLOT(onSceneRectChanged(QRectF)));
}

void BaseChartScene::setMainTitle(const QString &text)
{
    m_mainTitle = text;
}

void BaseChartScene::setSubTitle(const QString &text)
{
    m_subTitle = text;
}

void BaseChartScene::setCurrentGroup(int group)
{
    m_currentGroup = group;
}

void BaseChartScene::clearChannelVisible()
{
    m_visibleChannels.clear();
}

void BaseChartScene::setChannelVisible(int channel, bool visible)
{
    Q_UNUSED(channel)
    Q_UNUSED(visible)
}

bool BaseChartScene::isChannelVisible(int channel)
{
    if (!m_visibleChannels.contains(m_currentGroup)) {
        return true;
    }
    return m_visibleChannels.value(m_currentGroup).value(channel, true);
}

QString BaseChartScene::legendName(int channel)
{
    QString text;
    if (channel == TOTAL_LINE_INDEX) {
        text = GET_TEXT("OCCUPANCY/74213", "Total");
    } else {
        text = qMsNvr->channelName(channel);
    }
    return text;
}

void BaseChartScene::showLegends(const QList<int> &channels)
{
    int left = legendRect().left() + 20;
    int y = legendRect().top();
    int itemWidth = legendRect().width() - 20;
    int itemHeight = 20;
    int yMargin = 10;
    for (int i = 0; i < channels.size(); ++i) {
        int channel = channels.at(i);
        QString channelText = legendName(channel);
        bool checked = isChannelVisible(channel);
        QImage image;
        if (channels.size() > 1 && checked) {
            image = QImage(QString(":/checkbox/checkbox/checked%1.png").arg(i));
        } else {
            image = QImage(QString(":/checkbox/checkbox/unchecked%1.png").arg(i));
        }
        LegendItem *item = new LegendItem;
        addItem(item);
        item->setRect(QRectF(left, y, itemWidth, itemHeight));
        item->setData(channel, channelText, checked, image);
        item->setCheckable(channels.size() > 1);
        //
        ToolTipItem *toolTip = new ToolTipItem;
        addItem(toolTip);
        toolTip->setZValue(1);
        toolTip->hide();
        item->setToolTip(toolTip);
        //
        y += itemHeight + yMargin;
        //超过10个通道时仅显示total, 11 = 10 channel + total
        if (channels.size() > 11) {
            break;
        }
    }
}

void BaseChartScene::drawBackgroundColor(QPainter *painter)
{
#if 0
    painter->save();
    painter->setPen(QPen(Qt::red));
    painter->setBrush(QColor(0, 255, 0, 80));
    painter->drawRect(sceneRect());
    painter->restore();
#else
    Q_UNUSED(painter)
#endif
}

void BaseChartScene::drawTitle(QPainter *painter)
{
    painter->save();
    painter->setRenderHints(QPainter::TextAntialiasing);
    QFont font = painter->font();
    font.setPointSize(16);
    painter->setFont(font);
    painter->setPen(QPen(gMsColor.settings.text));
    painter->drawText(QRect(0, 0, width(), marginTop() / 2), Qt::AlignCenter, m_mainTitle);
    painter->drawText(QRect(0, marginTop() / 3, width(), marginTop() / 2), Qt::AlignCenter, m_subTitle);
    painter->restore();
}

void BaseChartScene::drawYAxis(QPainter *painter, const QRectF &rect)
{
    painter->save();
    qreal yStep = (rect.height() - marginTop() - marginBottom()) / (m_gridData.yLineCount - 1);
    qreal y = rect.top() + marginTop();
    for (int i = 0; i < m_gridData.yLineCount; ++i) {
        painter->setPen(QPen(QColor(139, 139, 139), 1));
        painter->drawLine(QPointF(rect.left() + marginLeft(), y), QPointF(rect.right() - marginRight(), y));

        //
        QString text = QString("%1").arg(yLineValue(i));
        QRectF textRect;
        textRect.setLeft(rect.left());
        textRect.setRight(rect.left() + marginLeft() - 10);
        textRect.setTop(y - 10);
        textRect.setBottom(y + 10);
        painter->setPen(QPen(QColor(87, 87, 87)));
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignRight, text);

        //
        y += yStep;
    }
    painter->restore();
}

int BaseChartScene::yLineValue(int index) const
{
    return m_gridData.yMaxValue / 10.0 * (m_gridData.yLineCount - index - 1);
}

void BaseChartScene::drawXAxis(QPainter *painter, const QRectF &rect)
{
    painter->save();
    qreal xStep = (rect.width() - marginLeft() - marginRight()) / (m_gridData.xLineCount - 1);
    qreal x = rect.left() + marginLeft();
    for (int i = 0; i < m_gridData.xLineCount; ++i) {
        painter->setPen(QPen(QColor(139, 139, 139), 1));
        painter->drawLine(QPointF(x, rect.top() + marginTop()), QPointF(x, rect.bottom() - marginBottom()));

        //
        if (!m_gridData.xAxisStrings.isEmpty()) {
            QString text = m_gridData.xAxisStrings.at(i);
            QRectF textRect;
            textRect.setLeft(x - xStep);
            textRect.setRight(x + xStep);
            textRect.setTop(rect.bottom() - marginBottom());
            textRect.setBottom(rect.bottom() - marginBottom() + 30);
            painter->setPen(QPen(QColor(87, 87, 87)));
            painter->drawText(textRect, Qt::AlignCenter, text);
        }

        //
        x += xStep;
    }
    painter->restore();
}

int BaseChartScene::xLineValue(int index) const
{
    return m_gridData.xMaxValue / m_gridData.xLineCount * index;
}

QRectF BaseChartScene::gridRect() const
{
    QRectF rc = sceneRect();
    rc.setLeft(rc.left() + marginLeft());
    rc.setRight(rc.right() - marginRight());
    rc.setTop(rc.top() + marginTop());
    rc.setBottom(rc.bottom() - marginBottom());
    return rc;
}

QRectF BaseChartScene::legendRect() const
{
    QRectF rc = sceneRect();
    rc.setLeft(rc.right() - marginRight());
    rc.setRight(rc.right());
    rc.setTop(rc.top() + marginTop());
    rc.setBottom(rc.bottom() - marginBottom());
    return rc;
}

int BaseChartScene::marginLeft() const
{
    return 100;
}

int BaseChartScene::marginRight() const
{
    return 160;
}

int BaseChartScene::marginTop() const
{
    return 80;
}

int BaseChartScene::marginBottom() const
{
    return 50;
}

void BaseChartScene::onSceneRectChanged(const QRectF &rc)
{
    Q_UNUSED(rc)
}
