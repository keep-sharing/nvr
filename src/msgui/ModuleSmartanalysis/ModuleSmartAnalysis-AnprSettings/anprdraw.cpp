#include "anprdraw.h"

#include "anprregion.h"

#include <QMouseEvent>
#include <QPainter>
#include <QtDebug>

AnprDraw::AnprDraw(QWidget *parent)
    : QWidget(parent)
{
}

void AnprDraw::finishEdit()
{
    if (m_editingRegion) {
        m_editingRegion->setSelected(false);
    }
    m_editingRegion = nullptr;
    m_selectedRegion = nullptr;
}

QString AnprDraw::regionName(int index) const
{
    QString name;
    if (m_regionList.size() > index) {
        AnprRegion *region = m_regionList.at(index);
        name = region->name();
    }
    return name;
}

void AnprDraw::setRegionName(int index, const QString &name)
{
    for (int i = 0; i < m_regionList.size(); ++i) {
        AnprRegion *region = m_regionList.at(i);
        if (region->index() == index) {
            region->setName(name);
        }
    }
}

void AnprDraw::clearAt(int index)
{
    //qDebug() << QString("AnprDraw::clearAt, index: %1").arg(index);
    for (int i = 0; i < m_regionList.size(); ++i) {
        AnprRegion *region = m_regionList.at(i);
        if (region->index() == index) {
            m_regionList.removeAt(i);
            if (m_editingRegion == region) {
                m_editingRegion = nullptr;
            }
            if (m_selectedRegion == region) {
                m_selectedRegion = nullptr;
            }
            region->deleteLater();
            region = nullptr;
            break;
        }
    }

    update();
}

void AnprDraw::clearCurrent()
{
    if (!m_editingRegion) {
        return;
    }
    if (m_selectedRegion == m_editingRegion) {
        m_selectedRegion = nullptr;
    }
    m_regionList.removeOne(m_editingRegion);
    m_editingRegion->deleteLater();
    m_editingRegion = nullptr;

    update();
}

void AnprDraw::clearSelected()
{
    if (m_selectedRegion) {
        m_regionList.removeOne(m_selectedRegion);
        delete m_selectedRegion;
        m_selectedRegion = nullptr;
    }

    update();
}

void AnprDraw::clearAll()
{
    for (int i = m_regionList.size(); i > 0; --i) {
        AnprRegion *widget = m_regionList.at(i - 1);
        delete widget;
    }
    m_regionList.clear();
    m_editingRegion = nullptr;
    m_selectedRegion = nullptr;

    update();
}

bool AnprDraw::hasDraw() const
{
    return m_editingRegion != nullptr;
}

bool AnprDraw::isFull() const
{
    return m_regionList.size() >= m_maxRegion;
}

void AnprDraw::setCurrentIndex(int index)
{
    m_currentIndex = index;
}

void AnprDraw::setRegionData(ms_lpr_position_info *region_array)
{
    clearAll();

    for (int i = 0; i < m_maxRegion; ++i) {
        const ms_lpr_position_info &info = region_array[i];
        if (info.endX <= 0 || info.endY <= 0) {
            continue;
        }

        AnprRegion *region = new AnprRegion(i, this);
        //qDebug() << QString("AnprDraw::setRegionData, index: %1, (%2, %3, %4 x %5)").arg(i).arg(info.x).arg(info.y).arg(info.width).arg(info.height);
        int x1 = qRound((qreal)info.startX / m_baseRegionWidth * width());
        int y1 = qRound((qreal)info.startY / m_baseRegionHeight * height());
        int x2 = qRound((qreal)info.endX / m_baseRegionWidth * width());
        int y2 = qRound((qreal)info.endY / m_baseRegionHeight * height());
        region->setRealRect(QRect(QPoint(x1, y1), QPoint(x2, y2)));
        region->setName(info.name);
        region->show();
        m_regionList.append(region);
    }
}

void AnprDraw::getRegionData(ms_lpr_position_info *region_array)
{
    for (int i = 0; i < m_regionList.size(); ++i) {
        AnprRegion *region = m_regionList.at(i);
        const QRect &rc = region->realRect();
        int index = region->index();
        if (index >= 0 && index < 4) {
            ms_lpr_position_info &lpr_position_info = region_array[index];
            lpr_position_info.startX = (qreal)rc.left() / width() * m_baseRegionWidth;
            lpr_position_info.startY = (qreal)rc.top() / height() * m_baseRegionHeight;
            lpr_position_info.endX = (qreal)rc.right() / width() * m_baseRegionWidth;
            lpr_position_info.endY = (qreal)rc.bottom() / height() * m_baseRegionHeight;
            snprintf(lpr_position_info.name, sizeof(lpr_position_info.name), "%s", region->name().toStdString().c_str());
        }
    }
}

void AnprDraw::setBaseRegionResolution(int width, int height)
{
    m_baseRegionWidth = width;
    m_baseRegionHeight = height;

    update();
}

void AnprDraw::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.setPen(Qt::red);
    for (int i = 0; i < m_regionList.size(); ++i) {
        const AnprRegion *region = m_regionList.at(i);
        const QRect &rc = region->realRect();
        int w = qMin(qRound((qreal)rc.width() / width() * m_baseRegionWidth), m_baseRegionWidth);
        int h = qMin(qRound((qreal)rc.height() / height() * m_baseRegionHeight), m_baseRegionHeight);
        QString text = QString("%1 %2*%3").arg(region->index() + 1).arg(w).arg(h);
        QFontMetrics fm(painter.font());
        int textHeight = fm.height();
        painter.drawText(region->x() + 10, region->y() + textHeight, text);
    }
}

void AnprDraw::mousePressEvent(QMouseEvent *event)
{
    m_pressed = true;
    m_pressPoint = event->pos();
    m_selectedRegion = regionUnderMouse(event->pos());
    if (m_selectedRegion) {
        m_operation = ModeOld;
        m_selectedRegion->setSelected(true);
        m_selectedRegion->raise();
        //处理z轴问题
        m_regionList.removeOne(m_selectedRegion);
        m_regionList.append(m_selectedRegion);
    } else {
        m_operation = ModeNew;
    }
    for (int i = m_regionList.size(); i > 0; --i) {
        AnprRegion *widget = m_regionList.at(i - 1);
        if (widget != m_selectedRegion) {
            widget->setSelected(false);
        }
        if (!m_selectedRegion && widget == m_editingRegion) {
            widget->setSelected(true);
        }
    }
    QWidget::mousePressEvent(event);
}

void AnprDraw::mouseReleaseEvent(QMouseEvent *event)
{
    m_pressed = false;
    m_operation = ModeNull;
    QWidget::mouseReleaseEvent(event);
}

void AnprDraw::mouseMoveEvent(QMouseEvent *event)
{
    if (m_pressed) {
        if (m_editingRegion == nullptr && m_selectedRegion == nullptr && m_regionList.size() < m_maxRegion) {
            int index = availableIndex();
            m_selectedRegion = new AnprRegion(index, this);
            m_selectedRegion->setSelected(true);
            m_selectedRegion->show();
            m_regionList.append(m_selectedRegion);

            m_editingRegion = m_selectedRegion;
        }
        if (m_editingRegion && m_operation == ModeNew) {
            QRect rc(m_pressPoint, event->pos());
            QRect normalRect = rc.normalized();
            int margin = m_editingRegion->margin();
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
            m_editingRegion->setGeometry(normalRect);
        }
    }

    update();
    QWidget::mouseMoveEvent(event);
}

AnprRegion *AnprDraw::regionUnderMouse(const QPoint &pos) const
{
    for (int i = m_regionList.size(); i > 0; --i) {
        AnprRegion *widget = m_regionList.at(i - 1);
        if (widget->isUnderMouse(pos)) {
            return widget;
        }
    }
    return nullptr;
}

int AnprDraw::availableIndex() const
{
    QMap<int, int> m_usedIndexMap;
    for (int i = 0; i < m_regionList.size(); ++i) {
        AnprRegion *region = m_regionList.at(i);
        m_usedIndexMap.insert(region->index(), 0);
    }
    for (int i = 0; i < m_maxRegion; ++i) {
        if (!m_usedIndexMap.contains(i)) {
            return i;
        }
    }
    return -1;
}
