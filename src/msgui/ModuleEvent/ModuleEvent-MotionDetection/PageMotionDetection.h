#ifndef PAGEMOTIONDETECTION_H
#define PAGEMOTIONDETECTION_H

#include "AbstractSettingPage.h"
#include "GraphicsDrawGrid.h"

extern "C" {
#include "msdb.h"
#include "msg.h"

}

class ActionMotion;
class EffectiveTimeMotionDetection;

namespace Ui {
class PageMotionDetection;
}

class PageMotionDetection : public AbstractSettingPage
{
    Q_OBJECT

public:
    explicit PageMotionDetection(QWidget *parent = 0);
    ~PageMotionDetection();

    void initializeData() override;

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_IPCMTMAP(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_IPCMTMAP(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message);

    void saveChannelSetting(int channel);

private slots:
    void onLanguageChanged() override;

    void setSettingEnable(bool enable);
    void clearSetting();

    void onButtonGroupClicked(int index);

    void on_checkBox_enable_clicked(bool checked);
    void on_pushButton_setAll_clicked();
    void on_pushButtonDeleteAll_clicked();
    void on_pushButton_effectiveTime_clicked();
    void on_pushButton_action_clicked();

    void on_pushButton_copy_clicked();
    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

private:
    Ui::PageMotionDetection *ui;

    GraphicsDrawGrid *m_drawGrid = nullptr;
    int m_currentChannel = 0;

    req_set_motionmap m_motionmap;

    //copy effective time
    smart_event_schedule *m_eventSchedule = nullptr;
    //copy action
    struct motion *m_motion = nullptr;
    struct motion_schedule *m_motionScheduleAudible = nullptr;
    struct motion_schedule *m_motionScheduleEmail = nullptr;
    struct motion_schedule *m_motionSchedulePopup = nullptr;
    struct motion_schedule *m_motionSchedulePTZ = nullptr;
    struct ptz_action_params *m_ptzActionParams = nullptr;

    ActionMotion *m_action = nullptr;
    EffectiveTimeMotionDetection *m_effective = nullptr;
};

#endif // PAGEMOTIONDETECTION_H
