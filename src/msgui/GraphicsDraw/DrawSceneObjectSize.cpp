#include "DrawSceneObjectSize.h"
#include "DrawItemObjectSize.h"
#include <QGraphicsSceneMouseEvent>
#include "MyDebug.h"

extern "C" {
#include "msg.h"
}

const int LOGIC_MAX_WIDTH = 320;
const int LOGIC_MAX_HEIGHT = 240;
//从8.0.3-r9开始使用新的尺寸
const int LOGIC_MAX_WIDTH_NEW = 608;
const int LOGIC_MAX_HEIGHT_NEW = 352;
DrawSceneObjectSize::DrawSceneObjectSize(QObject *parent)
    : QGraphicsScene(parent)
{
    m_item = new DrawItemObjectSize();
    addItem(m_item);
    m_item->hide();
}

void DrawSceneObjectSize::showMinimumObjectSize(ms_vca_settings_info2 *info, MsQtVca::ObjectVcaType type, int sizeType)
{
    m_logicSizeType = static_cast<LogicSizeType>(sizeType);
    QRectF rc(physicalPos(info->minobject_window_rectangle_x[type], info->minobject_window_rectangle_y[type]),
              physicalSize(info->minobject_window_rectangle_width[type], info->minobject_window_rectangle_height[type]));
    rc.moveCenter(sceneRect().center());
    m_item->setObjectRect(rc, "Min", MsQtVca::MinSize);
    m_item->show();
}

void DrawSceneObjectSize::showMaximumObjectSize(ms_vca_settings_info2 *info, MsQtVca::ObjectVcaType type, int sizeType)
{
    m_logicSizeType = static_cast<LogicSizeType>(sizeType);
    QRectF rc(physicalPos(info->maxobject_window_rectangle_x[type], info->maxobject_window_rectangle_y[type]),
              physicalSize(info->maxobject_window_rectangle_width[type], info->maxobject_window_rectangle_height[type]));
    rc.moveCenter(sceneRect().center());
    m_item->setObjectRect(rc, "Max", MsQtVca::MaxSize);
    m_item->show();
}

void DrawSceneObjectSize::showMinimumObjectSize(MsIpcRegionalPeople *regionInfo, int sizeType)
{
    m_logicSizeType = static_cast<LogicSizeType>(sizeType);
    QRectF rc(physicalPos(regionInfo->minObjX, regionInfo->minObjY), physicalSize(regionInfo->minObjWidth, regionInfo->minObjHeight));
    rc.moveCenter(sceneRect().center());
    m_item->setObjectRect(rc, "Min", MsQtVca::MinSize);
    m_item->show();
}

void DrawSceneObjectSize::showMaximumObjectSize(MsIpcRegionalPeople *regionInfo, int sizeType)
{
    m_logicSizeType = static_cast<LogicSizeType>(sizeType);
    QRectF rc(physicalPos(regionInfo->maxObjX, regionInfo->maxObjY), physicalSize(regionInfo->maxObjWidth, regionInfo->maxObjHeight));
    rc.moveCenter(sceneRect().center());
    m_item->setObjectRect(rc, "Max", MsQtVca::MaxSize);
    m_item->show();
}

void DrawSceneObjectSize::showMinimumObjectSize(int x, int y, int w, int h, int sizeType)
{
    m_logicSizeType = static_cast<LogicSizeType>(sizeType);
    QRectF rc(physicalPos(x, y), physicalSize(w, h));
    rc.moveCenter(sceneRect().center());
    m_item->setObjectRect(rc, "Min", MsQtVca::MinSize);
    m_item->show();
}

void DrawSceneObjectSize::showMaximumObjectSize(int x, int y, int w, int h, int sizeType)
{
    m_logicSizeType = static_cast<LogicSizeType>(sizeType);
    QRectF rc(physicalPos(x, y), physicalSize(w, h));
    rc.moveCenter(sceneRect().center());
    m_item->setObjectRect(rc, "Max", MsQtVca::MaxSize);
    m_item->show();
}

void DrawSceneObjectSize::updateSize(MsQtVca::ObjectSizeType sizeType)
{
    const QRectF &rc = m_item->objectRect();
    int x = logicalWidth(rc.x());
    int y = logicalHeight(rc.y());
    int w = logicalWidth(rc.width());
    int h = logicalHeight(rc.height());
    emit objectSizeChanged(QRect(x, y, w, h), sizeType);

    update(sceneRect());
}

void DrawSceneObjectSize::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (itemAt(event->scenePos())) {
        QGraphicsScene::mousePressEvent(event);
    } else {
        event->accept();
    }
}

/**
 * @brief DrawScene_ObjectSize::physicalPos
 * ipc传来的大小转换为qt实际绘制大小
 * @param x
 * @param y
 * @return
 */
QPointF DrawSceneObjectSize::physicalPos(qreal x, qreal y)
{
    return QPointF(physicalWidth(x), physicalHeight(y));
}

QSizeF DrawSceneObjectSize::physicalSize(qreal w, qreal h)
{
    return QSizeF(physicalWidth(w), physicalHeight(h));
}

qreal DrawSceneObjectSize::physicalWidth(qreal w)
{
    //qMsDebug() << QString("%1 / %2 * %3").arg(w).arg(LOGIC_MAX_WIDTH).arg(sceneRect().width());
    if (m_logicSizeType == Before_8_0_3_r9) {
        return w / LOGIC_MAX_WIDTH * sceneRect().width();
    } else {
        return w / LOGIC_MAX_WIDTH_NEW * sceneRect().width();
    }
}

qreal DrawSceneObjectSize::physicalHeight(qreal h)
{
    if (m_logicSizeType == Before_8_0_3_r9) {
        return h / LOGIC_MAX_HEIGHT * sceneRect().height();
    }else {
        return h / LOGIC_MAX_HEIGHT_NEW * sceneRect().height();
    }
}

/**
 * @brief DrawScene_ObjectSize::logicalPos
 * qt绘制的大小转换为传到ipc的实际大小
 * @param pos
 * @return
 */
QPoint DrawSceneObjectSize::logicalPos(const QPointF &pos)
{
    return QPoint(logicalWidth(pos.x()), logicalHeight(pos.y()));
}

int DrawSceneObjectSize::logicalWidth(qreal w)
{
    if (m_logicSizeType == Before_8_0_3_r9) {
        return qRound(w / sceneRect().width() * LOGIC_MAX_WIDTH);
    } else {
        return qRound(w / sceneRect().width() * LOGIC_MAX_WIDTH_NEW);
    }
}

int DrawSceneObjectSize::logicalHeight(qreal h)
{
    if (m_logicSizeType == Before_8_0_3_r9) {
        return qRound(h / sceneRect().height() * LOGIC_MAX_HEIGHT);
    } else {
        return qRound(h / sceneRect().height() * LOGIC_MAX_HEIGHT_NEW);
    }
}
