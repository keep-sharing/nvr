#ifndef CUSTOMLAYOUTSCENE_H
#define CUSTOMLAYOUTSCENE_H

#include <QGraphicsScene>
#include <QMap>
#include "CustomLayoutInfo.h"

class CustomLayoutDialog;
class CustomLayoutRubberBand;

class CustomLayoutScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit CustomLayoutScene(QObject *parent = nullptr);

    int baseRow() const;
    int baseColumn() const;

    QString currentName() const;
    void setNewName(const QString &text);
    void clear();

    void setCustomLayoutInfo(const CustomLayoutInfo &info);
    CustomLayoutInfo customLayoutInfo() const;

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

private:
    void recalculateCellSize();

    void prepareMergeRect(const QRectF &rc);

protected slots:
    void onSceneRectChanged(const QRectF &rc);

private:
    CustomLayoutDialog *m_drawDialog = nullptr;

    qreal m_cellWidth = 0;
    qreal m_cellHeight = 0;

    CustomLayoutInfo m_info;

    QPointF m_pressPos;
    CustomLayoutRubberBand *m_rubberBand = nullptr;
};

#endif // CUSTOMLAYOUTSCENE_H
