#ifndef VIDEOTRACK_H
#define VIDEOTRACK_H

#include "MsWidget.h"
#include <QMap>

class QTimer;

class VideoTrack : public MsWidget
{
    Q_OBJECT
public:

    enum Mode
    {
        ModeVca,
        ModePtz
    };

    struct TrackInfo
    {
        bool vaild = false;
        int id;
        QColor lineColor;
        QRect rc;
        QVector<QPoint> trackPoints;
    };

    explicit VideoTrack(QWidget *parent = 0);

    void setCurrentChannel(int channel);
    bool isTrackEnable();
    void setTrackEnable(bool enable);
    void clear();

    void setMode(Mode mode);

    void processMessage(MessageReceive *message) override;

protected:
    void ON_RESPONSE_FLAG_GET_VAC_PERSONPOINT(MessageReceive *message);

    void paintEvent(QPaintEvent *) override;
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;

private:
    QPoint physicalPos(int x, int y);

signals:

public slots:

private slots:
    void onTrackTimer();

private:
    int m_channel = 0;
    QTimer *m_trackTimer = nullptr;
    bool m_isTrackEnable = false;

    int m_realWidth = 0;
    int m_realHeight = 0;

    QColor m_rcColor;
    QColor m_pointColor;
    QList<QColor> m_lineColorList;

    QMap<int, TrackInfo> m_trackInfoMap;

    Mode m_trackMode = ModeVca;
};

#endif // VIDEOTRACK_H
