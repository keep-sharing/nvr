#ifndef IMAGEPAGEENHANCEMENT_H
#define IMAGEPAGEENHANCEMENT_H

#include "MsCameraModel.h"
#include "abstractimagepage.h"
#include "bwhschedule.h"
#include "drawenhancement.h"
#include "exposureschedule.h"
#include "manualmodesettings.h"
#include "whitebalanceschedule.h"

namespace Ui {
class ImagePageEnhancement;
}

class ImagePageEnhancement : public AbstractImagePage {
    Q_OBJECT

    enum PowerLineFrequency {
        Frequency_60Hz = 0,
        Frequency_50Hz = 1,
        Frequency_Unknow
    };

public:
    explicit ImagePageEnhancement(QWidget *parent = nullptr);
    ~ImagePageEnhancement();

    void initializeData(int channel) override;
    void processMessage(MessageReceive *message) override;

private:
    void clearSettings();
    void setSettingsEnable(bool enable);

    void initEnhancement();
    void hideBWHAll();

    void ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_SINGLE_IPCTYPE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPCPARAM(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPCIMAGE_ENHANCEMENT(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_COMMON_PARAM(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_WHITE_BALANCE_SCHE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_EXPOSURE_SCHE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_BWH_SCHE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_LPR_NIGHT_MODE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(MessageReceive *message);

    void getCommonParam(const QString &param);
    void setCommonParam(int channel, const QString &param, int value);
    void drawEnhanceArea();
    void initWideDynamicRange(int index);
    void saveEnhancement(int channel, bool isNightMode);
    void copyEnhancement();
    void showBlcWdrHlcMode();
    void initExposureTime();
    void initHighLightCompensation(int type);
    void showLensType(bool enable);

private slots:
    void onLanguageChanged();

    void on_comboBox_WhiteBalance_activated(int index);
    void on_pushButton_WhiteBalanceSchedule_clicked();

    void on_comboBox_DigitalAntifogMode_activated(int index);
    void on_comboBox_DigitalImageStabilisation_activated(int index);

    void on_comboBox_ExposureMode_activated(int index);
    void on_pushButton_ManualModeSettings_clicked();
    void on_pushButton_ExposureSchedule_clicked();

    void on_pushButton_BlcWdrHlcSchedule_clicked();
    void on_comboBox_BlcWdrHlcMode_activated(int index);
    void on_comboBox_BlcRegion_activated(int index);
    void on_comboBox_BlcWdrHlc_activated(int index);
    void on_comboBox_WideDynamicRange_activated(int index);
    void on_comboBox_HighLightCompensation_activated();
    void on_comboBox_reduceBlur_activated();

    void on_pushButton_copy_clicked();
    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();
    void onChangeDefog();
    void changeRedGain();
    void changeBlueGain();
    void changeHlcLevel();

    void settingFinish();
    void scheduleAccept();

private:
    Ui::ImagePageEnhancement *ui;

    MsCameraModel m_model;
    int m_channel;
    struct image_enhancement m_info;
    QList<int> m_copyChannelList;
    DrawEnhancement *m_drawEnhance = nullptr;
    WhiteBalanceSchedule *m_whiteBalanceSche = nullptr;
    BWHSchedule *m_BlcWdrHlcSche = nullptr;
    ExposureSchedule *m_exposureSche = nullptr;
    ManualModeSettings *m_manualModeSetting = nullptr;
    PowerLineFrequency m_powerLineFrequency = Frequency_Unknow;
    int m_mainFrameRate = 0;
    bool wdrIsOn = false;
};

#endif // IMAGEPAGEENHANCEMENT_H
