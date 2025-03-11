#include "DrawScenePtzMask.h"
#include <QGraphicsSceneMouseEvent>
#include <QDebug>

DrawScenePtzMask::DrawScenePtzMask(QObject *parent)
    : DrawSceneMask(parent)
{
    if (m_currentItem == nullptr) {
        m_currentItem = new DrawItemMask();
        m_currentItem->setSelected(true);
        m_currentItem->setIndex(-1);
        m_currentItem->setVisible(false);
        addItem(m_currentItem);
        m_maskList.append(m_currentItem);
    }
}

void DrawScenePtzMask::AreaEdit(const mask_area_ex &area)
{
    for (int i = 0; i < m_maskList.size(); i++) {
        DrawItemMask *item = m_maskList[i];
        if (area.area_id == item->getIndex()) {
            m_currentItem =item;
        }
    }
    qreal x = (qreal)area.start_x / 100 * width();
    qreal y = (qreal)area.start_y / 100 * height();
    qreal w = (qreal)area.area_width / 100 * width();
    qreal h = (qreal)area.area_height / 100 * height();
    m_currentItem->setRealRect(QRect (x, y, w, h));
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

void DrawScenePtzMask::hideArea()
{
    for (int i = 0; i < m_maskList.size(); ++i) {
        DrawItemMask *item = m_maskList.at(i);
        if (item != m_currentItem) {
            item->setVisible(false);
        }
    }
}

void DrawScenePtzMask::hideArea(int index)
{
    for (int i = 0; i < m_maskList.size(); i++) {
        DrawItemMask *item = m_maskList.at(i);
        if (index == item->getIndex()) {
            item->setVisible(false);
            break;
        }
    }
}

void DrawScenePtzMask::setItemRatio(int index, int ratio)
{
    for (int i = 0; i < m_maskList.size(); i++) {
        DrawItemMask *item = m_maskList.at(i);
        if (index == item->getIndex()) {
            item->setRatio(ratio);
            break;
        }
    }
}

void DrawScenePtzMask::setCurrentItemSelected(bool enable)
{
    m_currentItem->setSelected(enable);
}

void DrawScenePtzMask::cancelCurrentItem()
{
    m_currentItem = nullptr;
}

void DrawScenePtzMask::addMask(const mask_area_ex &area)
{
    if (area.area_width == 0 || area.area_height == 0) {
        return;
    }

    qreal x = (qreal)area.start_x / 100 * width();
    qreal y = (qreal)area.start_y / 100 * height();
    qreal w = (qreal)area.area_width / 100 * width();
    qreal h = (qreal)area.area_height / 100 * height();

    DrawItemMask *item = new DrawItemMask();
    item->setRealRect(QRectF(x, y, w, h));
    item->setIndex(area.area_id);
    item->setRatio(area.ratio);
    addItem(item);

    m_maskList.append(item);
}

void DrawScenePtzMask::getMask(mask_area_ex *area_array, int count)
{
    int index = 0;

    for (int i = 0; i < count; i++) {
        mask_area_ex &area = area_array[i];
        int color = area.fill_color;
        memset(&area, 0, sizeof(mask_area_ex));
        area.area_id = i;
        area.fill_color = color;
    }
    for (int i = 0; i < m_maskList.size(); i++) {
        DrawItemMask *item = m_maskList.at(i);
        index = item->getIndex();
        mask_area_ex &area = area_array[index];

        const QRectF &rc = item->realRect();
        area.start_x = qRound(rc.x() / width() * 100);
        area.start_y = qRound(rc.y() / height() * 100);
        area.area_width = qRound(rc.width() / width() * 100);
        area.area_height = qRound(rc.height() / height() * 100);
        area.enable = 1;
        area.ratio = item->getRatio();
    }
}

void DrawScenePtzMask::updateMask(mask_area_ex *area_array, int count)
{
    for (int i = 0; i < count; i++) {
        mask_area_ex &area = area_array[i];
        if (area.area_width != 0 && area.area_height != 0) {
            for (int j = 0; j < m_maskList.size(); j++) {
                DrawItemMask *item = m_maskList.at(j);
                if (item->getIndex() == i) {
                    const QRectF &rc = item->realRect();
                    area.start_x = qRound(rc.x() / width() * 100);
                    area.start_y = qRound(rc.y() / height() * 100);
                    area.area_width = qRound(rc.width() / width() * 100);
                    area.area_height = qRound(rc.height() / height() * 100);
                    break;
                }
            }
        }
    }
}

void DrawScenePtzMask::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_isEnabled) {
        return;
    }

    m_pressed = true;
    m_pressedPoint = event->scenePos();
    m_currentItem = static_cast<DrawItemMask *>(itemAt(event->scenePos(), QTransform()));
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

    QGraphicsScene::mousePressEvent(event);
}

void DrawScenePtzMask::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
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
                m_currentItem->setSelected(true);
                m_currentItem->setIndex(newId);
                addItem(m_currentItem);
                m_maskList.insert(newId, m_currentItem);
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
            if (normalRect.right() > width() + margin) {
                normalRect.setRight(width() + margin);
            }
            if (normalRect.bottom() > height() + margin) {
                normalRect.setBottom(height() + margin);
            }
            //设置最小值
            if (normalRect.width() < m_itemMinSize.width()) {
                normalRect.setWidth(m_itemMinSize.width());
            }
            if (normalRect.height() < m_itemMinSize.height()) {
                normalRect.setHeight(m_itemMinSize.height());
            }
            if (normalRect.right() > width() + margin) {
                normalRect.moveRight(width() + margin);
            }
            if (normalRect.bottom() > height() + margin) {
                normalRect.moveBottom(height() + margin);
            }
            //
            m_currentItem->setRect(normalRect);
            update(sceneRect());
        }
    }

    QGraphicsScene::mouseMoveEvent(event);
}


