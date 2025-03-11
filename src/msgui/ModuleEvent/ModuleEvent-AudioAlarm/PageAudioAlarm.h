#ifndef PAGEAUDIOALARM_H
#define PAGEAUDIOALARM_H

#include "AbstractSettingPage.h"
#include "QEventLoop"
extern "C" {
#include "msdb.h"
#include "msg.h"

}

class ActionAudioAlarm;
class EffectiveTimeAudioAlarm;

namespace Ui {
class PageAudioAlarm;
}

class PageAudioAlarm : public AbstractSettingPage {
    Q_OBJECT

public:
    explicit PageAudioAlarm(QWidget *parent = nullptr);
    ~PageAudioAlarm();

    void initializeData() override;

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_IPC_AUDIO_ALARM(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_IPC_AUDIO_ALARM(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_AUDIO_ALARM_SAMPLE(MessageReceive *message);
    void saveSetting(quint64 copyFlags);

private slots:
    void onLanguageChanged() override;

    void setSettingEnable(bool enable);
    void clearSetting();

    void onButtonGroupClicked(int index);

    void on_checkBoxEnable_clicked(bool checked);
    void on_pushButtonEffectiveTime_clicked();
    void on_pushButtonAction_clicked();

    void on_pushButtonCopy_clicked();
    void on_pushButtonApply_clicked();
    void on_pushButtonBack_clicked();

    void on_toolButtonReset_clicked();

    void on_toolButtonReset_pressed();

    void on_toolButtonReset_released();

private:
    Ui::PageAudioAlarm *ui;

    int m_currentChannel = 0;

    QEventLoop m_eventLoop;

    ActionAudioAlarm *m_action = nullptr;
    EffectiveTimeAudioAlarm *m_effectiveTime = nullptr;
    bool m_haveJsonData = false;
};

#endif // PAGEAUDIOALARM_H
