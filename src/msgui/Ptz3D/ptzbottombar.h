#ifndef PTZBOTTOMBAR_H
#define PTZBOTTOMBAR_H

#include "MsWidget.h"

extern "C" {
#include "msg.h"
}

class MessageReceive;

namespace Ui {
class PtzBottomBar;
}

class PtzBottomBar : public MsWidget {
    Q_OBJECT

public:
    explicit PtzBottomBar(QWidget *parent = nullptr);
    ~PtzBottomBar() override;
    void hideEvent(QHideEvent *) override;
    void showEvent(QShowEvent *) override;

    static PtzBottomBar *instance();

    void initializeData(int channel);

    //ptz 3d
    void setPtzButtonChecked(bool checked);
    //
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_AUTO_TRACKING(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_PTZ_PANEL_STATUS(MessageReceive *message);

    void setPtzManualTrackButtonEnabled(bool enabled);
    void setPtzManualTrackButtonChecked(bool checked);
    void stopStatusTimer();
    void startStatusTimer();

private slots:
    void onLanguageChanged();
    //time
    void onStatusTimeout();
    //ptz
    void on_toolButton_3d_clicked(bool checked);
    void on_toolButton_manualTrack_clicked(bool checked);
    void on_toolButton_ptz_clicked(bool checked);
    void on_toolButton_closePtz3D_clicked();

    void on_toolButtonOneTouchPatrol_clicked();
    void on_toolButtonDehumidifying_clicked();
    void on_toolButtonAutoHome_clicked();
    void on_toolButtonWiper_clicked();

  private:
    static PtzBottomBar *s_self;
    Ui::PtzBottomBar *ui;
    int m_channel;

    ms_auto_tracking m_auto_tracking;
    QTimer *statusTimer = nullptr;
};

#endif // PTZBOTTOMBAR_H
