#include "DrawSceneAnpr.h"
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
DrawSceneAnpr::DrawSceneAnpr(QObject *parent)
    : QGraphicsScene(parent)
{
}

void DrawSceneAnpr::finishEdit()
{
    if (m_selectedRegion) {
        m_selectedRegion->setSelected(false);
    }
    m_selectedRegion = nullptr;
}

QString DrawSceneAnpr::regionName(int index) const
{
    for (int i = 0; i < m_regionList.size(); ++i) {
        DrawItemAnpr *region = m_regionList.at(i);
        if (region->index() == index) {
            return region->name(true);
        }
    }
    return QString();
}

void DrawSceneAnpr::setRegionName(int index, const QString &name)
{
    for (int i = 0; i < m_regionList.size(); ++i) {
        DrawItemAnpr *region = m_regionList.at(i);
        if (region->index() == index) {
            region->setName(name);
            break;
        }
    }
}

void DrawSceneAnpr::clearAt(int index)
{
    for (int i = 0; i < m_regionList.size(); ++i) {
        DrawItemAnpr *region = m_regionList.at(i);
        if (region->index() == index) {
            m_regionList.removeAt(i);
            if (m_selectedRegion == region) {
                m_selectedRegion = nullptr;
            }
            removeItem(region);
            delete region;
            region = nullptr;
            break;
        }
    }
}

void DrawSceneAnpr::clearCurrent()
{
    if (!m_selectedRegion) {
        return;
    }
    m_regionList.removeOne(m_selectedRegion);
    delete m_selectedRegion;
    m_selectedRegion = nullptr;
}

void DrawSceneAnpr::clearSelected()
{
    if (m_selectedRegion) {
        m_regionList.removeOne(m_selectedRegion);
        delete m_selectedRegion;
        m_selectedRegion = nullptr;
    }
}

void DrawSceneAnpr::clearAll()
{
    for (int i = m_regionList.size(); i > 0; --i) {
        DrawItemAnpr *widget = m_regionList.at(i - 1);
        delete widget;
    }
    m_regionList.clear();
    m_selectedRegion = nullptr;
}

bool DrawSceneAnpr::hasDraw() const
{
    return m_selectedRegion != nullptr;
}

bool DrawSceneAnpr::isFull() const
{
    return m_regionList.size() >= m_maxRegion;
}

void DrawSceneAnpr::setCurrentIndex(int index)
{
    m_currentIndex = index;
}

int DrawSceneAnpr::getSelectedRegionIndex()
{
    if (m_selectedRegion) {
        return m_selectedRegion->index();
    } else {
        return -1;
    }
}

void DrawSceneAnpr::cancelSelected()
{
    if (m_selectedRegion) {
        m_selectedRegion->setSelected(false);
    }
}

void DrawSceneAnpr::setRegionData(ms_lpr_position_info *region_array)
{
    clearAll();

    for (int i = 0; i < m_maxRegion; ++i) {
        const ms_lpr_position_info &info = region_array[i];
        if (info.endX <= 0 || info.endY <= 0) {
            continue;
        }

        DrawItemAnpr *region = new DrawItemAnpr();
        region->setIndex(i);
        //qDebug() << QString("AnprDraw::setRegionData, index: %1, (%2, %3, %4 x %5)").arg(i).arg(info.x).arg(info.y).arg(info.width).arg(info.height);
        qreal x1 = (qreal)info.startX / m_baseRegionWidth * width();
        qreal y1 = (qreal)info.startY / m_baseRegionHeight * height();
        qreal x2 = (qreal)info.endX / m_baseRegionWidth * width();
        qreal y2 = (qreal)info.endY / m_baseRegionHeight * height();
        region->setRealRect(QRectF(QPointF(x1, y1), QPointF(x2, y2)));
        region->setName(info.name);
        region->show();
        m_regionList.append(region);
        addItem(region);
    }
    refreshstack();
    updateItemSizeText();
}

void DrawSceneAnpr::getRegionData(ms_lpr_position_info *region_array, bool jsonSupport)
{
    for (int i = 0; i < m_regionList.size(); ++i) {
        DrawItemAnpr *region = m_regionList.at(i);
        const QRectF &rc = region->realRect();
        int index = region->index();
        if (index >= 0 && index < 4) {
            ms_lpr_position_info &lpr_position_info = region_array[index];
            lpr_position_info.startX = rc.left() / width() * m_baseRegionWidth;
            lpr_position_info.startY = rc.top() / height() * m_baseRegionHeight;
            lpr_position_info.endX = rc.right() / width() * m_baseRegionWidth;
            lpr_position_info.endY = rc.bottom() / height() * m_baseRegionHeight;
            snprintf(lpr_position_info.name, sizeof(lpr_position_info.name), "%s", region->name(jsonSupport).toStdString().c_str());
        }
    }
}

void DrawSceneAnpr::updateRegionData(ms_lpr_position_info *region_array)
{
    for (int i = 0; i < m_maxRegion; i++) {
        ms_lpr_position_info &lpr_position_info = region_array[i];
        if (lpr_position_info.endX - lpr_position_info.startX != 0 && lpr_position_info.endY - lpr_position_info.startY != 0) {
            for (int j = 0; j < m_regionList.size(); j++) {
                DrawItemAnpr *region = m_regionList.at(j);
                if (region->index() == i) {
                    const QRectF &rc = region->realRect();
                    lpr_position_info.startX = rc.left() / width() * m_baseRegionWidth;
                    lpr_position_info.startY = rc.top() / height() * m_baseRegionHeight;
                    lpr_position_info.endX = rc.right() / width() * m_baseRegionWidth;
                    lpr_position_info.endY = rc.bottom() / height() * m_baseRegionHeight;
                    break;
                }
            }
        }
    }
}

void DrawSceneAnpr::setBaseRegionResolution(int width, int height)
{
    m_baseRegionWidth = width;
    m_baseRegionHeight = height;

    updateItemSizeText();
}

int DrawSceneAnpr::baseRegionWidth() const
{
    return m_baseRegionWidth;
}

int DrawSceneAnpr::baseRegionHeight() const
{
    return m_baseRegionHeight;
}

void DrawSceneAnpr::refreshstack()
{
    //重新按item从小到大的顺序排列
    for (auto iter : m_regionList) {
        for (auto nextIter : m_regionList) {
            if (iter->index() > nextIter->index()) {
                iter->stackBefore(nextIter);
            }
        }
    }
}

void DrawSceneAnpr::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_pressed = true;
    m_pressPoint = event->scenePos();
    m_selectedRegion = nullptr;
    QList<QGraphicsItem *> temp = items(event->scenePos());
    for (auto *i : temp) {
        DrawItemAnpr *item = qgraphicsitem_cast<DrawItemAnpr *>(i);
        if (item) {
            m_selectedRegion = item;
            break;
        }
    }
    if (m_selectedRegion) {
        m_operation = ModeOld;
        m_selectedRegion->setSelected(true);
    } else {
        m_operation = ModeNew;
    }
    for (int i = 0; i < m_regionList.size(); ++i) {
        DrawItemAnpr *widget = m_regionList.at(i);
        if (widget != m_selectedRegion) {
            widget->setSelected(false);
        }
    }
    QGraphicsScene::mousePressEvent(event);
}

void DrawSceneAnpr::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_pressed) {
        if (m_selectedRegion == nullptr && m_regionList.size() < m_maxRegion) {
            int index = availableIndex();
            m_selectedRegion = new DrawItemAnpr(nullptr);
            m_selectedRegion->setIndex(index);
            m_selectedRegion->setSelected(true);
            m_selectedRegion->setName(QString("ROI_%1").arg(index + 1));
            //m_selectedRegion->show();
            addItem(m_selectedRegion);
            m_regionList.append(m_selectedRegion);
            refreshstack();
        }
        if (m_selectedRegion && m_operation == ModeNew) {
            QRectF rc(m_pressPoint, event->scenePos());
            QRectF normalRect = rc.normalized();
            int margin = m_selectedRegion->margin();
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
            m_selectedRegion->setRect(normalRect);
            //m_selectedRegion->setSelected(true);
        }
    }
    updateItemSizeText();
    QGraphicsScene::mouseMoveEvent(event);
}

void DrawSceneAnpr::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    m_pressed = false;
    m_operation = ModeNull;
    QGraphicsScene::mouseReleaseEvent(event);
}

int DrawSceneAnpr::availableIndex() const
{
    QMap<int, int> m_usedIndexMap;
    for (int i = 0; i < m_regionList.size(); ++i) {
        DrawItemAnpr *region = m_regionList.at(i);
        m_usedIndexMap.insert(region->index(), 0);
    }
    for (int i = 0; i < m_maxRegion; ++i) {
        if (!m_usedIndexMap.contains(i)) {
            return i;
        }
    }
    return -1;
}

void DrawSceneAnpr::updateItemSizeText()
{
    for (int i = 0; i < m_regionList.size(); ++i) {
        DrawItemAnpr *widget = m_regionList.at(i);
        widget->updateSizeText();
    }
}
