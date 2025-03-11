#include "DrawSceneMask.h"
#include "MyDebug.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>

DrawSceneMask::DrawSceneMask(QObject *parent)
    : QGraphicsScene(parent)
{
    m_itemMinSize = QSize(20, 20);
}

void DrawSceneMask::addMask(const mask_area_ex &area)
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
    addItem(item);

    m_maskList.append(item);
}

void DrawSceneMask::getMask(mask_area_ex *area_array, int count)
{
    int index = 0;

    for (int i = 0; i < count; i++) {
        mask_area_ex &area = area_array[i];
        int color;
        if (area.enable) {
            color = area.fill_color;
        } else {
            color = m_colorType;
        }
        memset(&area, 0, sizeof(mask_area_ex));
        area.area_id    = i;
        area.fill_color = color;
    }
    for (int i = 0; i < m_maskList.size(); i++) {
        DrawItemMask *item = m_maskList.at(i);
        index              = item->getIndex();
        mask_area_ex &area = area_array[index];

        const QRectF &rc = item->realRect();
        area.start_x     = qRound(rc.x() / width() * 100);
        area.start_y     = qRound(rc.y() / height() * 100);
        area.area_width  = qRound(rc.width() / width() * 100);
        area.area_height = qRound(rc.height() / height() * 100);
        area.enable      = 1;
    }
}

void DrawSceneMask::updateMask(mask_area_ex *area_array, int count)
{
    for (int i = 0; i < count; i++) {
        mask_area_ex &area = area_array[i];
        if (area.area_width != 0 && area.area_height != 0) {
            for (int j = 0; j < m_maskList.size(); j++) {
                DrawItemMask *item = m_maskList.at(j);
                if (item->getIndex() == i) {
                    const QRectF &rc = item->realRect();
                    area.start_x     = qRound(rc.x() / width() * 100);
                    area.start_y     = qRound(rc.y() / height() * 100);
                    area.area_width  = qRound(rc.width() / width() * 100);
                    area.area_height = qRound(rc.height() / height() * 100);
                    break;
                }
            }
        }
    }
}

int DrawSceneMask::getCurrentItemID()
{
    if (m_currentItem) {
        return m_currentItem->getIndex();
    } else {
        return -1;
    }
}

void DrawSceneMask::setColorType(int type)
{
    m_colorType = type;
}

void DrawSceneMask::setMaxItemCount(int count)
{
    m_maxItemCount = count;
}

void DrawSceneMask::clear()
{
    if (m_currentItem) {
        m_maskList.removeOne(m_currentItem);
        removeItem(m_currentItem);
        m_currentItem = nullptr;
    }
}

void DrawSceneMask::clear(int index)
{
    for (int i = 0; i < m_maskList.size(); i++) {
        DrawItemMask *item = m_maskList.at(i);
        if (index == item->getIndex()) {
            m_maskList.removeAt(i);
            removeItem(item);
            break;
        }
    }
}

void DrawSceneMask::clearAll()
{
    for (int i = 0; i < m_maskList.size(); ++i) {
        DrawItemMask *item = m_maskList.at(i);
        removeItem(item);
        delete item;
    }
    m_maskList.clear();
    m_currentItem = nullptr;
}

void DrawSceneMask::clearSelected()
{
    for (int i = 0; i < m_maskList.size(); ++i) {
        DrawItemMask *item = m_maskList.at(i);
        item->setSelected(false);
    }
}

void DrawSceneMask::setEnabled(bool enable)
{
    m_isEnabled = enable;

    for (int i = 0; i < m_maskList.size(); ++i) {
        DrawItemMask *item = m_maskList.at(i);
        if (!enable) {
            item->setSelected(false);
        }
        item->setEnabled(enable);
    }
}

void DrawSceneMask::setVisible(bool visible)
{
    QList<QGraphicsView *> viewList = views();
    for (int i = 0; i < viewList.size(); ++i) {
        QGraphicsView *view = viewList.at(i);
        view->setVisible(visible);
    }
}

void DrawSceneMask::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_isEnabled) {
        return;
    }

    m_pressed      = true;
    m_pressedPoint = event->scenePos();

    if (m_maxItemCount == 1) {

    } else {
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
    }

    QGraphicsScene::mousePressEvent(event);
}

void DrawSceneMask::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_isEnabled) {
        return;
    }
    if (m_pressed) {
        if (m_currentItem == nullptr) {
            if (m_maskList.size() < m_maxItemCount) {
                //get id
                int newId = 0;
                int i     = 0;
                int j     = 0;
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
                addItem(m_currentItem);
                m_maskList.append(m_currentItem);
            }
        }
        if (m_currentItem && m_operation == ModeNew) {
            QRectF rc(m_pressedPoint, event->scenePos());
            QRectF normalRect = rc.normalized();
            int margin        = m_currentItem->margin();
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
        emit mouseDragging();
    }

    QGraphicsScene::mouseMoveEvent(event);
}

void DrawSceneMask::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_isEnabled) {
        return;
    }
    m_pressed   = false;
    m_operation = ModeNull;

    QGraphicsScene::mouseReleaseEvent(event);
}

const QSize &DrawSceneMask::itemMinSize() const
{
    return m_itemMinSize;
}

void DrawSceneMask::setItemMinSize(const QSize &newItemMinSize)
{
    m_itemMinSize = newItemMinSize;
}

qreal DrawSceneMask::itemBorderWidth() const
{
    return m_itemBorderWidth;
}

void DrawSceneMask::setItemBorderWidth(qreal width)
{
    m_itemBorderWidth = width;
}

void DrawSceneMask::setPosItemRegion(int x, int y, int w, int h)
{
    qreal _x = (qreal)x / 1000 * width();
    qreal _y = (qreal)y / 1000 * height();
    qreal _w = (qreal)w / 1000 * width();
    qreal _h = (qreal)h / 1000 * height();

    DrawItemMask *item = nullptr;
    if (m_maskList.isEmpty()) {
        item = new DrawItemMask();
        item->setItemMinSize(m_itemMinSize);
        item->setBorderWidth(m_itemBorderWidth);
        addItem(item);
        m_maskList.append(item);
    } else {
        item = m_maskList.first();
    }
    m_currentItem = item;
    m_currentItem->setRealRect(QRectF(_x, _y, _w, _h));
    m_currentItem->setSelected(true);
    update(sceneRect());

    emit mouseDragging();
}

void DrawSceneMask::getPosItemRegion(int &x, int &y, int &w, int &h)
{
    const QRectF &rc = m_currentItem->realRect();
    x                = qRound(rc.x() / width() * 1000);
    y                = qRound(rc.y() / height() * 1000);
    w                = qRound(rc.width() / width() * 1000);
    h                = qRound(rc.height() / height() * 1000);
}

QRect DrawSceneMask::posGlobalGeometry() const
{
    QRect rc;
    if (m_currentItem) {
        rc                              = m_currentItem->realRect().toRect();
        QList<QGraphicsView *> viewList = views();
        if (!viewList.isEmpty()) {
            QPoint p = viewList.first()->mapToGlobal(rc.topLeft());
            rc.moveTopLeft(p);
        }
    }
    return rc;
}

bool DrawSceneMask::checkRegionNum(const mask_area_ex *area_array, int count)
{
    int maskNum = 0, mosaicNum = 0;
    //先遍历一遍当前已经在表格中的区域
    for (int i = 0; i < count; i++) {
        mask_area_ex area = area_array[i];
        if (area.enable) {
            if (area.fill_color == 8) {
                mosaicNum++;
            } else {
                maskNum++;
            }
        }
    }
    //再计算本次要添加的
    for (int i = 0; i < m_maskList.size(); i++) {
        DrawItemMask *item = m_maskList.at(i);
        int index          = item->getIndex();
        mask_area_ex area  = area_array[index];
        if (area.enable == 0) {
            if (m_colorType == 8) {
                mosaicNum++;
            } else {
                maskNum++;
            }
        }
    }
    if (maskNum > 24 || mosaicNum > 4) {
        return false;
    }
    return true;
}

void DrawSceneMask::setSelectedItem(int index)
{
    for (int i = 0; i < m_maskList.size(); i++) {
        DrawItemMask *item = m_maskList.at(i);
        int itemIndex      = item->getIndex();
        if (itemIndex == index) {
            m_currentItem = item;
            item->setSelected(true);
        } else {
            item->setSelected(false);
        }
    }
}

bool DrawSceneMask::itemTextVisible() const
{
    return m_itemTextVisible;
}

void DrawSceneMask::setItemTextVisible(bool newItemTextVisible)
{
    m_itemTextVisible = newItemTextVisible;
}
