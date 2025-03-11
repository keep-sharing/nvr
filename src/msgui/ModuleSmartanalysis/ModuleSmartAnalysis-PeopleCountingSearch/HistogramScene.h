#ifndef HISTOGRAMSCENE_H
#define HISTOGRAMSCENE_H

#include "BaseChartScene.h"

class HistogramScene : public BaseChartScene
{
    Q_OBJECT

    enum Mode
    {
        ModeHistogram,
        ModeHistogram2
    };

public:
    explicit HistogramScene(QObject *parent = 0);

    void showHistogram(const QList<int> &channels, int groupFilter = -1);
    void showHistogram2(const QList<int> &channels, int groupFilter = -1);

    void setChannelVisible(int channel, bool visible) override;

protected:
    void drawXAxis(QPainter *painter, const QRectF &rect) override;

    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    Mode m_mode = ModeHistogram;
};

#endif // HISTOGRAMSCENE_H
