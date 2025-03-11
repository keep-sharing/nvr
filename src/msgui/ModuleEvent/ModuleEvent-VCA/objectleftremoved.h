#ifndef OBJECTLEFTREMOVED_H
#define OBJECTLEFTREMOVED_H

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
class ms_smart_leftremove_info;

struct smart_event;

namespace Ui {
class ObjectLeftRemoved;
}

class ObjectLeftRemoved : public BaseSmartWidget {
    Q_OBJECT

public:
    explicit ObjectLeftRemoved(QWidget *parent = 0);
    ~ObjectLeftRemoved();

    void initializeData(int channel) override;
    void saveData() override;
    void copyData() override;
    void clearCopyInfo() override;

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_VCA_LEFTREMOVE(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_VCA_LEFTREMOVE(MessageReceive *message);
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
    void on_pushButton_setAll_clicked();
    void on_pushButton_clearAll_clicked();
    void on_pushButton_editTime_clicked();
    void on_pushButton_editAction_clicked();
    void onCheckBoxEnableClicked();
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

    void on_checkBox_obj_left_clicked(bool checked);
    void on_checkBox_obj_remove_clicked(bool checked);

private slots:
    void onLanguageChanged();
    void onNeedSensitivityChanged();

    void onDrawPolygonConflicted();

    void on_comboBoxEffectiveRegion_indexSet(int index);
    void on_comboBoxEffectiveWithPreset_activated(int index);
    void on_comboBoxRegionNo_indexSet(int index);
    void on_pushButtonEffectiveWithPreset_clicked();
    void on_lineEdit_mintime_editingFinished();

private:
    Ui::ObjectLeftRemoved *ui;
    DrawMotion *m_videoDraw = nullptr;
    ActionSmartEvent *m_action = nullptr;
    EffectiveTimeVCA *m_effectiveTime = nullptr;
    ms_smart_leftremove_info *m_object_info = nullptr;

    bool m_needSensitivity = true;
    DrawSceneObjectSize *m_drawScene = nullptr;
    ms_vca_settings_info2 *m_settings_info2 = nullptr;

    DrawMultiControlPolygon *m_drawMultiControl = nullptr;
    int m_currentRegion = 0;
    bool m_isFirstChoiceAdvanced = false;
    bool m_ptzNeedToCall = true;
    int m_objectSizeType;
};

#endif // OBJECTLEFTREMOVED_H
