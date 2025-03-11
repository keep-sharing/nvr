#include "GraphicsItemPosText.h"
#include "MyDebug.h"
#include "PosData.h"
#include <QGraphicsScene>
#include <QPainter>
#include <QTextDocument>
#include <QTimer>

GraphicsItemPosText::GraphicsItemPosText(QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
    setZValue(ZValuePos);

    m_timeout = new QTimer(this);
    connect(m_timeout, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_timeout->setInterval(100);
}

void GraphicsItemPosText::setRect(const QRectF &rc)
{
    m_rect = rc;
    update();

    //
    m_listText.clear();
    setPosData(m_posData);
}

void GraphicsItemPosText::setPosData(const PosData &data)
{
    if (m_updateBehavior == NoPartialUpdate) {
        hideAllItem();
    }

    //显示方式改变
    if (m_posData.overlayMode() != data.overlayMode()) {
        m_listText.clear();
    }
    //避免大字体改成小字体时残留
    if (m_posData.fontSize() != data.fontSize()) {
        m_listText.clear();
        update();
    }
    //显示位置改变
    QRectF posRect = data.geometry(m_rect.toRect());
    if (posRect != m_posRect) {
        m_posRect = posRect;
        m_listText.clear();
    }
    //
    m_posData = data;
    QPointF itemPos = m_posRect.topLeft();

    //
    QStringList strList = data.text().split("\n", QString::SkipEmptyParts);

    int index = 0;
    bool isSame = true;
    bool isOverPage = false;
    for (; index < strList.size(); ++index) {
        QString text = strList.at(index);
        GraphicsTextItem *item = nullptr;
        if (m_listBlockItem.size() > index) {
            item = m_listBlockItem.at(index);
        } else {
            item = new GraphicsTextItem(this);
            m_listBlockItem.append(item);
        }
        item->setMaxHeight(0);

        if (isSame) {
            if (m_listText.size() > index && m_listText.at(index) == text) {
                itemPos.setY(itemPos.y() + item->boundingRect().height());
                continue;
            } else {
                isSame = false;
            }
        }

        item->setText(text, data.fontSize(), data.fontColor());
        QFont font = item->font();
        QFontMetrics fm(font);
        qreal w = qMin(m_posRect.width(), (qreal)fm.width(text) + 10);
        item->setTextWidth(w);
        //避免第一条没有刷新
        if (index == 0) {
            item->hide();
        }
        if (!item->isVisible()) {
            item->show();
        }

        //
        if (!isOverPage) {
            if (itemPos.y() + item->boundingRect().height() > m_posRect.bottom()) {
                isOverPage = true;
            } else {
                item->setPos(itemPos);
                itemPos.setY(itemPos.y() + item->boundingRect().height());
            }
        }

        //
        switch (m_posData.overlayMode()) {
        case 0: {
            //翻页
            if (isOverPage && index == strList.size() - 1) {
                QPointF p = m_posRect.topLeft();
                for (int i = 0; i < strList.size(); ++i) {
                    GraphicsTextItem *item = m_listBlockItem.at(i);
                    //如果最后一个比整体区域还大
                    if (item->boundingRect().height() > m_posRect.height()) {
                        item->setMaxHeight(m_posRect.height());
                    }
                    //
                    if (p.y() + item->boundingRect().height() > m_posRect.bottom()) {
                        for (int j = 0; j < i; ++j) {
                            GraphicsTextItem *item = m_listBlockItem.at(j);
                            if (item->isVisible()) {
                                item->hide();
                            }
                        }
                        p = m_posRect.topLeft();
                    }
                    item->setPos(p);
                    p.setY(p.y() + item->boundingRect().height());
                }
            }
            break;
        }
        case 1: {
            //滚动
            if (isOverPage && index == strList.size() - 1) {
                int h = 0;
                int realIndex = -1;
                for (int i = index; i >= 0; --i) {
                    GraphicsTextItem *item = m_listBlockItem.at(i);
                    h += item->boundingRect().height();
                    if (h > m_posRect.height()) {
                        item->hide();
                        if (realIndex < 0) {
                            realIndex = i + 1;
                        }
                    }
                }
                QPointF p = m_posRect.topLeft();
                for (int i = realIndex; i < strList.size(); ++i) {
                    GraphicsTextItem *item = m_listBlockItem.at(i);
                    item->setPos(p);
                    p.setY(p.y() + item->boundingRect().height());
                }
                //避免每一条都很长时不显示，至少会显示一条
                if (realIndex == strList.size()) {
                    GraphicsTextItem *item = m_listBlockItem.at(realIndex - 1);
                    item->setPos(p);
                    //如果最后一个比整体区域还大
                    if (item->boundingRect().height() > m_posRect.height()) {
                        item->setMaxHeight(m_posRect.height());
                    }
                    p.setY(p.y() + item->boundingRect().height());
                    item->show();
                }
            }
            break;
        }
        default:
            break;
        }
    }

    //
    for (; index < m_listBlockItem.size(); ++index) {
        GraphicsTextItem *item = m_listBlockItem.at(index);
        if (item->isVisible()) {
            item->hide();
        }
    }

    //
    m_listText = strList;

    //
    m_timeoutValue = 0;
    m_timeoutInterval = data.showTime() * 10;
    if (!m_timeout->isActive()) {
        m_timeout->start();
    }
}

void GraphicsItemPosText::setPaused(bool pause)
{
    if (pause) {
        m_timeout->stop();
    } else {
        m_timeout->start();
    }
}

void GraphicsItemPosText::clear()
{
    hideAllItem();
}

void GraphicsItemPosText::setUpdateBehavior(UpdateBehavior newUpdateBehavior)
{
    m_updateBehavior = newUpdateBehavior;
}

QRectF GraphicsItemPosText::boundingRect() const
{
    return m_rect;
}

void GraphicsItemPosText::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
#if 1
    Q_UNUSED(painter)
#else
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);
    painter->setPen(Qt::red);
    painter->drawRect(m_posRect);
    painter->restore();
#endif
}

void GraphicsItemPosText::hideAllItem()
{
    m_listText.clear();
    m_posData.clear();
    for (int i = 0; i < m_listBlockItem.size(); ++i) {
        GraphicsTextItem *item = m_listBlockItem.at(i);
        item->hide();
    }
}

void GraphicsItemPosText::onTimeout()
{
    m_timeoutValue++;
    if (m_timeoutValue > m_timeoutInterval) {
        hideAllItem();
        m_timeoutValue = 0;
        m_timeout->stop();
    }
}
