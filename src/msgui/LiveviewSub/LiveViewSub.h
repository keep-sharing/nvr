#ifndef LIVEVIEWSUB_H
#define LIVEVIEWSUB_H

#include <QWidget>
#include "LiveLayout.h"
#include "TimeWidget.h"

class LiveVideo;
class LiveLayout;
class CustomLayoutInfo;

struct VacDynamicBoxALL;
struct RegionalRectInfo;
struct RegionalAlarmInfo;

namespace Ui {
class LiveViewSub;
}

class LiveViewSub : public QWidget
{
    Q_OBJECT
public:
    explicit LiveViewSub(QWidget *parent = nullptr);
    ~LiveViewSub();

    static LiveViewSub *instance();

    static bool isSubEnable();

    void setGeometry(const QRect &rc);

    LiveVideo *liveVideo(int channel);
    void setLayoutMode(const CustomLayoutInfo &info, int page);
    void showPage(int page, int count);

    void showVcaRects(VacDynamicBoxALL *info);
    void showRegionRects(RegionalRectInfo *info);
    void showRegionAlarm(const RegionalAlarmInfo &info);

    //
    void setPopupVideo(int screen, int layout, const QList<int> &channels);
    void setTimeInfoMode(int mode);
    void resetTimeInfoMode();

    void updateDisplayInfo();
    void updateStreamInfo(int channel);
    void updateMotion(int channel);
    void updateRecord(int channel);
    void updateVideoLoss(int channel);
    void showNoResource(int channel);
    void updateAnprEvent(int channel);
    void updateSmartEvent(int channel);

    //
    void clearAllPosData();

protected:
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;

private slots:
    void onShowPage(int page, int count);

private:
    void initializeVideo();

    LiveVideo *makeNewVideo(int channel);
    LiveVideo *videoFromChannel(int channel);

signals:

public slots:

private:
    static LiveViewSub *s_liveViewSub;

    Ui::LiveViewSub *ui;

    //channel, video
    QMap<int, LiveVideo *> m_mapChannelVideo;

    //
    TimeWidget *m_timeWidget = nullptr;
};

#endif // LIVEVIEWSUB_H
