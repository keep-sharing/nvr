#ifndef DRAWSCENEPOLYGON_H
#define DRAWSCENEPOLYGON_H

#include <QGraphicsScene>

class DrawItem_Polygon;

class DrawScenePolygon : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit DrawScenePolygon(QObject *parent = nullptr);

    void setPolygon(const QString &xList, const QString &yList);
    void getPolygon(char *xList,int xSize, char *yList, int ySize);

    void selectAll();
    void clearAll();
    void clearPolygon();

    void showConflict();

    bool isFinished();

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    QPointF physicalPoint(const QPoint &p) const;
    QPoint logicalPoint(const QPointF &p) const;

    virtual DrawItem_Polygon *currentItem();

signals:
    void conflicted();

protected slots:
    virtual void onSceneRectChanged(const QRectF &rect);

private:
    DrawItem_Polygon *m_currentItem = nullptr;
};

#endif // DRAWSCENEPOLYGON_H
