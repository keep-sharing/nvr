#ifndef IMAGEPAGEDISPLAY_H
#define IMAGEPAGEDISPLAY_H

#include "MsCameraModel.h"
#include "abstractimagepage.h"
#include <QEventLoop>
#include <QMap>

extern "C" {
#include "msg.h"
}

namespace Ui {
class ImagePageDisplay;
}

class ImagePageDisplay : public AbstractImagePage {
    Q_OBJECT

public:
    enum lensType {
        PWM_TYPE_NONE,
        PWM_TYPE_1IR_1W,
        PWM_TYPE_2IR,
        PWM_TYPE_2IR_1W,
        PWM_TYPE_2IR_1NW,
        PWM_TYPE_1IR,
        PWM_TYPE_3IR,
        PWM_TYPE_1IR_2W
    };
    enum smartIrType {
        m_bnc_len = 1 << 3,
        m_pwm = 1 << 6,
        m_pwm_2 = 1 << 8,
        m_PWM_1IR_1W = 1 << 9,
        m_PWM_2IR = 1 << 10,
        m_PWM_2IR_1W = 1 << 11,
        m_PWM_1IR_2W = 1 << 23,
        m_PWM_2IR_1NW = 1 << 12,
        m_PWM_1IR = 1 << 14,
        m_PWM_3IR = 1 << 15,
        m_distance_measure = 1 << 20,
        m_fix_len = 1 << 21
    };
    enum WhiteLEDControlMode {
        WhiteLEDAutoMode = 0,
        WhiteLEDAlwaysOn,
        WhiteLEDOFF,
        WhiteLEDCustomize
    };

    explicit ImagePageDisplay(QWidget *parent = nullptr);
    ~ImagePageDisplay();

    virtual void initializeData(int channel) override;
    virtual void processMessage(MessageReceive *message) override;

    static int defaultNearViewIRValue(const MsCameraModel &model);
    static int defaultMiddleViewIRValue(const MsCameraModel &model);
    static int defaultFarViewIRValue(const MsCameraModel &model);
    static int defaultIRLEDLevel(const MsCameraModel &model);

signals:
    void needDNSensitivityChanged();
    void needZoomLimitChanged();

private:
    void clearSettings();
    void setSettingsEnable(bool enable);

    void initDisplay();
    void getCommonParam(const QString &param);

    void ON_RESPONSE_FLAG_GET_SINGLE_IPCTYPE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_MODEL_TYPE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_COMMON_PARAM(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_IPCIMAGE_DISPLAY(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_LPR_NIGHT_MODE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_LPR_SETTINGS(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_CAP_IMAGE(MessageReceive *message);

    int whiteandIRPWMInterface(int m_chipninterface);
    bool isLessOldPWMInterface(int m_chipninterface);
    bool isSupDistanceMeasure(int m_chipninterface);
    bool isProDome(char *productmodel);
    bool isSmokedDomeCoverSensor(char *sensortype);
    bool isMiniPtzDome(char *productmodel);
    bool isFixFocus(int m_chipninterface);
    bool isLessBncInterface(int m_chipninterface);
    int saveDisplayInfo();
    void updateCorridor(int type);
    bool checkCorridor(int type);
    void onShowWhite();
    bool isSpeed(char *productmodel);
    bool isMiniZoomDome(char *productmodel);
    bool isMiniWithoutIR(char *productmodel);
    bool isBox(char *productmodel);
    bool isMiniProBulletFull(char *productmodel);
    void onCompatiblePhotosensitive();
    void isOneIRModel();
    void mayDisplayDNSensitivityPanel(bool visible);
    void mayDisplayZoomLimitPanel(bool visible, int zoomLimit = -1);
    bool needDNSensitivity();
    void setNeedDNSensitivity(bool);
    void sensitivitySanityCheck(bool DPreferred);
    bool needZoomLimit();
    void setNeedZoomLimit(bool);
    bool isNewFixProBullet();
    bool isAFLens3X();
    bool isSpeedLens36Or42();
    bool isHisiIPC();
    bool isPoe(char *productmodel);
    bool isSupportDayNightSwitchRefocus();
    void fullColorWhiteLEDLightInit();

private slots:
    void onLanguageChanged();

    void onModeIRChange(int index);
    void onDayNightModeChange(int index);
    void onSliderDayToNightChanged(int value);
    void onSliderNightToDayChanged(int value);
    void on_pushButton_copy_clicked();
    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();
    void on_pushButton_NearViewIrValue_clicked();
    void on_pushButton_MiddleViewIrValue_clicked();
    void on_pushButton_FarViewIrValue_clicked();
    void on_pushButton_IRLedValue_clicked();
    void on_pushButton_whiteLedLevel_clicked();
    void on_pushButton_fovAdjust_clicked();
    void on_pushButton_DayToNightValue_clicked();
    void on_pushButton_NightToDayValue_clicked();
    void on_pushButton_DayToNightSensitivity_clicked();
    void on_pushButton_NightToDaySensitivity_clicked();
    void onDSensitivityChanged(int);
    void onNSensitivityChanged(int);
    void on_comboBox_LensDistortCorrect_activated();
    void onSliderDayToNightValueChange();
    void onSliderNightToDayValueChange();
    void onChangeNearValue();
    void onChangeFarValue();
    void onNeedDNSensitivityChanged();
    void adjustSensitivity();
    void onNeedZoomLimitChanged();
    void on_pushButton_test_clicked();
    void onTimerTest();

    void on_comboBox_ZoomLimit_activated(int index);
    void on_comboBox_keepCorrectAspectRatio_activated(int index);

    void on_toolButtonIrStrengthValue_clicked();

    void on_comboBoxWhiteLEDLightControl_indexSet(int index);
    void on_comboBoxBrightnessControl_indexSet(int index);
    void onWhiteLEDLightTimeChange(const QTime &time);
    void onDayNightTimeChange(const QTime &time);

    void onUpdatePlayVideo();

  private:
    Ui::ImagePageDisplay *ui;

    MsCameraModel m_model;
    int m_channel = -1;
    int m_smartIrMode = -1;
    int m_oneIRModel = -1;
    int m_currentChnType = -1;
    image_display m_info;
    bool m_isCopy = false;
    QList<int> m_copyChannelList;
    QMap<int, CAM_MODEL_INFO> m_modelMap;
    bool m_supportCorridor;
    bool m_needDNSensitivity = false;
    bool m_daySensitivityPreferred = false;
    bool m_needZoomLimit = false;
    int isNightMode = -1;
    int m_nightModeEnable = 0;
    int m_nightModeEffectiveMode = -1;
    bool inAdjustSensitivity = false;
    static const int dnSensitivityLimit = 15;
    bool m_isInitDisplay = false;

    QTimer *m_timerTest = nullptr;
};

#endif // IMAGEPAGEDISPLAY_H
