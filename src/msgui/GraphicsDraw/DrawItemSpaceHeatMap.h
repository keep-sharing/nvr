#ifndef DRAWITEMSPACEHEATMAP_H
#define DRAWITEMSPACEHEATMAP_H

#include <QGraphicsRectItem>

class DrawItemSpaceHeatMap : public QGraphicsRectItem
{
public:
    DrawItemSpaceHeatMap(QGraphicsItem *parent = nullptr);

    void showHeatMap(int max, int min, QImage colorImage, QImage heatmapImage);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    int m_max = 0;
    int m_min = 0;

    QImage m_colorImage;
    QImage m_heatMapImage;
};

#endif // DRAWITEMSPACEHEATMAP_H
