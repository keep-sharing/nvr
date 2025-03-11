#include "DrawSceneRoi.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QtDebug>

#define MAX_WIDTH 100.0
#define MAX_HEIGHT 100.0

DrawSceneRoi::DrawSceneRoi(QObject *parent)
    : QGraphicsScene(parent)
{
}

void DrawSceneRoi::addRoiArea(const image_roi_area &area, int index)
{
    if (area.right == 0 || area.bottom == 0) {
        return;
    }

    qreal l = (qreal)area.left * width() / MAX_WIDTH;
    qreal t = (qreal)area.top * height() / MAX_HEIGHT;
    qreal w = (qreal)area.right * width() / MAX_WIDTH;
    qreal h = (qreal)area.bottom * height() / MAX_HEIGHT;

    DrawItemRoi *item = new DrawItemRoi(nullptr);
    item->setRealRect(QRect(l, t, w, h));
    item->setIndex(index);
    addItem(item);
    m_maskList.append(item);
}

void DrawSceneRoi::getRoiArea(image_roi_area *area_array, int count)
{
    //更新旧的区域位置
    updateRotArea(area_array, count);
    //添加新的区域

    for (int i = 0; i < m_maskList.size(); i++) {
        DrawItemRoi *item = m_maskList.at(i);
        int index = item->getIndex();
        image_roi_area &area = area_array[index];

        if (area.top - area.bottom == 0 || area.right - area.left == 0) {
            const QRectF &rc = item->realRect();
            area.left = qRound(rc.x() * MAX_WIDTH / width());
            area.top = qRound(rc.y() * MAX_HEIGHT / height());
            area.right = qRound(rc.width() * MAX_WIDTH / width());
            area.bottom = qRound(rc.height() * MAX_HEIGHT / height());
            area.enable = 1;
        }
    }
}

void DrawSceneRoi::updateRotArea(image_roi_area *area_array, int count)
{
    for (int i = 0; i < count; i++) {
        image_roi_area &area = area_array[i];
        if (area.top - area.bottom != 0 && area.right - area.left != 0) {
            for (int j = 0; j < m_maskList.size(); j++) {
                DrawItemRoi *item = m_maskList.at(j);
                if (item->getIndex() == i) {
                    const QRectF &rc = item->realRect();
                    area.left = qRound(rc.x() * MAX_WIDTH / width());
                    area.top = qRound(rc.y() * MAX_HEIGHT / height());
                    area.right = qRound(rc.width() * MAX_WIDTH / width());
                    area.bottom = qRound(rc.height() * MAX_HEIGHT / height());
                    break;
                }
            }
        }
    }
}

int DrawSceneRoi::getRoiWidth(const image_roi_area &area)
{
    int w = (qreal)area.right * width() / MAX_WIDTH;
    return w;
}

int DrawSceneRoi::getRoiHeight(const image_roi_area &area)
{
    int h = (qreal)area.bottom * height() / MAX_HEIGHT;
    return h;
}

int DrawSceneRoi::getCurrentItemID()
{
    if (m_currentItem) {
        return m_currentItem->getIndex();
    } else {
        return -1;
    }
}

void DrawSceneRoi::setColorType(int type)
{
    m_colorType = type;
}

void DrawSceneRoi::refreshstack()
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

void DrawSceneRoi::setMaxItemCount(int count)
{
    m_maxItemCount = count;
}

void DrawSceneRoi::clear()
{
    if (m_currentItem) {
        m_maskList.removeOne(m_currentItem);
        removeItem(m_currentItem);
        m_currentItem = nullptr;
    }
}

void DrawSceneRoi::clear(int index)
{
    for (int i = 0; i < m_maskList.size(); i++) {
        DrawItemRoi *item = m_maskList.at(i);
        if (index == item->getIndex()) {
            m_maskList.removeAt(i);
            removeItem(item);
            break;
        }
    }
}

void DrawSceneRoi::clearAll()
{
    for (int i = 0; i < m_maskList.size(); ++i) {
        DrawItemRoi *item = m_maskList.at(i);
        removeItem(item);
        delete item;
    }
    m_maskList.clear();
    m_currentItem = nullptr;
}

void DrawSceneRoi::setEnabled(bool enable)
{
    m_isEnabled = enable;
    for (int i = 0; i < m_maskList.size(); ++i) {
        DrawItemRoi *item = m_maskList.at(i);
        item->setSelected(false);
        item->setEnabled(enable);
    }
}

void DrawSceneRoi::setVisible(bool visible)
{
    QList<QGraphicsView *> viewList = views();
    for (int i = 0; i < viewList.size(); ++i) {
        QGraphicsView *view = viewList.at(i);
        view->setVisible(visible);
    }
}

void DrawSceneRoi::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_isEnabled) {
        return;
    }
    m_pressed = true;
    m_pressedPoint = event->scenePos();
    m_currentItem = static_cast<DrawItemRoi *>(itemAt(event->scenePos(), QTransform()));
    if (m_currentItem) {
        m_operation = ModeOld;
        m_currentItem->setSelected(true);
    } else {
        m_operation = ModeNew;
    }

    for (int i = 0; i < m_maskList.size(); ++i) {
        DrawItemRoi *item = m_maskList.at(i);
        if (item != m_currentItem) {
            item->setSelected(false);
        }
    }

    QGraphicsScene::mousePressEvent(event);
}

void DrawSceneRoi::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_isEnabled) {
        return;
    }
    if (m_pressed) {
        if (m_currentItem == nullptr && m_maskList.size() < m_maxItemCount) {
            //get id
            int newId = 0, i = 0, j = 0;
            for (i = 0; i < m_maxItemCount; i++) {
                for (j = 0; j < m_maskList.size(); j++) {
                    DrawItemRoi *widget = m_maskList.at(j);
                    if (widget->getIndex() == i) {
                        break;
                    }
                }
                if (j >= m_maskList.size()) {
                    newId = i;
                    break;
                }
            }
            m_currentItem = new DrawItemRoi(nullptr);
            m_currentItem->setSelected(true);
            m_currentItem->setIndex(newId);
            addItem(m_currentItem);
            m_maskList.append(m_currentItem);
            refreshstack();
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
            if (normalRect.width() < 20) {
                normalRect.setWidth(20);
            }
            if (normalRect.height() < 20) {
                normalRect.setHeight(20);
            }
            if (normalRect.right() > width() + margin) {
                normalRect.moveRight(width() + margin);
            }
            if (normalRect.bottom() > height() + margin) {
                normalRect.moveBottom(height() + margin);
            }
            //
            m_currentItem->setRect(normalRect);
        }
    }

    QGraphicsScene::mouseMoveEvent(event);
}

void DrawSceneRoi::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_isEnabled) {
        return;
    }
    m_pressed = false;
    m_operation = ModeNull;

    QGraphicsScene::mouseReleaseEvent(event);
}
