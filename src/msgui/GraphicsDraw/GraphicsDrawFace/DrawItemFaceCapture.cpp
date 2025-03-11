#include "DrawItemFaceCapture.h"
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

const qreal BaseWidth = 1024.0;
const qreal BaseHeight = 1024.0;
DrawItemFaceCapture::DrawItemFaceCapture(QGraphicsItem *parent)
    : MsGraphicsObject(parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
    setFiltersChildEvents(true);

    m_minDetection = new DrawItemFaceMinDetection(this);
    m_detectionRegion = new DrawItemFacePolygon(this);
}

void DrawItemFaceCapture::init()
{
    m_minDetection->show();
    m_minDetection->setObjectSize(30);
    m_minDetection->setRect(scene()->sceneRect());

    m_detectionRegion->clear();
    clearAll();
}

void DrawItemFaceCapture::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton && isEnabled()) {
        return;
    }
    //点击空白区域 取消所有区域的选中状态
    if ((m_mode == DetectionMode && m_detectionRegion->isFinished()) || (m_mode == ShieldMode && m_regionList.count() == 4)) {
        if (!clickedRegion(event->scenePos())) {
            m_detectionRegion->setEditable(false);
            m_detectionRegion->setSelected(false);
            for (auto i : m_regionList) {
                i->setSelected(false);
                i->setEditable(false);
            }
            m_selectedRegion = nullptr;
        }
    }

    MsGraphicsObject::mousePressEvent(event);
}

void DrawItemFaceCapture::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (clickedRegion(event->scenePos())) {
        return;
    }
    if (m_mode == ShieldMode) {
        //如果当前不存在选择区域，或上个区域已经画完，且已画区域不超过4个开启画下一个区域
        if (((!m_selectedRegion) || m_selectedRegion->isFinished()) && m_regionList.count() < 4) {
            m_selectedRegion = nullptr;
            startDraw();
        }
        event->accept();
    }
}

bool DrawItemFaceCapture::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
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

void DrawItemFaceCapture::clearSelect()
{
    //删除未画完未添加到list里的多边形
    if (m_selectedRegion && !m_selectedRegion->isFinished()) {
        m_selectedRegion->clear();
        if (m_mode == ShieldMode) {
            m_selectedRegion->hide();
            m_selectedRegion = nullptr;
        }
    }
}

void DrawItemFaceCapture::setSelectNull()
{
    m_selectedRegion = nullptr;
}

void DrawItemFaceCapture::refreshstack()
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

bool DrawItemFaceCapture::clickedRegion(QPointF point)
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

QPointF DrawItemFaceCapture::physicalPoint(const QPoint &p) const
{
    qreal x = p.x() / BaseWidth * sceneRect().width();
    qreal y = p.y() / BaseHeight * sceneRect().height();
    return QPointF(x, y);
}

QPoint DrawItemFaceCapture::logicalPoint(const QPointF &p) const
{
    int x = qRound(p.x() / sceneRect().width() * BaseWidth);
    int y = qRound(p.y() / sceneRect().height() * BaseHeight);
    return QPoint(x, y);
}

void DrawItemFaceCapture::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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

void DrawItemFaceCapture::startDraw()
{
    m_selectedRegion = new DrawItemFacePolygon(this);
    m_selectedRegion->setIndex(findNewRegionIndex());
    m_selectedRegion->setRect(scene()->sceneRect());
    m_selectedRegion->setSelected(true);
    m_selectedRegion->setEditable(true);
    m_selectedRegion->setIsFinished(false);
}

void DrawItemFaceCapture::setMinSize(int value)
{
    m_minDetection->setObjectSize(value);
}

int DrawItemFaceCapture::type() const
{
    return Type;
}

void DrawItemFaceCapture::setFaceMode(DrawItemFaceCapture::FaceCaptureMode mode)
{
    m_mode = mode;
    switch (mode) {
    case DetectionMode:
        if (m_selectedRegion && !m_selectedRegion->isFinished()) {
            m_selectedRegion->clear();
            if (m_selectedRegion != m_detectionRegion) {
                m_selectedRegion->hide();
                m_selectedRegion = nullptr;
            }
        }
        for (auto i : m_regionList) {
            i->setSelected(false);
            i->setEditable(false);
            i->setIsEnable(false);
            if (!i->isFinished()) {
                i->clear();
            }
        }
        if (m_detectionRegion->isEmpty()) {
            m_detectionRegion->setIsFinished(false);
        }
        m_detectionRegion->setIsEnable(true && isEnabled());
        m_detectionRegion->setZValue(1);
        m_selectedRegion = m_detectionRegion;
        if (isEnabled()) {
            setCurrentItem(m_selectedRegion);
        }
        break;
    case ShieldMode:
        if (!m_detectionRegion->isFinished()) {
            m_detectionRegion->clear();
        }
        m_detectionRegion->setSelected(false);
        m_detectionRegion->setEditable(false);
        m_detectionRegion->setIsEnable(false);
        m_detectionRegion->setIsFinished(true);
        m_detectionRegion->setZValue(0);
        for (auto i : m_regionList) {
            i->setSelected(true);
            i->setIsEnable(true && isEnabled());
        }
        m_selectedRegion = nullptr;
        break;
    }
}

void DrawItemFaceCapture::regionFinish()
{
    //当前绘制的区域绘制完毕，加入list
    if (m_mode == ShieldMode) {
        m_regionList.append(m_selectedRegion);
        refreshstack();
    }
}

void DrawItemFaceCapture::setItemEnable(bool enable)
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
            if ((m_mode == DetectionMode && i == m_detectionRegion) || (m_mode == ShieldMode && m_regionList.contains(item))) {
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
    if (enable && m_mode == DetectionMode) {
        m_selectedRegion = m_detectionRegion;
        setCurrentItem(m_selectedRegion);
    }
    setEnabled(enable);
}

void DrawItemFaceCapture::setCurrentItem(DrawItemFacePolygon *item)
{
    //设置当前区域为点击的已完成区域，
    m_detectionRegion->setEditable(false);
    m_detectionRegion->setSelected(false);
    for (auto i : m_regionList) {
        i->setSelected(false);
        i->setEditable(false);
    }
    m_selectedRegion = item;
    m_selectedRegion->setEditable(true);
    m_selectedRegion->setSelected(true);
}

void DrawItemFaceCapture::setItemsSeleteFalse()
{
    //Polygon中发生点击事件时，取消其他所有区域的选中状态
    if (m_mode == ShieldMode) {
        for (auto i : m_regionList) {
            i->setSelected(false);
            i->setEditable(false);
        }
    } else {
        m_detectionRegion->setSelected(false);
        m_detectionRegion->setEditable(false);
    }
}

int DrawItemFaceCapture::findNewRegionIndex()
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

void DrawItemFaceCapture::findCurrentItem()
{
    //如果当前区域没有点，说明还没开始画，实际上的被选择区域并不是它
    if (m_selectedRegion && m_selectedRegion->isEmpty() && m_mode == ShieldMode) {
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

void DrawItemFaceCapture::clearAll()
{
    if (m_selectedRegion && !m_selectedRegion->isFinished() && m_mode == ShieldMode) {
        m_selectedRegion->clear();
        m_selectedRegion->hide();
        m_selectedRegion = nullptr;
    }

    for (int i = 0; i < m_regionList.count(); i++) {
        DrawItemFacePolygon *item = m_regionList.at(i);
        item->hide();
    }
    m_regionList.clear();
    if (m_mode == ShieldMode) {
        startDraw();
    }
    update();
}

int DrawItemFaceCapture::clear()
{
    int index = -1;
    if (m_mode == ShieldMode && !m_selectedRegion) {
        return index;
    }
    findCurrentItem();
    switch (m_mode) {
    case DetectionMode:
        m_detectionRegion->clear();
        break;
    case ShieldMode:
        if (m_selectedRegion) {
            if (m_selectedRegion->isFinished()) {
                index = m_selectedRegion->index();
            }
            m_regionList.removeAll(m_selectedRegion);
            m_selectedRegion->hide();
            m_selectedRegion = nullptr;
        }
        break;
    }
    update();
    return index;
}

void DrawItemFaceCapture::clear(int index)
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

void DrawItemFaceCapture::detectionSetAll()
{
    QVector<QPointF> points;
    int width = static_cast<int>(sceneWidth());
    int height = static_cast<int>(sceneHeight());
    points.append(QPoint(0, 0));
    points.append(QPoint(width, 0));
    points.append(QPoint(width, height));
    points.append(QPoint(0, height));

    m_detectionRegion->setPoints(points);
}

void DrawItemFaceCapture::showConflict()
{
    emit conflicted();
}

void DrawItemFaceCapture::setDetection(const QString &xList, const QString &yList)
{
    QVector<QPointF> points;

    QStringList xs = xList.split(":", QString::SkipEmptyParts);
    QStringList ys = yList.split(":", QString::SkipEmptyParts);
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
    m_detectionRegion->setPoints(points);
    if (m_detectionRegion->isFinished()) {
        m_detectionRegion->setEditable(false);
        m_detectionRegion->setSelected(false);
    }
}

void DrawItemFaceCapture::getDetection(MS_POLYGON *polygon)
{
    QString xStr;
    QString yStr;
    const QVector<QPointF> &points = m_detectionRegion->scenePoints();
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
    snprintf(polygon->polygonX, sizeof(polygon->polygonX), "%s", xStr.toStdString().c_str());
    snprintf(polygon->polygonY, sizeof(polygon->polygonY), "%s", yStr.toStdString().c_str());
}

void DrawItemFaceCapture::addShield(const MS_POLYGON &polygon, const int index)
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
    m_regionList.append(item);
}

void DrawItemFaceCapture::getShield(MS_FACE_SHIELD *polygon)
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
        QStringList xs = QString(polygon[index].region.polygonX).split(":", QString::SkipEmptyParts);
        QStringList ys = QString(polygon[index].region.polygonY).split(":", QString::SkipEmptyParts);
        if (xs.isEmpty() || ys.isEmpty()) {
            continue;
        }
        if (xs.at(0).toInt() == -1 || ys.at(0).toInt() == -1) {
            snprintf(polygon[index].region.polygonX, sizeof(polygon[index].region.polygonX), "%s", xStr.toStdString().c_str());
            snprintf(polygon[index].region.polygonY, sizeof(polygon[index].region.polygonY), "%s", yStr.toStdString().c_str());
            polygon[index].enable = true;
        }
    }
}

void DrawItemFaceCapture::updataShield(MS_FACE_SHIELD *polygon)
{
    for (int i = 0; i < MAX_FACE_SHIELD; i++) {
        MS_FACE_SHIELD &shield = polygon[i];
        QStringList xs = QString(shield.region.polygonX).split(":", QString::SkipEmptyParts);
        QStringList ys = QString(shield.region.polygonY).split(":", QString::SkipEmptyParts);
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
                    snprintf(shield.region.polygonX, sizeof(shield.region.polygonX), "%s", xStr.toStdString().c_str());
                    snprintf(shield.region.polygonY, sizeof(shield.region.polygonY), "%s", yStr.toStdString().c_str());
                    break;
                }
            }
        }
    }
}
