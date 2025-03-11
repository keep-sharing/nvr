#ifndef TABPTZCONFIGURATION_H
#define TABPTZCONFIGURATION_H

#include <QWidget>
#include <QMap>
#include "ptzbasepage.h"

extern "C"
{
#include "msdb.h"
#include "msg.h"
}

class PresetTableItemDelegate;

namespace Ui {
class PtzConfigurationPage;
}

class TabPtzConfiguration : public PtzBasePage
{
    Q_OBJECT

    enum PresetColumn
    {
        PresetColumnCheck,
        PresetColumnNumber,
        PresetColumnPlay,
        PresetColumnSave,
        PresetColumnDelete
    };
    enum PatrolColumn
    {
        PatrolColumnCheck,
        PatrolColumnPoint,
        PatrolColumnPreset,
        PatrolColumnSpeed,
        PatrolColumnTime,
        PatrolColumnEdit,
        PatrolColumnDelete
    };

public:
    explicit TabPtzConfiguration(QWidget *parent = nullptr);
    ~TabPtzConfiguration();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private:
    //
    void adjustTableItemWidth();
    //
    void setPtzEnable(bool enable);
    //
    void sendPtzControl(int action);
    //preset
    void initializePreset();
    void showPresetData(const resp_ptz_preset *preset_array, int count);
    //patrol
    void initializePatrol();
    void showPatrolData(const resp_ptz_tour *tour_array, int count);
    void setPatrolData(int tourid);
    void updatePatrolDataFromTable();
    //pattern
    void initializePattern();
    void showPatternData(const int *pattern_array, int count);

private slots:
    void onLanguageChanged();

    void on_comboBox_channel_activated(int index);

    //preset
    void onPresetItemClicked(int row, int column);
    //patrol
    void on_comboBox_patrol_activated(int index);
    void onPatrolItemClicked(int row, int column);
    void on_toolButton_patrol_add_clicked();
    void on_toolButton_patrol_up_clicked();
    void on_toolButton_patrol_down_clicked();
    void on_toolButton_patrol_play_clicked();
    void on_toolButton_patrol_stop_clicked();
    void on_toolButton_patrol_delete_clicked();
    //pattern
    void on_toolButton_pattern_record_clicked(bool checked);
    void on_toolButton_pattern_play_clicked();
    void on_toolButton_pattern_stop_clicked();
    void on_toolButton_pattern_delete_clicked();

    void on_pushButton_back_clicked();

private:
    Ui::PtzConfigurationPage *ui;

    int m_channel;
    ptz_speed m_ptzSpeed;

    PresetTableItemDelegate *m_itemDelegate;

    QMap<int, resp_ptz_tour> m_patrolMap;

    CAM_MODEL_INFO m_model_info;
};

#endif // TABPTZCONFIGURATION_H
