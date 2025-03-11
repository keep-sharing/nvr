#ifndef LIVEVIEWBOTTOMBAR_H
#define LIVEVIEWBOTTOMBAR_H

#include "DisplaySetting.h"
#include "Disturbing.h"
#include "EventNotification.h"
#include "IrregularLayout.h"
#include "MsWidget.h"
#include "Sequence.h"
#include <QButtonGroup>
#include <QDateTime>
#include <QTimer>

extern "C" {
#include "msg.h"
}

class CustomLayoutKey;
class MyToolButton;
class UpgradeThread;

namespace Ui {
class LiveviewBottomBar;
}

class LiveviewBottomBar : public MsWidget {
    Q_OBJECT

public:
    explicit LiveviewBottomBar(QWidget *parent = nullptr);
    ~LiveviewBottomBar();

    static LiveviewBottomBar *instance();

    void updateTargetState();
    void updateDisturbState(bool checked);
    void updateOccupancyState();
    void updateUpgradedState();

    QWidget *test_randomLayoutButton();

    void getExceptionStatus();
    void dealMessage(MessageReceive *message);
    void processMessage(MessageReceive *message) override;
    void filterMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_INFOMATION_SHOW(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_ALL_RECORD(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_ONLINE_UPGRADE_INFO(MessageReceive *message);

    void setCurrentLayoutButton(const CustomLayoutKey &key);
    int screenRight();


  protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

signals:
    void setVideoRatio(int mode);
    void setLiveviewLayoutMode(int mode);
    void setAnprMode(bool enable);

private slots:
    void onLanguageChanged();
    void onToolButtonMouseEnter();
    void onLayoutModeChanged(const CustomLayoutKey &key);
    void onLayoutButtonGroupClicked(int id);
    void onScreenSwitched();
    void onDisplayStateChanged(bool open);
    void onSequenceStateChanged(int screen, bool open);

    //
    void on_toolButton_irregularLayout_clicked();
    //
    void on_toolButton_target_clicked();
    void on_toolButtonOccupancy_clicked();
    //
    void on_toolButton_event_clicked();
    void on_toolButton_startRecord_clicked();
    void on_toolButton_stopRecord_clicked();
    void on_toolButton_scaleMode_clicked();
    void on_toolButton_display_clicked();
    void on_toolButton_disturb_clicked();
    void on_toolButton_sequence_clicked();
    void on_toolButtonOnlineUpgrade_clicked();

    void onDownloadFinished(int result);
    void onUpgradeFinished(int result);
    void onTimerUpdate();

  private:
    static LiveviewBottomBar *s_self;
    Ui::LiveviewBottomBar *ui;

    QList<MyToolButton *> m_buttons;
    QButtonGroup *m_layoutButtonGroup = nullptr;
    QMap<QString, int> m_defaultNormalLayoutNames;

    EventNotification *m_eventNotification = nullptr;
    Disturbing *m_disturbing = nullptr;
    DisplaySetting *m_displaySetting = nullptr;
    Sequence *m_sequence = nullptr;
    IrregularLayout *m_irregularLayout = nullptr;

    RATIO_E m_ratio = RATIO_VO_FULL;

    resp_check_online_upgrade m_online_upgrade;
    UpgradeThread *m_upgradeThread = nullptr;
    bool canBeUpgraded = false;
    QTimer *m_timer = nullptr;
};

#endif // LIVEVIEWBOTTOMBAR_H
