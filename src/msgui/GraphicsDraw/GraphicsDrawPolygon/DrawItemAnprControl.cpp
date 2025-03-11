#include "DrawItemAnprControl.h"
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

const qreal BaseWidth = 1024.0;
const qreal BaseHeight = 1024.0;
DrawItemAnprControl::DrawItemAnprControl(QGraphicsItem *parent)
    : MsGraphicsObject(parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
    setFiltersChildEvents(true);
}
void DrawItemAnprControl::init()
{
    clearAll();
}

void DrawItemAnprControl::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton && isEnabled()) {
        return;
    }
    //点击空白区域 取消所有区域的选中状态
    if (m_regionList.count() == MAX_LEN_4) {
        if (!clickedRegion(event->scenePos())) {
            for (auto i : m_regionList) {
                i->setSelected(false);
                i->setEditable(false);
            }
            m_selectedRegion = nullptr;
        }
    }

    MsGraphicsObject::mousePressEvent(event);
}

void DrawItemAnprControl::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (clickedRegion(event->scenePos())) {
        return;
    }
    //如果当前不存在选择区域，或上个区域已经画完，且已画区域不超过4个开启画下一个区域
    if (((!m_selectedRegion) || m_selectedRegion->isFinished()) && m_regionList.count() < MAX_LEN_4) {
        m_selectedRegion = nullptr;
        startDraw();
    }
    event->accept();
}

bool DrawItemAnprControl::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    switch (event->type()) {
    case QEvent::GraphicsSceneMousePress: {
        QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);
        mousePressEvent(mouseEvent);
        break;
    }
    case QEvent::GraphicsSceneHoverMove: {
        QGraphicsSceneHoverEvent *hoverEvent = static_cast<QGraphicsSceneHoverEvent *>(event);
        hoverMoveEvent(hoverEvent);
        break;
    }
    default:
        break;
    }

    return MsGraphicsObject::sceneEventFilter(watched, event);
}

void DrawItemAnprControl::clearSelect()
{
    //删除未画完未添加到list里的多边形
    if (m_selectedRegion && !m_selectedRegion->isFinished()) {
        m_selectedRegion->clear();
        m_selectedRegion->hide();
        m_selectedRegion = nullptr;
    }
}

void DrawItemAnprControl::setSelectNull()
{
    m_selectedRegion = nullptr;
}

void DrawItemAnprControl::refreshstack()
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

bool DrawItemAnprControl::clickedRegion(QPointF point)
{
    bool hasRegion = false;
    QList<QGraphicsItem *> temp = scene()->items(point);
    for (auto *i : temp) {
        if (i->cursor().shape() != Qt::ArrowCursor) {
            hasRegion = true;
            break;
        }
    }
    return hasRegion;
}

QPointF DrawItemAnprControl::physicalPoint(const QPoint &p) const
{
    qreal x = p.x() / BaseWidth * sceneRect().width();
    qreal y = p.y() / BaseHeight * sceneRect().height();
    return QPointF(x, y);
}

QPoint DrawItemAnprControl::logicalPoint(const QPointF &p) const
{
    int x = qRound(p.x() / sceneRect().width() * BaseWidth);
    int y = qRound(p.y() / sceneRect().height() * BaseHeight);
    return QPoint(x, y);
}

void DrawItemAnprControl::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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

void DrawItemAnprControl::startDraw()
{
    m_selectedRegion = new DrawItemFacePolygon(this);
    m_selectedRegion->setIndex(findNewRegionIndex());
    m_selectedRegion->setRect(scene()->sceneRect());
    m_selectedRegion->setSelected(true);
    m_selectedRegion->setEditable(true);
    m_selectedRegion->setIsFinished(false);
    m_selectedRegion->setPointSizeRange(4, 4);
}

int DrawItemAnprControl::type() const
{
    return Type;
}

void DrawItemAnprControl::regionFinish()
{
    m_regionList.append(m_selectedRegion);
    refreshstack();
}

void DrawItemAnprControl::setItemEnable(bool enable)
{
    if (m_selectedRegion) {
        findCurrentItem();
        if (!m_selectedRegion->isFinished()) {
            m_selectedRegion->clear();
        }
        m_selectedRegion->setEditable(false);
        m_selectedRegion->setSelected(false);
    }
    //当前模式对应区域使能状态统一
    for (auto i : childItems()) {
        i->unsetCursor();
        if (enable) {
            DrawItemFacePolygon *item = static_cast<DrawItemFacePolygon *>(i);
            if (m_regionList.contains(item)) {
                i->setEnabled(enable);
            }
        } else {
            i->setEnabled(enable);
        }
    }
    for (auto iter : m_regionList) {
        iter->setEditable(false);
        iter->setSelected(false);
    }
    setEnabled(enable);
}

void DrawItemAnprControl::setCurrentItem(DrawItemFacePolygon *item)
{
    //设置当前区域为点击的已完成区域，
    for (auto i : m_regionList) {
        i->setSelected(false);
        i->setEditable(false);
    }
    m_selectedRegion = item;
    m_selectedRegion->setEditable(true);
    m_selectedRegion->setSelected(true);
}

void DrawItemAnprControl::setItemsSeleteFalse()
{
    //Polygon中发生点击事件时，取消其他所有区域的选中状态
    for (auto i : m_regionList) {
        i->setSelected(false);
        i->setEditable(false);
    }
}

int DrawItemAnprControl::findNewRegionIndex()
{
    int index = 0;
    for (int i = 0; i < 4; i++) {
        bool hasIndex = false;
        for (auto iter : m_regionList) {
            if (iter->index() == i) {
                hasIndex = true;
                break;
            }
        }
        if (!hasIndex) {
            index = i;
            break;
        }
    }
    return index;
}

void DrawItemAnprControl::findCurrentItem()
{
    //如果当前区域没有点，说明还没开始画，实际上的被选择区域并不是它
    if (m_selectedRegion && m_selectedRegion->isEmpty()) {
        m_selectedRegion->hide();
        m_selectedRegion = nullptr;
        for (auto iter : m_regionList) {
            if (iter->isFinished() && iter->isEditable()) {
                m_selectedRegion = iter;
                break;
            }
        }
    }
}

void DrawItemAnprControl::clearAll()
{
    if (m_selectedRegion && !m_selectedRegion->isFinished()) {
        m_selectedRegion->clear();
        m_selectedRegion->hide();
        m_selectedRegion = nullptr;
    }

    for (int i = 0; i < m_regionList.count(); i++) {
        DrawItemFacePolygon *item = m_regionList.at(i);
        item->hide();
    }
    m_regionList.clear();
    startDraw();
    update();
}

int DrawItemAnprControl::clear()
{
    int index = -1;
    if (!m_selectedRegion) {
        return index;
    }
    findCurrentItem();
    if (m_selectedRegion) {
        if (m_selectedRegion->isFinished()) {
            index = m_selectedRegion->index();
        }
        m_regionList.removeAll(m_selectedRegion);
        m_selectedRegion->hide();
        m_selectedRegion = nullptr;
    }
    update();
    return index;
}

void DrawItemAnprControl::clear(int index)
{
    for (auto iter : m_regionList) {
        if (iter->index() == index) {
            m_regionList.removeAll(iter);
            iter->hide();
            break;
        }
    }
    update();
}

void DrawItemAnprControl::showConflict()
{
    emit conflicted();
}

void DrawItemAnprControl::addShield(const MS_POLYGON &polygon, const int index)
{
    QVector<QPointF> points;
    QStringList xs = QString(polygon.polygonX).split(":", QString::SkipEmptyParts);
    QStringList ys = QString(polygon.polygonY).split(":", QString::SkipEmptyParts);
    for (int i = 0; i < xs.size(); ++i) {
        int x = xs.at(i).toInt();
        int y = ys.at(i).toInt();
        if (x < 0 || y < 0) {
            break;
        }
        points.append(physicalPoint(QPoint(x, y)));
    }
    if (points.isEmpty()) {
        return;
    }
    DrawItemFacePolygon *item = new DrawItemFacePolygon(this);
    item->setIndex(index);
    item->setPoints(points);
    item->setPointSizeRange(4, 4);
    m_regionList.append(item);
}

void DrawItemAnprControl::getShield(MS_POLYGON *polygon)
{
    if (m_selectedRegion && !m_selectedRegion->isFinished()) {
        m_selectedRegion->clear();
        m_selectedRegion->hide();
        m_selectedRegion = nullptr;
        for (auto iter : m_regionList) {
            if (iter->isFinished() && iter->isEditable()) {
                m_selectedRegion = iter;
                break;
            }
        }
    }
    //更新旧的区域位置
    updataShield(polygon);
    //添加新的区域
    for (auto iter : m_regionList) {
        QString xStr;
        QString yStr;
        const QVector<QPointF> &points = iter->scenePoints();
        for (int i = 0; i < 10; ++i) {
            if (i < points.size()) {
                const QPointF &p = points.at(i);
                const QPoint &lp = logicalPoint(p);
                xStr.append(QString("%1:").arg(lp.x()));
                yStr.append(QString("%1:").arg(lp.y()));
            } else {
                xStr.append("-1:");
                yStr.append("-1:");
            }
        }
        int index = iter->index();
        QStringList xs = QString(polygon[index].polygonX).split(":", QString::SkipEmptyParts);
        QStringList ys = QString(polygon[index].polygonY).split(":", QString::SkipEmptyParts);
        if (xs.isEmpty() || ys.isEmpty()) {
            continue;
        }
        if (xs.at(0).toInt() == -1 || ys.at(0).toInt() == -1) {
            snprintf(polygon[index].polygonX, sizeof(polygon[index].polygonX), "%s", xStr.toStdString().c_str());
            snprintf(polygon[index].polygonY, sizeof(polygon[index].polygonY), "%s", yStr.toStdString().c_str());
        }
    }
}

void DrawItemAnprControl::updataShield(MS_POLYGON *polygon)
{
    for (int i = 0; i < MAX_FACE_SHIELD; i++) {
        MS_POLYGON &shield = polygon[i];
        QStringList xs = QString(shield.polygonX).split(":", QString::SkipEmptyParts);
        QStringList ys = QString(shield.polygonY).split(":", QString::SkipEmptyParts);
        if (xs.isEmpty() || ys.isEmpty()) {
            break;
        }
        if (xs.at(0).toInt() > -1 && ys.at(0).toInt() > -1) {
            for (auto iter : m_regionList) {
                if (iter->index() == i) {
                    QString xStr;
                    QString yStr;
                    const QVector<QPointF> &points = iter->scenePoints();
                    for (int i = 0; i < 10; ++i) {
                        if (i < points.size()) {
                            const QPointF &p = points.at(i);
                            const QPoint &lp = logicalPoint(p);
                            xStr.append(QString("%1:").arg(lp.x()));
                            yStr.append(QString("%1:").arg(lp.y()));
                        } else {
                            xStr.append("-1:");
                            yStr.append("-1:");
                        }
                    }
                    snprintf(shield.polygonX, sizeof(shield.polygonX), "%s", xStr.toStdString().c_str());
                    snprintf(shield.polygonY, sizeof(shield.polygonY), "%s", yStr.toStdString().c_str());
                    break;
                }
            }
        }
    }
}
