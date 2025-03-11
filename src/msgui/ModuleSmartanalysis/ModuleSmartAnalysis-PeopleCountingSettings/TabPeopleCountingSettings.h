#ifndef TABPEOPLECOUNTINGSETTINGS_H
#define TABPEOPLECOUNTINGSETTINGS_H

#include "AbstractSettingTab.h"
#include "msqtvca.h"
#include "pushbuttoneditstate.h"
#include <QEventLoop>
#include "PeopleCountingInformationEdit.h"
#include "GraphicsDrawLineCrossing.h"

class ms_smart_event_people_cnt;
class DrawSceneObjectSize;
class ms_vca_settings_info2;
class EffectiveTimePeopleCounting;
class ThresholdsEdit;
class ActionSmartEvent;

namespace Ui {
class TabPeopleCountingSettings;
}

class TabPeopleCountingSettings : public AbstractSettingTab {
    Q_OBJECT

public:
    explicit TabPeopleCountingSettings(QWidget *parent = 0);
    ~TabPeopleCountingSettings();

    void initializeData() override;
    void saveData();

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_VAC_SUPPORT(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_VCA_LICENSE(MessageReceive *message);

    void ON_RESPONSE_FLAG_GET_VCA_PEOPLE_COUNT(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_VCA_PEOPLE_COUNT(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_VAC_SETTINGS2(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_VAC_SETTINGS2(MessageReceive *message);

    void clearSettings();
    void updateEnableState();

private:
    void hideMessage();
    void showNotConnectedMessage();
    void showNotSupportMessage();
    void showDataError();

    int waitForCheckVcaSupport();
    int waitForCheckFisheeyeSupport();

    bool checkObjectSize();

    void clearCopyInfo();
    void setMultiLineVisisble(bool visible);

private slots:
    void onLanguageChanged() override;
    void onChannelButtonClicked(int index);
    void on_pushButton_clear_clicked();
    void on_pushButton_clearCount_clicked();
    void on_checkBoxEnable_clicked(bool checked);
    void onSceneObjectSizeChanged(const QRect &rc, MsQtVca::ObjectSizeType sizeType);
    void on_radioButton_minLayout_clicked(bool checked);
    void on_radioButton_maxLayout_clicked(bool checked);

    void onLineEditMinWidthEditingFinished();
    void onLineEditMinHeightEditingFinished();
    void onLineEditMaxWidthEditingFinished();
    void onLineEditMaxHeightEditingFinished();

    void on_pushButtonLineEdit_clicked();
    void on_pushButtonLineEdit_stateSet(PushButtonEditState::State state);

    void on_pushButtonObjectSizeEdit_clicked();
    void on_pushButtonObjectSizeEdit_stateSet(PushButtonEditState::State state);
    void on_pushButtonObjectSizeReset_clicked();

    void on_pushButtonApply_clicked();
    void on_pushButtonBack_clicked();

    void on_pushButtonCountingInfoEdit_clicked();
    void on_comboBoxLine_indexSet(int index);
    void on_comboBoxDirection_indexSet(int index);
    void updateDynamicDisplay(int type, int channel);

    void on_pushButtonCopy_clicked();

    void on_comboBoxAlarmTrigger_activated(int index);
    void on_pushButtonThresholds_clicked();
    void on_pushButtonPeopleCountingSchedule_clicked();
    void on_pushButtonAction_clicked();

    void on_checkBoxAlarmTrigger_clicked();

private:
    Ui::TabPeopleCountingSettings *ui;

    QEventLoop m_eventLoop;

    int m_currentChannel = 0;
    bool m_isConnected = false;
    bool m_isSupported = -1;

    bool m_isVcaSupport = false;
    bool m_isLicenseVaild = false;
    int m_vcaType = 0;
    quint64 m_copyFlag = 0;

    GraphicsDrawLineCrossing *m_drawLine = nullptr;
    ms_smart_event_people_cnt *m_peopleCountInfo = nullptr;

    DrawSceneObjectSize *m_drawScene = nullptr;
    ms_vca_settings_info2 *m_settings_info2 = nullptr;

    PeopleCountingInformationEdit *m_countingInfo = nullptr;
    int m_currentLine = 0;

    ActionSmartEvent *m_action = nullptr;
    EffectiveTimePeopleCounting *m_effectiveTime = nullptr;
    ThresholdsEdit *m_thresholds = nullptr;
    int m_objectSizeType;
};

#endif // TABPEOPLECOUNTINGSETTINGS_H
