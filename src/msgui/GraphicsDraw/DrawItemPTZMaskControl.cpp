#include "DrawItemPTZMaskControl.h"
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QPainter>
DrawItemPTZMaskControl::DrawItemPTZMaskControl(QGraphicsItem *parent)
    : DrawItemMaskControl(parent)
{
    m_currentItem = new DrawItemMask();
    m_currentItem->setSelected(true);
    m_currentItem->setIndex(-1);
    m_currentItem->setVisible(false);
    m_currentItem->setParentItem(this);
    m_maskList.append(m_currentItem);
}

void DrawItemPTZMaskControl::editArea(const mask_area_ex &area)
{
    for (int i = 0; i < m_maskList.size(); i++) {
        DrawItemMask *item = m_maskList[i];
        if (area.area_id == item->getIndex()) {
            m_currentItem = item;
            break;
        }
    }
    qreal x = static_cast<qreal>(area.start_x) / 100 * scene()->width();
    qreal y = static_cast<qreal>(area.start_y) / 100 * scene()->height();
    qreal w = static_cast<qreal>(area.area_width) / 100 * scene()->width();
    qreal h = static_cast<qreal>(area.area_height) / 100 * scene()->height();
    m_currentItem->setRealRect(QRect(x, y, w, h));
    m_currentItem->setSelected(true);
    m_currentItem->setVisible(true);
    m_operation = ModeOld;
    for (int i = 0; i < m_maskList.size(); i++) {
        DrawItemMask *item = m_maskList.at(i);
        if (item->getIndex() != m_currentItem->getIndex()) {
            item->setSelected(false);
        }
    }
}

void DrawItemPTZMaskControl::hideArea()
{
    for (int i = 0; i < m_maskList.size(); ++i) {
        DrawItemMask *item = m_maskList.at(i);
        if (item != m_currentItem) {
            item->setVisible(false);
        }
    }
}

void DrawItemPTZMaskControl::hideArea(int index)
{
    for (int i = 0; i < m_maskList.size(); i++) {
        DrawItemMask *item = m_maskList.at(i);
        if (index == item->getIndex()) {
            item->setVisible(false);
            break;
        }
    }
}

void DrawItemPTZMaskControl::setItemRatio(int index, int ratio)
{
    for (int i = 0; i < m_maskList.size(); i++) {
        DrawItemMask *item = m_maskList.at(i);
        if (index == item->getIndex()) {
            item->setRatio(ratio);
            break;
        }
    }
}

void DrawItemPTZMaskControl::setCurrentItemSelected(bool enable)
{
    m_currentItem->setSelected(enable);
}

void DrawItemPTZMaskControl::cancelCurrentItem()
{
    m_currentItem = nullptr;
}

void DrawItemPTZMaskControl::addMask(const mask_area_ex &area)
{
    if (area.area_width == 0 || area.area_height == 0) {
        return;
    }

    qreal x = static_cast<qreal>(area.start_x) / 100 * scene()->width();
    qreal y = static_cast<qreal>(area.start_y) / 100 * scene()->height();
    qreal w = static_cast<qreal>(area.area_width) / 100 * scene()->width();
    qreal h = static_cast<qreal>(area.area_height) / 100 * scene()->height();

    DrawItemMask *item = new DrawItemMask();
    item->setIndex(area.area_id);
    item->setRealRect(QRectF(x, y, w, h));
    item->setRatio(area.ratio);
    item->setParentItem(this);

    m_maskList.append(item);
}

Uint64 DrawItemPTZMaskControl::getMask(mask_area_ex *area_array)
{
    Uint64 mask = 0;
    //更新旧的区域位置
    mask = updateMask(area_array, mask);
    //添加新的区域
    for (int i = 0; i < m_maskList.size(); i++) {
        DrawItemMask *item = m_maskList.at(i);
        int index = item->getIndex();
        mask_area_ex &area = area_array[index];
        if (area.area_width == 0 || area.area_height == 0) {
            mask |= (Uint64)1 << index;
            const QRectF &rc = item->realRect();
            area.start_x = qRound(rc.x() / scene()->width() * 100);
            area.start_y = qRound(rc.y() / scene()->height() * 100);
            area.area_width = qRound(rc.width() / scene()->width() * 100);
            area.area_height = qRound(rc.height() / scene()->height() * 100);
            area.enable = 1;
            area.ratio = item->getRatio();
            area.fill_color = m_colorType;
        }
    }
    return mask;
}

Uint64 DrawItemPTZMaskControl::updateMask(mask_area_ex *area_array, int count)
{
    Uint64 mask = 0;
    for (int i = 0; i < count; i++) {
        mask_area_ex &area = area_array[i];
        if (area.area_width != 0 && area.area_height != 0) {
            for (int j = 0; j < m_maskList.size(); j++) {
                DrawItemMask *item = m_maskList.at(j);
                if (item->getIndex() == i) {
                    const QRectF &rc = item->realRect();
                    if (area.start_x != qRound(rc.x() / scene()->width() * 100) || area.start_y != qRound(rc.y() / scene()->height() * 100) ||
                        area.area_width != qRound(rc.width() / scene()->width() * 100) || area.area_height != qRound(rc.height() / scene()->height() * 100)) {
                        mask |= (Uint64)1 << i;
                    }
                    area.start_x     = qRound(rc.x() / scene()->width() * 100);
                    area.start_y     = qRound(rc.y() / scene()->height() * 100);
                    area.area_width  = qRound(rc.width() / scene()->width() * 100);
                    area.area_height = qRound(rc.height() / scene()->height() * 100);
                    break;
                }
            }
        }
    }
    return mask;
}

void DrawItemPTZMaskControl::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_isEnabled) {
        return;
    }

    m_pressed = true;
    m_pressedPoint = event->scenePos();
    m_currentItem = nullptr;
    QList<QGraphicsItem *> temp = scene()->items(m_pressedPoint);
    for (auto *i : temp) {
        DrawItemMask *item = qgraphicsitem_cast<DrawItemMask *>(i);
        if (item && this != i) {
            m_currentItem = item;
            break;
        }
    }
    if (m_currentItem) {
        m_operation = ModeOld;
        m_currentItem->setSelected(true);
    } else {
        m_operation = ModeNew;
    }

    for (int i = 0; i < m_maskList.size(); ++i) {
        DrawItemMask *item = m_maskList.at(i);
        if (item != m_currentItem) {
            item->setSelected(false);
        }
    }

    QGraphicsRectItem::mousePressEvent(event);
}

void DrawItemPTZMaskControl::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_isEnabled) {
        return;
    }
    if (m_pressed) {
        if (m_currentItem == nullptr) {
            if (m_maskList.size() < m_maxItemCount) {
                //get id
                int newId = 0;
                int i = 0;
                int j = 0;
                for (i = 0; i < m_maxItemCount; i++) {
                    for (j = 0; j < m_maskList.size(); j++) {
                        DrawItemMask *widget = m_maskList.at(j);
                        if (widget->getIndex() == i) {
                            break;
                        }
                    }

                    if (j >= m_maskList.size()) {
                        newId = i;
                        break;
                    }
                }
                m_currentItem = new DrawItemMask();
                m_currentItem->setItemTextVisible(m_itemTextVisible);
                m_currentItem->setItemMinSize(m_itemMinSize);
                m_currentItem->setBorderWidth(m_itemBorderWidth);
                m_currentItem->setSelected(true);
                m_currentItem->setIndex(newId);
                m_currentItem->setParentItem(this);
                m_maskList.insert(newId, m_currentItem);
                refreshstack();
            }
        }
        if (m_currentItem && m_operation == ModeNew) {
            QRectF rc(m_pressedPoint, event->scenePos());
            QRectF normalRect = rc.normalized();
            int margin = m_currentItem->margin();
            if (normalRect.left() < -margin) {
                normalRect.setLeft(-margin);
            }
            if (normalRect.top() < -margin) {
                normalRect.setTop(-margin);
            }
            if (normalRect.right() > scene()->width() + margin) {
                normalRect.setRight(scene()->width() + margin);
            }
            if (normalRect.bottom() > scene()->height() + margin) {
                normalRect.setBottom(scene()->height() + margin);
            }
            //设置最小值
            if (normalRect.width() < m_itemMinSize.width()) {
                normalRect.setWidth(m_itemMinSize.width());
            }
            if (normalRect.height() < m_itemMinSize.height()) {
                normalRect.setHeight(m_itemMinSize.height());
            }
            if (normalRect.right() > scene()->width() + margin) {
                normalRect.moveRight(scene()->width() + margin);
            }
            if (normalRect.bottom() > scene()->height() + margin) {
                normalRect.moveBottom(scene()->height() + margin);
            }
            //
            m_currentItem->setRect(normalRect);
            update(scene()->sceneRect());
        }
    }

    QGraphicsRectItem::mouseMoveEvent(event);
}
