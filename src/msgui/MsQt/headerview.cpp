#include "headerview.h"
#include <QMouseEvent>
#include <QPainter>
#include <QtDebug>

HeaderView::HeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
{
    setClickable(true);

    m_uncheckedPixmap = QPixmap(":/common/common/checkbox-unchecked.png").scaled(18, 18, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_checkedPixmap = QPixmap(":/common/common/checkbox-checked.png").scaled(18, 18, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_partiallycheckedPixmap = QPixmap(":/common/common/checkbox-partiallychecked.png").scaled(18, 18, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    m_uncheckedDisablePixmap = QPixmap(":/common/common/checkbox-unchecked-disable.png").scaled(18, 18, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_checkedDisablePixmap = QPixmap(":/common/common/checkbox-checked-disable.png").scaled(18, 18, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_partiallycheckedDisablePixmap = QPixmap(":/common/common/checkbox-partiallychecked-disable.png").scaled(18, 18, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

void HeaderView::setCheckable(bool checkable)
{
    m_checkable = checkable;
    updateSection(0);
}

void HeaderView::setCheckState(Qt::CheckState state)
{
    m_checkState = state;
    updateSection(0);
}

void HeaderView::setSortableForColumn(int column, int enable)
{
    m_sortableMap.insert(column, enable);
}

bool HeaderView::sortableForColumn(int column)
{
    if (m_sortableMap.contains(column)) {
        return m_sortableMap.value(column);
    }
    return true;
}

void HeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();

    if (logicalIndex == 0 && m_checkable) {
        QRect rc = rect;
        rc.setSize(m_uncheckedPixmap.size());
        rc.moveCenter(rect.center());
        switch (m_checkState) {
        case Qt::Checked:
            if (isEnabled()) {
                painter->drawPixmap(rc, m_checkedPixmap);
            } else {
                painter->drawPixmap(rc, m_checkedDisablePixmap);
            }
            break;
        case Qt::Unchecked:
            if (isEnabled()) {
                painter->drawPixmap(rc, m_uncheckedPixmap);
            } else {
                painter->drawPixmap(rc, m_uncheckedDisablePixmap);
            }
            break;
        case Qt::PartiallyChecked:
            if (isEnabled()) {
                painter->drawPixmap(rc, m_partiallycheckedPixmap);
            } else {
                painter->drawPixmap(rc, m_partiallycheckedDisablePixmap);
            }
            break;
        }
    }
}

void HeaderView::mousePressEvent(QMouseEvent *event)
{
    m_pressedPoint = event->pos();
    return QHeaderView::mousePressEvent(event);
}

void HeaderView::mouseReleaseEvent(QMouseEvent *event)
{
    int index = logicalIndexAt(event->pos());
    //
    if (index == 0 && m_checkable) {
        if ((event->pos() - m_pressedPoint).manhattanLength() < 3) {
            switch (m_checkState) {
            case Qt::PartiallyChecked:
            case Qt::Checked:
                m_checkState = Qt::Unchecked;
                break;
            case Qt::Unchecked:
                m_checkState = Qt::Checked;
                break;
            }
            updateSection(0);
            emit headerChecked(m_checkState);
        }
        return;
    }
    //
    return QHeaderView::mouseReleaseEvent(event);
}
