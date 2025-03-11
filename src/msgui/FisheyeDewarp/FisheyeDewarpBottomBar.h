#ifndef FISHEYEDEWARPBOTTOMBAR_H
#define FISHEYEDEWARPBOTTOMBAR_H

#include "FisheyePanel.h"
#include <QEventLoop>
#include "MsWidget.h"

extern "C" {
#include "msg.h"
}

class MessageReceive;

namespace Ui {
class FisheyeDewarpBottomBar;
}

class FisheyeDewarpBottomBar : public MsWidget {
    Q_OBJECT

public:
    explicit FisheyeDewarpBottomBar(QWidget *parent = nullptr);
    ~FisheyeDewarpBottomBar();

    static FisheyeDewarpBottomBar *instance();

    void initializeData(int channel);

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_AUTO_TRACKING(MessageReceive *message);

signals:
    void fisheyePanelButtonClicked(int x, int yMargin);
    void closeFisheyePanel();
    void closeButtonClicked();

public slots:
    void onFisheyePanelButtonStateChanged(int state);

private slots:
    void onLanguageChanged();
    void onInitializeData();
    void on_toolButton_fish_panel_clicked();
    void on_toolButton_fish_track_clicked(bool checked);
    void on_toolButton_closeFisheye_clicked();

private:
    static FisheyeDewarpBottomBar *s_self;
    Ui::FisheyeDewarpBottomBar *ui;

    int m_channel = -1;

    QEventLoop m_eventLoop;

    bool m_fisheyeTrackButtonEnable = false;
    resp_fishmode_param m_fishmode_param;
    ms_auto_tracking m_auto_tracking;
};

#endif // FISHEYEDEWARPBOTTOMBAR_H
