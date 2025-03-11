#ifndef TRACKINFO_H
#define TRACKINFO_H

#include <QColor>
#include <QRectF>
#include <QVector>

enum TrackMode {
    TrackModeNone,
    TrackModeVca,
    TrackModePtz,
    TrackModeFisheye
};

class TrackInfo {
public:
    TrackInfo();

    bool vaild = false;
    int id = -1;
    QColor rcColor = QColor(24, 190, 18);
    QColor pointColor = QColor(255, 0, 0);
    QColor lineColor;
    QRectF rc;
    QVector<QPointF> trackPoints;
    TrackMode mode;
};

#endif // TRACKINFO_H
