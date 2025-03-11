#ifndef LINECROSSING_H
#define LINECROSSING_H

#include "BaseSmartWidget.h"
#include "MsCameraVersion.h"
#include "msqtvca.h"
#include "pushbuttoneditstate.h"
#include "GraphicsDrawLineCrossing.h"

class ActionLineCrossing;
class EffectiveTimeLineCrossing;
class DrawMotion;
class DrawSceneObjectSize;
class ms_smart_event_info;
class ms_vca_settings_info2;

struct smart_event;

namespace Ui {
class LineCrossing;
}

class LineCrossing : public BaseSmartWidget {
    Q_OBJECT

public:
    explicit LineCrossing(QWidget *parent = 0);
    ~LineCrossing();

    void initializeData(int channel) override;
    void saveData() override;
    void copyData() override;
    void clearCopyInfo() override;

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_VCA_LINECROSSING(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_VCA_LINECROSSING(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_VCA_LINECROSSING2(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_IPC_VCA_LINECROSSING2(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_VAC_SETTINGS2(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_VAC_SETTINGS2(MessageReceive *message);
    void clearSettings();
    void updateEnableState();
    void setEffectiveRegionVisible(bool visible);

protected:
    void hideEvent(QHideEvent *) override;

private:
    void saveLineCross(int index);

    bool checkObjectSize();

protected slots:
    void onChannelButtonClicked(int index);
    void on_pushButton_clear_clicked();
    void on_comboBox_direction_currentIndexChanged(int index);
    void on_pushButton_editTime_clicked();
    void on_pushButton_editAction_clicked();
    void on_comboBoxLine_indexSet(int index);
    void on_checkBoxEnable_clicked(bool checked);

    void onLineEditMinWidthEditingFinished();
    void onLineEditMinHeightEditingFinished();
    void onLineEditMaxWidthEditingFinished();
    void onLineEditMaxHeightEditingFinished();

    void onSceneObjectSizeChanged(const QRect &rc, MsQtVca::ObjectSizeType sizeType);
    void on_radioButton_minLayout_clicked(bool checked);
    void on_radioButton_maxLayout_clicked(bool checked);

    void on_pushButtonLineEdit_clicked();
    void on_pushButtonLineEdit_stateSet(PushButtonEditState::State state);

    void on_pushButtonObjectSizeEdit_clicked();
    void on_pushButtonObjectSizeEdit_stateSet(PushButtonEditState::State state);
    void on_pushButtonObjectSizeReset_clicked();

private slots:
    void onLanguageChanged();

    void on_comboBoxEffectiveRegion_indexSet(int index);
    void on_comboBoxEffectiveWithPreset_activated(int index);
    void on_pushButtonEffectiveWithPreset_clicked();

private:
    Ui::LineCrossing *ui;

    GraphicsDrawLineCrossing *m_drawLine = nullptr;

    ActionLineCrossing *m_action = nullptr;
    EffectiveTimeLineCrossing *m_effectiveTime = nullptr;
    ms_linecrossing_info m_linecrossing_info;
    ms_linecrossing_info2 m_linecrossing_info2;

    int m_currentLineIndex = -1;
    MsCameraVersion m_cameraVersion;
    DrawSceneObjectSize *m_drawScene = nullptr;
    ms_vca_settings_info2 *m_settings_info2 = nullptr;
    bool m_isFirstChoiceAdvanced = false;
    bool m_ptzNeedToCall = true;
    int m_objectSizeType;
};

#endif // LINECROSSING_H
