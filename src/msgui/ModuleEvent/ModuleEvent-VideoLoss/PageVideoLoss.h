#ifndef PAGEVIDEOLOSS_H
#define PAGEVIDEOLOSS_H

#include "AbstractSettingPage.h"

class ActionVideoLoss;

namespace Ui {
class PageVideoLoss;
}

class PageVideoLoss : public AbstractSettingPage {
    Q_OBJECT

public:
    explicit PageVideoLoss(QWidget *parent = nullptr);
    ~PageVideoLoss();

    void initializeData() override;

    void processMessage(MessageReceive *message) override;

private:
    void saveChannelSetting(int channel);

private slots:
    void onLanguageChanged() override;

    void onButtonGroupClicked(int index);

    void on_pushButton_actionEdit_clicked();
    void on_pushButton_copy_clicked();
    void on_pushButton_back_clicked();

    void on_pushButtonApply_clicked();

private:
    Ui::PageVideoLoss *ui;

    int m_currentChannel = 0;

    //copy action
    struct video_loss *m_videoloss = nullptr;
    struct video_loss_schedule *m_videolossScheduleAudible = nullptr;
    struct video_loss_schedule *m_videolossScheduleEmail = nullptr;
    struct video_loss_schedule *m_videolossSchedulePopup = nullptr;
    struct video_loss_schedule *m_videoloaddSchedulePTZ = nullptr;
    struct ptz_action_params *m_ptzActionParams = nullptr;

    ActionVideoLoss *m_action = nullptr;
};

#endif // PAGEVIDEOLOSS_H
