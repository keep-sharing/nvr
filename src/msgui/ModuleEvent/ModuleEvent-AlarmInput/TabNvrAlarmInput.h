#ifndef TABNVRALARMINPUT_H
#define TABNVRALARMINPUT_H

#include "AbstractSettingTab.h"

class ActionNvrAlarmInput;
class EffectiveTimeNvrAlarmInput;

struct smart_event;
namespace Ui {
class TabNvrAlarmInput;
}

class TabNvrAlarmInput : public AbstractSettingTab {
    Q_OBJECT

public:
    explicit TabNvrAlarmInput(QWidget *parent = nullptr);
    ~TabNvrAlarmInput();

    void initializeData();
    void processMessage(MessageReceive *message);

private:
    void ON_RESPONSE_FLAG_SET_ALARMIN(MessageReceive *message);

    void saveCurrentAlarmin();

signals:
    void back();

private slots:
    void onLanguageChanged();

    void onButtonGroupClicked(int index);

    void on_pushButton_actionEdit_clicked();
    void on_pushButton_effectiveEdit_clicked();

    void on_pushButton_copy_clicked();
    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

    void on_comboBoxLinkAction_indexSet(int index);
    void onActionButtonGroupClicked();

  private:
    Ui::TabNvrAlarmInput *ui;

    int m_currentIndex = 0;
    alarm_in m_alarmin;

    ActionNvrAlarmInput *m_action = nullptr;
    EffectiveTimeNvrAlarmInput *m_effective = nullptr;
    DISARMING_S m_disarm;
};

#endif // TABNVRALARMINPUT_H
