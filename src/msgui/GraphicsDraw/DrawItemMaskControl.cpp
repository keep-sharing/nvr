#include "DrawItemMaskControl.h"
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QPainter>
DrawItemMaskControl::DrawItemMaskControl(QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable);
    setFiltersChildEvents(true);
    m_itemMinSize = QSize(20, 20);
}

void DrawItemMaskControl::init()
{
    setRect(scene()->sceneRect());
}

QRectF DrawItemMaskControl::boundingRect() const
{
    QRectF rc = rect();
    rc.adjust(-10, -10, 10, 10);
    return rc;
}

int DrawItemMaskControl::type() const
{
    return Type;
}

void DrawItemMaskControl::setMaxItemCount(int count)
{
    m_maxItemCount = count;
}

void DrawItemMaskControl::clear()
{
    if (m_currentItem) {
        m_maskList.removeOne(m_currentItem);
        m_currentItem->setParentItem(nullptr);
        m_currentItem->hide();
        m_currentItem = nullptr;
    }
}

void DrawItemMaskControl::clear(int index)
{
    for (int i = 0; i < m_maskList.size(); i++) {
        DrawItemMask *item = m_maskList.at(i);
        if (index == item->getIndex()) {
            m_maskList.removeAt(i);
            if (item == m_currentItem) {
                m_currentItem = nullptr;
            }
            item->setParentItem(nullptr);
            item->hide();
            break;
        }
    }
}

void DrawItemMaskControl::clearAll()
{
    for (int i = 0; i < m_maskList.size(); ++i) {
        DrawItemMask *item = m_maskList.at(i);
        item->setParentItem(nullptr);
        item->hide();
    }
    m_maskList.clear();
    m_currentItem = nullptr;
}

void DrawItemMaskControl::clearSelected()
{
    for (int i = 0; i < m_maskList.size(); ++i) {
        DrawItemMask *item = m_maskList.at(i);
        item->setSelected(false);
    }
}

void DrawItemMaskControl::setEnabled(bool enable)
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

void DrawItemMaskControl::setVisible(bool visible)
{
    QList<QGraphicsView *> viewList = scene()->views();
    for (int i = 0; i < viewList.size(); ++i) {
        QGraphicsView *view = viewList.at(i);
        view->setVisible(visible);
    }
}

void DrawItemMaskControl::addMask(const mask_area_ex &area)
{
    if (area.area_width == 0 || area.area_height == 0) {
        return;
    }

    qreal x = static_cast<qreal>(area.start_x) / 100 * scene()->width();
    qreal y = static_cast<qreal>(area.start_y) / 100 * scene()->height();
    qreal w = static_cast<qreal>(area.area_width) / 100 * scene()->width();
    qreal h = static_cast<qreal>(area.area_height) / 100 * scene()->height();
    DrawItemMask *item = new DrawItemMask();
    item->setRealRect(QRectF(x, y, w, h));
    item->setIndex(area.area_id);
    item->setParentItem(this);

    m_maskList.append(item);
}

void DrawItemMaskControl::getMask(mask_area_ex *area_array)
{
    //更新旧的区域位置
    updateMask(area_array);
    //添加新的区域
    for (int i = 0; i < m_maskList.size(); i++) {
        DrawItemMask *item = m_maskList.at(i);
        int index = item->getIndex();
        mask_area_ex &area = area_array[index];

        if (area.area_width == 0 || area.area_height == 0) {
            const QRectF &rc = item->realRect();
            area.start_x = qRound(rc.x() / scene()->width() * 100);
            area.start_y = qRound(rc.y() / scene()->height() * 100);
            area.area_width = qRound(rc.width() / scene()->width() * 100);
            area.area_height = qRound(rc.height() / scene()->height() * 100);
            area.enable = 1;
            area.fill_color = m_colorType;
        }
    }
}

void DrawItemMaskControl::updateMask(mask_area_ex *area_array, int count)
{
    for (int i = 0; i < count; i++) {
        mask_area_ex &area = area_array[i];
        if (area.area_width != 0 && area.area_height != 0) {
            for (int j = 0; j < m_maskList.size(); j++) {
                DrawItemMask *item = m_maskList.at(j);
                if (item->getIndex() == i) {
                    const QRectF &rc = item->realRect();
                    area.start_x = qRound(rc.x() / scene()->width() * 100);
                    area.start_y = qRound(rc.y() / scene()->height() * 100);
                    area.area_width = qRound(rc.width() / scene()->width() * 100);
                    area.area_height = qRound(rc.height() / scene()->height() * 100);
                    if (!isSupportMosaic) {
                        area.fill_color = m_colorType;
                    }
                    break;
                }
            }
        }
    }
}

int DrawItemMaskControl::getCurrentItemID()
{
    if (m_currentItem) {
        return m_currentItem->getIndex();
    } else {
        return -1;
    }
}

void DrawItemMaskControl::setColorType(int type)
{
    m_colorType = type;
}

bool DrawItemMaskControl::itemTextVisible() const
{
    return m_itemTextVisible;
}

void DrawItemMaskControl::setItemTextVisible(bool newItemTextVisible)
{
    m_itemTextVisible = newItemTextVisible;
}

const QSize &DrawItemMaskControl::itemMinSize() const
{
    return m_itemMinSize;
}

void DrawItemMaskControl::setItemMinSize(const QSize &newItemMinSize)
{
    m_itemMinSize = newItemMinSize;
}

qreal DrawItemMaskControl::itemBorderWidth() const
{
    return m_itemBorderWidth;
}

void DrawItemMaskControl::setItemBorderWidth(qreal width)
{
    m_itemBorderWidth = width;
}

void DrawItemMaskControl::refreshstack()
{
    //重新按item从小到大的顺序排列
    for(auto iter : m_maskList) {
        for (auto nextIter : m_maskList) {
            if (iter->getIndex() > nextIter->getIndex()){
                iter->stackBefore(nextIter);
            }
        }
    }
}

void DrawItemMaskControl::updateSingleColor(mask_area_ex *area_array, int color, int count)
{
    for (int i = 0; i < count; i++) {
        mask_area_ex &area = area_array[i];
        if (area.area_width != 0 && area.area_height != 0) {
            for (int j = 0; j < m_maskList.size(); j++) {
                DrawItemMask *item = m_maskList.at(j);
                if (item->getIndex() == i) {
                    if (area.fill_color != 8) {
                        area.fill_color = color;
                    }
                    break;
                }
            }
        }
    }
}

bool DrawItemMaskControl::checkRegionNum(const mask_area_ex *area_array, int count)
{
    Q_UNUSED(count)

    int maskNum = 0, mosaicNum = 0;
    for (int i = 0; i < m_maskList.size(); i++) {
        DrawItemMask *item = m_maskList.at(i);
        if (item->realRect().width() > 0 && item->realRect().height() > 0) {
            int index = item->getIndex();
            mask_area_ex area = area_array[index];
            bool isEnable = area.area_width > 0 && area.area_height > 0 && area.enable;
            if ((isEnable && area.fill_color == 8) || (!isEnable && m_colorType == 8)) {
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

void DrawItemMaskControl::setSelectedItem(int index)
{
    for (int i = 0; i < m_maskList.size(); i++) {
        DrawItemMask *item = m_maskList.at(i);
        int itemIndex = item->getIndex();
        if (itemIndex == index) {
            m_currentItem = item;
            item->setSelected(true);
        } else {
            item->setSelected(false);
        }
    }
}

void DrawItemMaskControl::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    if (scene()) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        painter->setBrush(Qt::NoBrush);

        painter->drawRect(scene()->sceneRect());
        painter->restore();
    }
}

void DrawItemMaskControl::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_isEnabled) {
        return;
    }

    m_pressed = true;
    m_pressedPoint = event->scenePos();

    if (m_maxItemCount != 1) {
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
    }
    QGraphicsRectItem::mousePressEvent(event);
}

void DrawItemMaskControl::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
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
                m_currentItem = new DrawItemMask(nullptr);
                m_currentItem->setItemTextVisible(m_itemTextVisible);
                m_currentItem->setItemMinSize(m_itemMinSize);
                m_currentItem->setBorderWidth(m_itemBorderWidth);
                m_currentItem->setSelected(true);
                m_currentItem->setIndex(newId);
                m_currentItem->setParentItem(this);
                m_maskList.append(m_currentItem);
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

void DrawItemMaskControl::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_isEnabled) {
        return;
    }
    m_pressed = false;
    m_operation = ModeNull;

    QGraphicsRectItem::mouseReleaseEvent(event);
}

bool DrawItemMaskControl::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);
    switch (event->type()) {
    case QEvent::GraphicsSceneMousePress: {
        mousePressEvent(mouseEvent);
        break;
    }
    case QEvent::GraphicsSceneMouseMove: {
        mouseMoveEvent(mouseEvent);
        break;
    }
    case QEvent::GraphicsSceneMouseRelease: {
        mouseReleaseEvent(mouseEvent);
        break;
    }
    default:
        break;
    }

    return QGraphicsRectItem::sceneEventFilter(watched, event);
}

bool DrawItemMaskControl::getIsSupportMosaic() const
{
    return isSupportMosaic;
}

void DrawItemMaskControl::setIsSupportMosaic(bool value)
{
    isSupportMosaic = value;
}
