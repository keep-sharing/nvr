#ifndef MSGRAPHICSOBJECT_H
#define MSGRAPHICSOBJECT_H

#include <DrawItemFacePolygon.h>
#include <QGraphicsObject>

class MessageReceive;

class MsGraphicsObject : public QGraphicsObject {
    Q_OBJECT
public:
    explicit MsGraphicsObject(QGraphicsItem *parent = nullptr);

    virtual void processMessage(MessageReceive *message);
    virtual void filterMessage(MessageReceive *message);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    virtual void setItemsSeleteFalse();
    virtual void setCurrentItem(DrawItemFacePolygon *item);
    virtual void showConflict();
    virtual void regionFinish();

protected:
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;

    //
    qreal sceneWidth() const;
    qreal sceneHeight() const;
    QRectF sceneRect() const;

    //
    void setIpcMaxSize(int w, int h);
    QPointF mapToQtPos(int x, int y);
    QPoint mapToIpcPos(const QPointF &pos);

signals:

private slots:
    void onSceneRectChanged(const QRectF &rect);

private:
    int m_ipcMaxWidth = 0;
    int m_ipcMaxHeight = 0;

    QGraphicsScene *m_scene = nullptr;
    QRectF m_rect;
};

#endif // MSGRAPHICSOBJECT_H
