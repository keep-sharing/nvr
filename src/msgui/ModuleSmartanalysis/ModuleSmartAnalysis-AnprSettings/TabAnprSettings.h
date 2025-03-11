#ifndef TABANPRSETTINGS_H
#define TABANPRSETTINGS_H

#include "anprbasepage.h"
#include <QEventLoop>

class DrawView;
class DrawSceneAnpr;
class EffectiveTimeAnpr;
class DrawItemAnprControl;

namespace Ui {
class AnprSettingPage;
}

class TabAnprSettings : public AnprBasePage {
    Q_OBJECT

    enum RegionColumn {
        ColumnCheck,
        ColumnId,
        ColumnName,
        ColumnEdit,
        ColumnDelete
    };
    enum LPRType {
        None,
        TPOS,
        Transportation,
    };

public:
    explicit TabAnprSettings(QWidget *parent = nullptr);
    ~TabAnprSettings();

    virtual void initializeData(int channel) override;
    virtual void processMessage(MessageReceive *message) override;

signals:
    void needLPRNightModeChanged();

protected:
    void showEvent(QShowEvent *) override;

private:
    void updateTable();
    void ON_RESPONSE_FLAG_GET_LPR_SUPPORT(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_LPR_SETTINGS(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_LPR_SETTINGS(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_LPR_WILDCARDS(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_IPC_LPR_WILDCARDS(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPCIMAGE_ENHANCEMENT(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_IPCIMAGE_DISPLAY(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_COMMON_PARAM(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_MODEL_TYPE(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_ANPR_EVENT(MessageReceive *message);

    void showEnable(bool enable);
    bool needLPRNightMode();
    void setNeedLPRNightMode(bool);
    void setLPRNightModePanelEnabled(bool);
    void setAutoModeVisible(bool vsb);
    void setCustomModeVisible(bool vsb);
    void getCommonParam(const QString &param);
    bool isProDome(char *productmodel);
    bool isSmokedDomeCoverSensor(char *sensortype);
    bool isMiniPtzDome(char *productmodel);
    void clearSetting();
    bool isMsLpr(IPC_AITYPE_E aitype);

private slots:
    void onLanguageChanged();
    void onTableItemClicked(int row, int column);
    void on_pushButton_add_clicked();
    void on_pushButton_clear_clicked();
    void on_pushButton_clearAll_clicked();

    void on_comboBox_processingResolution_currentIndexChanged(int index);
    void on_comboBox_lprNightMode_currentIndexChanged(int index);
    void on_comboBox_LPRNightModeEffectiveTime_activated(int index);
    void on_pushButton_DayToNightValue_clicked();
    void on_pushButton_NightToDayValue_clicked();
    void onSliderDayToNightValueChange();
    void onSliderNightToDayValueChange();

    void on_pushButton_effectiveTime_clicked();
    void on_pushButton_detectionSettings_clicked();

    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

    void on_checkBox_enable_stateChanged(int arg1);
    void on_comboBox_countryRegion_activated(const QString &arg1);
    void on_checkBoxLPRImageMode_stateChanged(int arg1);
    void onDrawPolygonConflicted();

private:
    Ui::AnprSettingPage *ui;
    DrawView *m_drawView = nullptr;
    DrawSceneAnpr *m_anprDraw = nullptr;
    DrawItemAnprControl *m_anprTrapeziform = nullptr;
    int m_currentChannel = -1;
    ms_lpr_settings m_lpr_settings;
    ms_lpr_wildcards m_lpr_wildcards;
    EffectiveTimeAnpr *m_effectiveTime = nullptr;
    struct {
        image_enhancement data;
        image_enhancement *p = nullptr;
    } imageEnhancement;
    bool m_needLPRNightMode = true;
    int m_currentChnType = -1;
    image_display m_info;
    char version[64];

    bool m_hasEnable = false;
    QEventLoop m_eventLoop;

    bool m_isTrapeziform = true;
};

#endif // TABANPRSETTINGS_H
