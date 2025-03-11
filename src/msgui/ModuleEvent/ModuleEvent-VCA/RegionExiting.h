#ifndef REGIONEXITING_H
#define REGIONEXITING_H

#include "BaseSmartWidget.h"
#include "msqtvca.h"
#include "pushbuttoneditstate.h"

class ActionSmartEvent;
class EffectiveTimeVCA;
class DrawMotion;
class DrawSceneObjectSize;
class DrawMultiControlPolygon;

class ms_smart_event_info;
class ms_vca_settings_info2;

struct smart_event;
namespace Ui {
class RegionExiting;
}

class RegionExiting : public BaseSmartWidget {
    Q_OBJECT

public:
    explicit RegionExiting(QWidget *parent = 0);
    ~RegionExiting();

    void initializeData(int channel) override;
    void saveData() override;
    void copyData() override;
    void clearCopyInfo() override;

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_VCA_REGIONEXIT(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_VCA_REGIONEXIT(MessageReceive *message);

    void ON_RESPONSE_FLAG_GET_VAC_SETTINGS2(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_VAC_SETTINGS2(MessageReceive *message);

    void clearSettings();
    void updateEnableState();

    bool checkObjectSize();

    bool isSupportPolygon();
    void setEffectiveRegionVisible(bool visible);
    void showRegionEdit();
    //index 0 normal 1-4 preset
    void readRegion(int index);

signals:
    void needSensitivityChanged();

protected:
    bool needSensitivity();
    void setNeedSensitivity(bool);
    bool isSupportSensitivity(int channel);

protected slots:
    void onChannelButtonClicked(int index);

    void on_checkBoxEnable_clicked(bool checked);

    void on_pushButton_setAll_clicked();
    void on_pushButton_clearAll_clicked();

    void onSceneObjectSizeChanged(const QRect &rc, MsQtVca::ObjectSizeType sizeType);

    void on_radioButton_minLayout_clicked(bool checked);
    void on_radioButton_maxLayout_clicked(bool checked);

    void onLineEditMinWidthEditingFinished();
    void onLineEditMinHeightEditingFinished();
    void onLineEditMaxWidthEditingFinished();
    void onLineEditMaxHeightEditingFinished();

    void on_pushButtonRegionEdit_clicked();
    void on_pushButtonRegionEdit_stateSet(PushButtonEditState::State state);

    void on_pushButtonObjectSizeEdit_clicked();
    void on_pushButtonObjectSizeEdit_stateSet(PushButtonEditState::State state);
    void on_pushButtonObjectSizeReset_clicked();

    void on_pushButton_editTime_clicked();
    void on_pushButton_editAction_clicked();

    void onDrawPolygonConflicted();

private slots:
    void onLanguageChanged();
    void onNeedSensitivityChanged();

    void on_checkBoxHuman_clicked();
    void on_checkBoxVehicle_clicked();

    void on_comboBoxEffectiveRegion_indexSet(int index);
    void on_comboBoxEffectiveWithPreset_activated(int index);
    void on_comboBoxRegionNo_indexSet(int index);
    void on_pushButtonEffectiveWithPreset_clicked();

private:
    Ui::RegionExiting *ui;

    DrawMotion *m_videoDraw = nullptr;
    ActionSmartEvent *m_action = nullptr;
    EffectiveTimeVCA *m_effectiveTime = nullptr;
    bool m_needSensitivity = true;

    ms_smart_event_info *m_event_info = nullptr;

    DrawSceneObjectSize *m_drawScene = nullptr;
    ms_vca_settings_info2 *m_settings_info2 = nullptr;

    DrawMultiControlPolygon *m_drawMultiControl = nullptr;
    int m_currentRegion = 0;
    bool m_isFirstChoiceAdvanced = false;
    bool m_ptzNeedToCall = true;
    int m_objectSizeType;
};

#endif // REGIONEXITING_H
