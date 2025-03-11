#ifndef TabHeatMapSettings_H
#define TabHeatMapSettings_H

#include "AbstractSettingTab.h"

#include <QEventLoop>

extern "C" {
#include "msg.h"
}

class DrawView;
class DrawSceneMotion;
class EffectiveTimePeopleCounting;

namespace Ui {
class HeatMapSettings;
}

class TabHeatMapSettings : public AbstractSettingTab {
    Q_OBJECT

public:
    explicit TabHeatMapSettings(QWidget *parent = nullptr);
    ~TabHeatMapSettings();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_IPC_HEATMAP_SUPPORT(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_SINGLE_IPCTYPE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_MODEL_TYPE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_HEAT_MAP_SETTING(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_HEAT_MAP_SETTING(MessageReceive *message);

    void setSettingEnable();
    void saveSetting();

private slots:
    void onLanguageChanged() override;

    void onChannelGroupClicked(int channel);

    void onSliderObjectSizeChanged(int value);

    void on_pushButton_setAll_clicked();
    void on_pushButtonDeleteAll_clicked();

    void on_pushButton_copy_clicked();
    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

    void on_checkBox_enable_clicked(bool checked);

    void on_pushButtonHeatmapSchedule_clicked();

private:
    Ui::HeatMapSettings *ui;

    int m_currentChannel = 0;

    DrawView *m_drawView = nullptr;
    DrawSceneMotion *m_drawScene = nullptr;

    QEventLoop m_eventLoop;

    CAM_MODEL_INFO m_model;
    resp_image_display m_image_display;
    int m_modelType = -1;
    resp_fishmode_param m_fishmode;

    ms_heat_map_setting m_heat_map_setting;
    EffectiveTimePeopleCounting *m_effectiveTime = nullptr;

    bool m_isConnected = false;
    bool m_isSupported = false;
};

#endif // TabHeatMapSettings_H
