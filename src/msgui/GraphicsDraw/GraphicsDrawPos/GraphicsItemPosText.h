#ifndef GRAPHICSITEMPOSTEXT_H
#define GRAPHICSITEMPOSTEXT_H

#include "GraphicsItem.h"
#include "GraphicsTextItem.h"
#include "PosData.h"
#include <QGraphicsObject>
#include <QGraphicsTextItem>

class GraphicsItemPosText : public QGraphicsObject {
    Q_OBJECT

public:
    enum UpdateBehavior {
        NoPartialUpdate, //完全刷新，用于回放
        PartialUpdate    //增量更新，用于预览，提高性能
    };

    explicit GraphicsItemPosText(QGraphicsItem *parent = nullptr);

    void setRect(const QRectF &rc);
    void setPosData(const PosData &data);
    void setPaused(bool pause);
    void clear();

    void setUpdateBehavior(UpdateBehavior newUpdateBehavior);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    void hideAllItem();

private slots:
    void onTimeout();

private:
    QRectF m_rect;
    QList<GraphicsTextItem *> m_listBlockItem;

    PosData m_posData;
    QRectF m_posRect;
    //缓存上一次数据，用来对比改变
    QStringList m_listText;

    //100毫秒为单位，50*100=5秒
    QTimer *m_timeout     = nullptr;
    int m_timeoutValue    = 0;
    int m_timeoutInterval = 50;

    UpdateBehavior m_updateBehavior = PartialUpdate;
};

#endif // GRAPHICSITEMPOSTEXT_H
