#ifndef LINESCENE_H
#define LINESCENE_H

#include "BaseChartScene.h"

class LineScene : public BaseChartScene {
    Q_OBJECT

public:
    explicit LineScene(QObject *parent = 0);

    void showLine(const QList<int> &channels, int groupFilter = -1);

    void setChannelVisible(int channel, bool visible) override;

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    QMap<int, QColor> m_colorMap;

    //key: channel
    QMap<int, QVector<QPointF>> m_pointsMap;
};

#endif // LINESCENE_H
