#include "CustomLayoutScene.h"
#include "CustomLayoutDialog.h"
#include "CustomLayoutRubberBand.h"
#include "MsDevice.h"
#include "MyDebug.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

CustomLayoutScene::CustomLayoutScene(QObject *parent)
    : QGraphicsScene(parent)
{
    connect(this, SIGNAL(sceneRectChanged(QRectF)), this, SLOT(onSceneRectChanged(QRectF)));

    m_drawDialog = static_cast<CustomLayoutDialog *>(parent);

    m_rubberBand = new CustomLayoutRubberBand();
    addItem(m_rubberBand);
    m_rubberBand->hide();
}

int CustomLayoutScene::baseRow() const
{
    return m_info.baseRow();
}

int CustomLayoutScene::baseColumn() const
{
    return m_info.baseColumn();
}

QString CustomLayoutScene::currentName() const
{
    return m_info.name();
}

void CustomLayoutScene::setNewName(const QString &text)
{
    m_info.setName(text);
}

void CustomLayoutScene::clear()
{
    m_info.clear();
    update(sceneRect());
}

void CustomLayoutScene::setCustomLayoutInfo(const CustomLayoutInfo &info)
{
    m_info = info;

    qMsDebug() << QString("baseRow: %1, baseColumn %2, count: %3")
                      .arg(m_info.baseRow())
                      .arg(m_info.baseColumn())
                      .arg(m_info.positions().count());

    recalculateCellSize();

    update(sceneRect());
}

CustomLayoutInfo CustomLayoutScene::customLayoutInfo() const
{
    return m_info;
}

void CustomLayoutScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect)

    //
    if (!m_info.isValid()) {
        return;
    }
    painter->setRenderHints(QPainter::Antialiasing);
    int index = 0;
    const auto &positionsMap = m_info.positions();
    for (auto iter = positionsMap.constBegin(); iter != positionsMap.constEnd(); ++iter) {
        painter->save();
        painter->setPen(QPen(QColor(185, 185, 185), 1));
        QRectF rc = iter.value();
        painter->drawRect(rc);
        painter->restore();
        //
        if (index < qMsNvr->maxChannel()) {
            painter->save();
            QFont font = painter->font();
            font.setPixelSize(16);
            painter->setFont(font);
            painter->setPen(QColor(74, 74, 74));
            painter->drawText(rc, Qt::AlignCenter, QString::number(index + 1));
            painter->restore();
        }
        ++index;
    }
}

void CustomLayoutScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_pressPos = event->scenePos();
    m_rubberBand->setRect(QRect());
    m_rubberBand->show();
}

void CustomLayoutScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)

    QRectF rc = m_rubberBand->rect().toRect();
    m_rubberBand->hide();
    if (rc.width() < 1) {
        rc.setWidth(1);
    }
    if (rc.height() < 1) {
        rc.setHeight(1);
    }
    prepareMergeRect(rc);
}

void CustomLayoutScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF p = event->scenePos();
    if (p.x() < sceneRect().left()) {
        p.setX(sceneRect().left());
    }
    if (p.x() > sceneRect().right()) {
        p.setX(sceneRect().right());
    }
    if (p.y() < sceneRect().top()) {
        p.setY(sceneRect().top());
    }
    if (p.y() > sceneRect().bottom()) {
        p.setY(sceneRect().bottom());
    }
    m_rubberBand->setRect(QRectF(m_pressPos, p).normalized());
}

void CustomLayoutScene::recalculateCellSize()
{
    m_cellWidth = sceneRect().width() / m_info.baseColumn();
    m_cellHeight = sceneRect().height() / m_info.baseRow();

    const auto &positionsMap = m_info.positions();
    for (auto iter = positionsMap.constBegin(); iter != positionsMap.constEnd(); ++iter) {
        VideoPosition pos = iter.key();
        QRectF rect;
        rect.setLeft(m_cellWidth * pos.column);
        rect.setTop(m_cellHeight * pos.row);
        rect.setRight(rect.left() + m_cellWidth * pos.columnSpan);
        rect.setBottom(rect.top() + m_cellHeight * pos.rowSpan);
        m_info.insertPosition(pos, rect);
    }
}

void CustomLayoutScene::prepareMergeRect(const QRectF &rc)
{
    CustomLayoutInfo info = m_info;
    auto positionsMap = info.positions();

    int count = 0;
    //计算最小包围矩形，遍历直到没有交叉的矩形
    QRectF realRect = rc;
    bool ok = true;
    while (ok) {
        ok = false;
        for (auto iter = positionsMap.begin(); iter != positionsMap.end();) {
            QRectF tempRc = iter.value();
            QRectF intersectedRc = tempRc & realRect;
            //兼容 qreal减法乘法精度误差，0.001像素以下视为无改动
            if (intersectedRc.isValid() && (intersectedRc.width() > qreal(0.001) && intersectedRc.height() > qreal(0.001))) {
                realRect |= tempRc;
                ok = true;
                ++count;
                iter = positionsMap.erase(iter);
            } else {
                ++iter;
            }
        }
    }
    if (count == 0) {
        qMsDebug() << "no need merge";
    } else {
        //添加新矩形
        VideoPosition p;
        p.row = qRound(realRect.top() / m_cellHeight);
        p.column = qRound(realRect.left() / m_cellWidth);
        p.rowSpan = qRound(realRect.height() / m_cellHeight);
        p.columnSpan = qRound(realRect.width() / m_cellWidth);
        positionsMap.insert(p, realRect);
        //
        info.setPositions(positionsMap);

        if (positionsMap != m_info.positions()) {
            m_drawDialog->pushInfo(info);
        } else {
            qMsDebug() << "no need merge";
        }
    }
}

void CustomLayoutScene::onSceneRectChanged(const QRectF &rc)
{
    Q_UNUSED(rc)

    recalculateCellSize();
}
