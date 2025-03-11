#ifndef TAMPERDETECTION_H
#define TAMPERDETECTION_H

#include "BaseSmartWidget.h"
#include "pushbuttoneditstate.h"
#include <QWidget>

class ActionSmartEvent;
class EffectiveTimeVCA;
class DrawMotion;
class DrawSceneObjectSize;

class ms_smart_event_info;
class ms_vca_settings_info2;

struct smart_event;

namespace Ui {
class TamperDetection;
}

class TamperDetection : public BaseSmartWidget {
    Q_OBJECT

public:
    explicit TamperDetection(QWidget *parent = 0);
    ~TamperDetection();

    void initializeData(int channel) override;
    void saveData() override;
    void copyData() override;
    void clearCopyInfo() override;

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_VCA_TAMPER(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_VCA_TAMPER(MessageReceive *message);

    void clearSettings();
    void updateEnableState();

private slots:
    void onChannelButtonClicked(int index);
    void on_checkBoxEnable_clicked(bool checked);
    void on_pushButton_editTime_clicked();
    void on_pushButton_editAction_clicked();
    void onLanguageChanged();

private:
    Ui::TamperDetection *ui;
    DrawMotion *m_videoDraw = nullptr;
    ActionSmartEvent *m_action = nullptr;
    EffectiveTimeVCA *m_effectiveTime = nullptr;
    ms_smart_event_info *m_event_info = nullptr;
};

#endif // TAMPERDETECTION_H
