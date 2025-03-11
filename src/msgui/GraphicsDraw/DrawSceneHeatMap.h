#ifndef DRAWSCENEHEATMAP_H
#define DRAWSCENEHEATMAP_H

#include <QDateTime>
#include <QGraphicsScene>

class DrawItemSpaceHeatMap;
class DrawItemTimeHeatMap;

struct ImageParameters {
    int max;
    int min;
    QImage colorImage;
    QImage heatmapImage;
};

class DrawSceneHeatMap : public QGraphicsScene {
    Q_OBJECT
public:
    explicit DrawSceneHeatMap(QObject *parent = nullptr);

    void clearInfo();

    void showSpaceHeatMap();
    void showSpaceHeatMap(const ImageParameters &imageParameters);
    void showTimeHeatMap();
    void showTimeHeatMap(const QString &text, int reportType, const QDateTime &dateTime);

    bool saveTimeHeatMap(const QString &fileName);
    bool saveTimeHeatMap(const QString &text, int reportType, const QDateTime &dateTime, const QString &fileName);

signals:

private:
    DrawItemSpaceHeatMap *m_spaceItem = nullptr;
    DrawItemTimeHeatMap *m_timeItem = nullptr;
};

#endif // DRAWSCENEHEATMAP_H
