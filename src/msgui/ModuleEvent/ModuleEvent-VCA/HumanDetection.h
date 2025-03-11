#ifndef HUMANDETECTION_H
#define HUMANDETECTION_H

#include "BaseSmartWidget.h"
#include "msqtvca.h"
#include "pushbuttoneditstate.h"

class ActionSmartEvent;
class EffectiveTimeVCA;
class VideoTrack;
class DrawSceneObjectSize;

class ms_smart_event_info;
class ms_vca_settings_info2;
class ms_auto_tracking;

struct smart_event;

namespace Ui {
class HumanDetection;
}

class HumanDetection : public BaseSmartWidget {
    Q_OBJECT

public:
    explicit HumanDetection(QWidget *parent = 0);
    ~HumanDetection();

    void initializeData(int channel) override;
    void saveData() override;
    void copyData() override;
    void clearCopyInfo() override;

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONE_FLAG_GET_VCA_HUMANDETECTION(MessageReceive *message);
    void ON_RESPONE_FLAG_SET_VCA_HUMANDETECTION(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_AUTO_TRACKING(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_AUTO_TRACKING(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_VAC_SETTINGS2(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_VAC_SETTINGS2(MessageReceive *message);

    void clearSettings();
    void updateEnableState();

    bool checkObjectSize();

private slots:
    void onChannelButtonClicked(int index);
    void on_pushButton_editTime_clicked();
    void on_pushButton_editAction_clicked();
    void on_checkBoxEnable_clicked(bool checked);
    void on_checkBox_tracks_clicked(bool checked);
    void onSceneObjectSizeChanged(const QRect &rc, MsQtVca::ObjectSizeType sizeType);
    void on_radioButton_minLayout_clicked(bool checked);
    void on_radioButton_maxLayout_clicked(bool checked);

    void onLineEditMinWidthEditingFinished();
    void onLineEditMinHeightEditingFinished();
    void onLineEditMaxWidthEditingFinished();
    void onLineEditMaxHeightEditingFinished();

    void on_pushButtonObjectSizeEdit_clicked();
    void on_pushButtonObjectSizeEdit_stateSet(PushButtonEditState::State state);
    void on_pushButtonObjectSizeReset_clicked();
    void onLanguageChanged();

private:
    Ui::HumanDetection *ui;

    VideoTrack *m_videoDraw = nullptr;
    ActionSmartEvent *m_action = nullptr;
    EffectiveTimeVCA *m_effectiveTime = nullptr;
    QEventLoop m_eventLoop;

    ms_auto_tracking *m_auto_tracking = nullptr;
    ms_smart_event_info *m_event_info = nullptr;
    DrawSceneObjectSize *m_drawScene = nullptr;
    ms_vca_settings_info2 *m_settings_info2 = nullptr;
    int m_objectSizeType;
};

#endif // HUMANDETECTION_H
