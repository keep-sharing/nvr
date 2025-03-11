#include "imagepageenhancement.h"
#include "ui_imagepageenhancement.h"
#include "EventLoop.h"
#include "MessageBox.h"
#include "MsCameraVersion.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsSdkVersion.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "channelcopydialog.h"

extern "C" {
#include "msg.h"

}

ImagePageEnhancement::ImagePageEnhancement(QWidget *parent)
    : AbstractImagePage(parent)
    , ui(new Ui::ImagePageEnhancement)
{
    ui->setupUi(this);

    m_drawEnhance = new DrawEnhancement(this);
    m_drawEnhance->setColor(Qt::transparent);
    m_drawEnhance->setMaxMaskCount(1);

    //IR Balance Mode
    ui->comboBox_IrBalanceMode->clear();
    ui->comboBox_IrBalanceMode->addItem(GET_TEXT("COMMON/1013", "Off"), 0);
    ui->comboBox_IrBalanceMode->addItem(GET_TEXT("COMMON/1012", "On"), 1);

    //Reduce Video Stuttering
    ui->comboBox_reduceVideoStuttering->clear();
    ui->comboBox_reduceVideoStuttering->addItem(GET_TEXT("COMMON/1013", "Off"), 0);
    ui->comboBox_reduceVideoStuttering->addItem(GET_TEXT("COMMON/1012", "On"), 1);

    //White Balance
    ui->comboBox_WhiteBalance->clear();
    ui->comboBox_WhiteBalance->addItem(GET_TEXT("IMAGE/37344", "Auto White Balance"), 0);
    ui->comboBox_WhiteBalance->addItem(GET_TEXT("IMAGE/37345", "Manual White Balance"), 1);
    ui->comboBox_WhiteBalance->addItem(GET_TEXT("IMAGE/37346", "Incandescent Lamp"), 2);
    ui->comboBox_WhiteBalance->addItem(GET_TEXT("IMAGE/37347", "Warm Light Lamp"), 3);
    ui->comboBox_WhiteBalance->addItem(GET_TEXT("IMAGE/37348", "Natural Light"), 4);
    ui->comboBox_WhiteBalance->addItem(GET_TEXT("IMAGE/37349", "Fluorescent Lamp"), 5);
    ui->comboBox_WhiteBalance->addItem(GET_TEXT("IMAGE/37202", "Schedule Mode"), 6);
    ui->label_RedGainLevel->hide();
    ui->slider_RedGainLevel->hide();
    ui->label_BlueGainLevel->hide();
    ui->slider_BlueGainLevel->hide();
    ui->label_WhiteBalanceSchedule->hide();
    ui->widget_WhiteBalanceSchedule->hide();

    //Digital Anti-fog Mode
    ui->comboBox_DigitalAntifogMode->clear();
    ui->comboBox_DigitalAntifogMode->addItem(GET_TEXT("COMMON/1013", "Off"), 0);
    ui->comboBox_DigitalAntifogMode->addItem(GET_TEXT("COMMON/1012", "On"), 1);
    ui->label_AntifogIntensity->hide();
    ui->slider_AntifogIntensity->hide();

    //Digital Image Stabilisation
    ui->comboBox_DigitalImageStabilisation->clear();
    ui->comboBox_DigitalImageStabilisation->addItem(GET_TEXT("COMMON/1013", "Off"), 0);
    ui->comboBox_DigitalImageStabilisation->addItem(GET_TEXT("COMMON/1012", "On"), 1);

    //Exposure Mode
    ui->comboBox_ExposureMode->clear();
    ui->comboBox_ExposureMode->addItem(GET_TEXT("IMAGE/37104", "Auto Mode"), 0);
    ui->comboBox_ExposureMode->addItem(GET_TEXT("IMAGE/37203", "Manual Mode"), 1);
    ui->comboBox_ExposureMode->addItem(GET_TEXT("IMAGE/37202", "Schedule Mode"), 2);
    ui->label_ExposureTime->hide();
    ui->comboBox_ExposureTime->hide();
    ui->label_GainLevel->hide();
    ui->slider_GainLevel->hide();
    ui->slider_GainLevel->setRange(1, 100);
    ui->label_ManualModeSettings->hide();
    ui->widget_ManualModeSettings->hide();
    ui->label_ExposureSchedule->hide();
    ui->widget_ExposureSchedule->hide();

    //BLC/WDR/HLC Mode
    ui->comboBox_BlcWdrHlcMode->clear();
    ui->comboBox_BlcWdrHlcMode->addItem(GET_TEXT("IMAGE/37216", "Single Mode"), 0);
    ui->comboBox_BlcWdrHlcMode->addItem(GET_TEXT("IMAGE/37217", "Day/Night Mode"), 1);
    ui->comboBox_BlcWdrHlcMode->addItem(GET_TEXT("IMAGE/37202", "Schedule Mode"), 2);

    //BLC/WDR/HLC
    ui->comboBox_BlcWdrHlc->clear();
    ui->comboBox_BlcWdrHlc->addItem("BLC", 0);
    ui->comboBox_BlcWdrHlc->addItem("WDR", 1);
    ui->comboBox_BlcWdrHlc->addItem("HLC", 2);

    //BLC Region
    ui->comboBox_BlcRegion->clear();
    ui->comboBox_BlcRegion->addItem(GET_TEXT("COMMON/1013", "Off"), 0);
    ui->comboBox_BlcRegion->addItem(GET_TEXT("COMMON/1015", "Customize"), 1);
    ui->comboBox_BlcRegion->addItem(GET_TEXT("IMAGE/37009", "Centre"), 2);

    //Region Type
    ui->comboBox_RegionType->clear();
    ui->comboBox_RegionType->addItem(GET_TEXT("IMAGE/37221", "Inclusive"), 0);
    ui->comboBox_RegionType->addItem(GET_TEXT("IMAGE/37222", "Exclusive"), 1);

    //Wide Dynamic Level
    ui->comboBox_WideDynamicLevel->clear();
    ui->comboBox_WideDynamicLevel->addItem(GET_TEXT("IMAGE/37236", "Low"), 0);
    ui->comboBox_WideDynamicLevel->addItem(GET_TEXT("IMAGE/37237", "High"), 1);
    ui->comboBox_WideDynamicLevel->addItem(GET_TEXT("IMAGE/37238", "Auto"), 2);

    //High Light Compensation
    ui->comboBox_HighLightCompensation->clear();
    ui->comboBox_HighLightCompensation->addItem(GET_TEXT("COMMON/1013", "Off"), 0);
    ui->comboBox_HighLightCompensation->addItem(GET_TEXT("IMAGE/37239", "General Mode"), 1);
    ui->comboBox_HighLightCompensation->addItem(GET_TEXT("IMAGE/37240", "Enhanced Mode"), 2);

    //Day Enhancement Mode
    ui->comboBox_DayEnhancementMode->clear();
    ui->comboBox_DayEnhancementMode->addItem("BLC", 0);
    ui->comboBox_DayEnhancementMode->addItem("WDR", 1);
    ui->comboBox_DayEnhancementMode->addItem("HLC", 2);

    //Night Enhancement Mode
    ui->comboBox_NightEnhancementMode->clear();
    ui->comboBox_NightEnhancementMode->addItem("BLC", 0);
    ui->comboBox_NightEnhancementMode->addItem("WDR", 1);
    ui->comboBox_NightEnhancementMode->addItem("HLC", 2);

    //Wide Dynamic Range
    ui->comboBox_WideDynamicRange->clear();
    ui->comboBox_WideDynamicRange->addItem(GET_TEXT("COMMON/1013", "Off"), 0);
    ui->comboBox_WideDynamicRange->addItem(GET_TEXT("COMMON/1012", "On"), 1);
    ui->comboBox_WideDynamicRange->addItem(GET_TEXT("COMMON/1015", "Customize"), 3);

    //Reduce Motion Blur
    ui->comboBox_reduceBlur->clear();
    ui->comboBox_reduceBlur->addItem(GET_TEXT("COMMON/1013", "Off"), 0);
    ui->comboBox_reduceBlur->addItem(GET_TEXT("COMMON/1012", "On"), 1);

    //Anti-flicker Level
    ui->slider_AntiflickerLevel->setRange(0, 10);

    ui->comboBox_lensType->clear();
    ui->comboBox_lensType->addItem(GET_TEXT("IMAGE/37370", "DC_Iris"), 1);
    ui->comboBox_lensType->addItem(GET_TEXT("IMAGE/37371", "P_Iris"), 2);
    ui->comboBox_lensType->addItem(GET_TEXT("IMAGE/37372", "Manual"), 0);

    connect(ui->slider_AntifogIntensity, SIGNAL(valueChanged(int)), this, SLOT(onChangeDefog()));
    connect(ui->slider_RedGainLevel, SIGNAL(valueChanged(int)), this, SLOT(changeRedGain()));
    connect(ui->slider_BlueGainLevel, SIGNAL(valueChanged(int)), this, SLOT(changeBlueGain()));
    connect(ui->slider_HlcLevel, SIGNAL(valueChanged(int)), this, SLOT(changeHlcLevel()));

    m_whiteBalanceSche = new WhiteBalanceSchedule(this);

    m_manualModeSetting = new ManualModeSettings(this);
    m_exposureSche = new ExposureSchedule(this);
    connect(m_exposureSche, SIGNAL(scheduleAccept()), this, SLOT(scheduleAccept()));
    connect(m_manualModeSetting, SIGNAL(settingFinish()), this, SLOT(settingFinish()));

    m_BlcWdrHlcSche = new BWHSchedule(this);

    onLanguageChanged();
    on_comboBox_BlcWdrHlcMode_activated(0);
    on_comboBox_BlcWdrHlc_activated(0);

    showLensType(false);
}

ImagePageEnhancement::~ImagePageEnhancement()
{
    delete ui;
}

void ImagePageEnhancement::initializeData(int channel)
{
    m_drawEnhance->clearAll();
    setDrawWidget(m_drawEnhance);

    m_channel = channel;

    if (m_manualModeSetting) {
        m_manualModeSetting->clearCache();
    }
    if (m_exposureSche) {
        m_exposureSche->clearCache();
    }

    clearSettings();
    setSettingsEnable(false);

    if (!isChannelConnected()) {
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
        return;
    }
    if (!qMsNvr->isMsCamera(m_channel)) {
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
        return;
    }

    setSettingsEnable(true);

    QString cameraVersion(qMsNvr->cameraInfo(m_channel).fwversion);
    MsCameraModel cameraModel = qMsNvr->cameraRealModel(m_channel);
    int chipVersion = MsCameraVersion(cameraVersion).chipVersion();
    qDebug() << "Channel:" << m_channel << "CameraVersion:" << cameraVersion << "chipVersion:" << chipVersion;
    //安霸平台不支持《Defog Mode》和《Digital Image Stabilisation》，NVR界面安霸机型需要隐藏该选项
    if (qMsNvr->isAmba(m_channel)) {
        ui->label_DigitalAntifogMode->hide();
        ui->comboBox_DigitalAntifogMode->hide();
    }
    //MSHN-9072 QT-Image：鱼眼&4K全景机型&sigma&amba&联咏，Enhancement界面应去掉Digital Image Stabilisation选项
    //4K全景机型包括：全景筒8165、全景半球8176
    if (chipVersion == 6 || qMsNvr->isFisheye(m_channel) || cameraModel.isStartsWith("MS-C8165") || cameraModel.isStartsWith("MS-C8176") || qMsNvr->isSigma(m_channel) || qMsNvr->isAmba(m_channel) || qMsNvr->isNT(m_channel)) {
        ui->label_DigitalImageStabilisation->hide();
        ui->comboBox_DigitalImageStabilisation->hide();
    } else {
        ui->label_DigitalImageStabilisation->show();
        ui->comboBox_DigitalImageStabilisation->show();
    }

    showLensType(false);
    //MsWaitting::showGlobalWait();
    //
    //multi鱼眼，BLC配置：只有O视图支持，其他视图通道下该项置灰不可配置
    if (qMsNvr->isFisheye(m_channel)) {
        sendMessage(REQUEST_FLAG_GET_FISHEYE_MODE, (void *)&m_channel, sizeof(int));
        // int result = m_eventLoop.exec();
        // if (result < 0) {
        //     ui->comboBox_BlcRegion->setEnabled(false);
        // } else {
        //     ui->comboBox_BlcRegion->setEnabled(true);
        // }
    } else {
        ui->comboBox_BlcRegion->setEnabled(true);
    }
    //
    sendMessage(REQUEST_FLAG_GET_SINGLE_IPCTYPE, &m_channel, sizeof(int));
    sendMessage(REQUEST_FLAG_GET_IPCPARAM, &m_channel, sizeof(int));
    sendMessage(REQUEST_FLAG_GET_IPCIMAGE_DISPLAY, &m_channel, sizeof(int));
}

void ImagePageEnhancement::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_FISHEYE_MODE:
        ON_RESPONSE_FLAG_GET_FISHEYE_MODE(message);
        break;
    case RESPONSE_FLAG_GET_SINGLE_IPCTYPE:
        ON_RESPONSE_FLAG_GET_SINGLE_IPCTYPE(message);
        break;
    case RESPONSE_FLAG_GET_IPCPARAM:
        ON_RESPONSE_FLAG_GET_IPCPARAM(message);
        break;
    case RESPONSE_FLAG_GET_IPCIMAGE_ENHANCEMENT:
        ON_RESPONSE_FLAG_GET_IPCIMAGE_ENHANCEMENT(message);
        break;
    case RESPONSE_FLAG_GET_IPC_COMMON_PARAM:
        ON_RESPONSE_FLAG_GET_IPC_COMMON_PARAM(message);
        break;
    case RESPONSE_FLAG_GET_WHITE_BALANCE_SCHE:
        ON_RESPONSE_FLAG_GET_WHITE_BALANCE_SCHE(message);
        break;
    case RESPONSE_FLAG_GET_EXPOSURE_SCHE:
        ON_RESPONSE_FLAG_GET_EXPOSURE_SCHE(message);
        break;
    case RESPONSE_FLAG_GET_BWH_SCHE:
        ON_RESPONSE_FLAG_GET_BWH_SCHE(message);
        break;
    case RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY:
        ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(message);
        break;
    case RESPONSE_FLAG_GET_IPC_LPR_NIGHT_MODE:
        ON_RESPONSE_FLAG_GET_IPC_LPR_NIGHT_MODE(message);
        break;
    default:
        break;
    }
}

void ImagePageEnhancement::clearSettings()
{
    ui->widgetMessage->hideMessage();
    ui->comboBox_BlcWdrHlcMode->setCurrentIndex(0);
    ui->comboBox_WhiteBalance->setCurrentIndex(0);
    ui->comboBox_ExposureMode->setCurrentIndex(0);
    ui->comboBox_DigitalAntifogMode->setCurrentIndex(0);
    ui->comboBox_IrBalanceMode->setCurrentIndex(0);
    ui->comboBox_DigitalImageStabilisation->setCurrentIndex(0);
    ui->comboBox_BlcWdrHlc->setCurrentIndex(0);
    ui->comboBox_WideDynamicRange->setCurrentIndex(0);
    ui->comboBox_reduceBlur->setCurrentIndex(0);
    on_comboBox_BlcWdrHlcMode_activated(ui->comboBox_BlcWdrHlcMode->currentIndex());
    on_comboBox_ExposureMode_activated(ui->comboBox_ExposureMode->currentIndex());
    ui->label_AntifogIntensity->hide();
    ui->slider_AntifogIntensity->hide();
    ui->label_RedGainLevel->hide();
    ui->slider_RedGainLevel->hide();
    ui->label_BlueGainLevel->hide();
    ui->slider_BlueGainLevel->hide();
    ui->label_WhiteBalanceSchedule->hide();
    ui->widget_WhiteBalanceSchedule->hide();
    ui->label_deblur->hide();
    ui->slider_deblur->hide();
    ui->label_lensType->hide();
    ui->comboBox_lensType->hide();
    ui->label_reduceVideoStuttering->hide();
    ui->comboBox_reduceVideoStuttering->hide();

    ui->label_reduceBlur->show();
    ui->comboBox_reduceBlur->show();
    ui->label_DigitalAntifogMode->show();
    ui->comboBox_DigitalAntifogMode->show();
    ui->label_ExposureMode->show();
    ui->comboBox_ExposureMode->show();
}

void ImagePageEnhancement::setSettingsEnable(bool enable)
{
    ui->widgetContainer->setEnabled(enable);

    ui->pushButton_copy->setEnabled(enable);
    ui->pushButton_apply->setEnabled(enable);
}

void ImagePageEnhancement::ON_RESPONSE_FLAG_GET_IPC_LPR_NIGHT_MODE(MessageReceive *message)
{
    resp_lpr_night_mode *data = reinterpret_cast<resp_lpr_night_mode *>(message->data);
    if (!data) {
        //MsWaitting::closeGlobalWait();
        qMsWarning() << "data is null";
        gEventLoopExit(0);
        return;
    }
    saveEnhancement(data->chnid, data->nightMode);
    gEventLoopExit(0);
}

void ImagePageEnhancement::ON_RESPONSE_FLAG_GET_IPCIMAGE_ENHANCEMENT(MessageReceive *message)
{
    struct resp_image_enhancement *data = (struct resp_image_enhancement *)message->data;
    if (!data) {
        qMsWarning() << "data is null.";
        return;
    }
    memcpy(&m_info, &(data->image), sizeof(struct image_enhancement));
    initEnhancement();
}

void ImagePageEnhancement::showLensType(bool enable)
{
    if (enable) {
        ui->label_lensType->show();
        ui->comboBox_lensType->show();
    } else {
        ui->label_lensType->hide();
        ui->comboBox_lensType->hide();
    }
}

void ImagePageEnhancement::ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(MessageReceive *message)
{
    resp_image_display *data = (resp_image_display *)message->data;
    if (!data) {
        //MsWaitting::closeGlobalWait();
        qMsWarning() << "data is null.";
        return;
    }

    m_powerLineFrequency = static_cast<PowerLineFrequency>(data->image.exposurectrl);

    //枪机显示Lens Tyep选项。
    QString model = QString("%1").arg(data->image.productmodel);
    int tmp = model.mid(6, 1).toInt();
    if (tmp == 5) {
        showLensType(true);
    }
    //全彩机型白灯功能隐藏IR Balance Mode
    if (data->image.whiteled.support) {
        ui->label_IrBalanceMode->hide();
        ui->comboBox_IrBalanceMode->hide();
    } else {
        ui->label_IrBalanceMode->show();
        ui->comboBox_IrBalanceMode->show();
    }

    //
    sendMessage(REQUEST_FLAG_GET_IPCIMAGE_ENHANCEMENT, &m_channel, sizeof(int));
    //为了可以copy，先获取
    sendMessage(REQUEST_FLAG_GET_WHITE_BALANCE_SCHE, &m_channel, sizeof(int));
    sendMessage(REQUEST_FLAG_GET_EXPOSURE_SCHE, &m_channel, sizeof(int));
    sendMessage(REQUEST_FLAG_GET_BWH_SCHE, &m_channel, sizeof(int));

    ////showWait();
}

void ImagePageEnhancement::ON_RESPONSE_FLAG_GET_IPC_COMMON_PARAM(MessageReceive *message)
{
    struct resp_http_common_param *param = (struct resp_http_common_param *)message->data;
    if (!param) {
        qMsWarning() << "data is null.";
        return;
    }
    if (QString(param->info.key) == "redgain") {
        int value = QString(param->info.value).toInt();
        ui->slider_RedGainLevel->setValue(value);
    } else if (QString(param->info.key) == "bluegain") {
        int value = QString(param->info.value).toInt();
        ui->slider_BlueGainLevel->setValue(value);
    } else if (QString(param->info.key) == "defogmode") {
        int value = QString(param->info.value).toInt();
        ui->comboBox_DigitalAntifogMode->setCurrentIndex(value);
        on_comboBox_DigitalAntifogMode_activated(ui->comboBox_DigitalAntifogMode->currentIndex() && ui->comboBox_DigitalAntifogMode->isVisible());
    } else if (QString(param->info.key) == "defoglevel") {
        int value = QString(param->info.value).toInt();
        ui->slider_AntifogIntensity->setValue(value);
    } else if (QString(param->info.key) == "dismode") {
        int value = QString(param->info.value).toInt();
        ui->comboBox_DigitalImageStabilisation->setCurrentIndex(value);
    } else if (QString(param->info.key) == "hlclevel") {
        int value = QString(param->info.value).toInt();
        ui->slider_HlcLevel->setValue(value);
    } else if (QString(param->info.key) == "exposetime") {
        int value = QString(param->info.value).toInt();
        ui->comboBox_ExposureTime->setCurrentIndexFromData(value);
    } else if (QString(param->info.key) == "exposegain") {
        int value = QString(param->info.value).toInt();
        ui->slider_GainLevel->setValue(value);
    } else if (QString(param->info.key) == "wdraflevel") {
        int value = QString(param->info.value).toInt();
        ui->slider_AntiflickerLevel->setValue(value);
    }
}

void ImagePageEnhancement::ON_RESPONSE_FLAG_GET_WHITE_BALANCE_SCHE(MessageReceive *message)
{
    m_whiteBalanceSche->ON_RESPONSE_FLAG_GET_WHITE_BALANCE_SCHE(message);
}

void ImagePageEnhancement::ON_RESPONSE_FLAG_GET_EXPOSURE_SCHE(MessageReceive *message)
{
    if (m_manualModeSetting) {
        m_manualModeSetting->dealMessage(message);
    }
    //
    m_exposureSche->ON_RESPONSE_FLAG_GET_EXPOSURE_SCHE(message);
}

void ImagePageEnhancement::ON_RESPONSE_FLAG_GET_BWH_SCHE(MessageReceive *message)
{
    //MsWaitting::closeGlobalWait();
    m_BlcWdrHlcSche->ON_RESPONSE_FLAG_GET_BWH_SCHE(message);
}

void ImagePageEnhancement::showBlcWdrHlcMode()
{
    if (m_info.regiontype != 0) {
        ui->comboBox_BlcWdrHlc->setCurrentIndex(0);
    } else if (m_info.wdrenable != 0) {
        ui->comboBox_BlcWdrHlc->setCurrentIndex(1);
    } else if (m_info.hlcmode) {
        ui->comboBox_BlcWdrHlc->setCurrentIndex(2);
    }
}

void ImagePageEnhancement::initExposureTime()
{
    if (!m_info.exposurectrl) {
        ui->comboBox_ExposureTime->clear();
        ui->comboBox_ExposureTime->addItem("1", 13);
        ui->comboBox_ExposureTime->addItem("1/5", 0);
        ui->comboBox_ExposureTime->addItem("1/15", 1);
        ui->comboBox_ExposureTime->addItem("1/30", 2);
        ui->comboBox_ExposureTime->addItem("1/60", 3);
        ui->comboBox_ExposureTime->addItem("1/120", 4);
        ui->comboBox_ExposureTime->addItem("1/250", 5);
        ui->comboBox_ExposureTime->addItem("1/500", 6);
        ui->comboBox_ExposureTime->addItem("1/750", 7);
        ui->comboBox_ExposureTime->addItem("1/1000", 8);
        ui->comboBox_ExposureTime->addItem("1/2000", 9);
        ui->comboBox_ExposureTime->addItem("1/4000", 10);
        ui->comboBox_ExposureTime->addItem("1/10000", 11);
        ui->comboBox_ExposureTime->addItem("1/100000", 12);
        ui->comboBox_ExposureTime->setCurrentIndexFromData(2);
    } else {
        ui->comboBox_ExposureTime->clear();
        ui->comboBox_ExposureTime->addItem("1", 13);
        ui->comboBox_ExposureTime->addItem("1/5", 0);
        ui->comboBox_ExposureTime->addItem("1/10", 1);
        ui->comboBox_ExposureTime->addItem("1/25", 2);
        ui->comboBox_ExposureTime->addItem("1/50", 3);
        ui->comboBox_ExposureTime->addItem("1/100", 4);
        ui->comboBox_ExposureTime->addItem("1/250", 5);
        ui->comboBox_ExposureTime->addItem("1/500", 6);
        ui->comboBox_ExposureTime->addItem("1/750", 7);
        ui->comboBox_ExposureTime->addItem("1/1000", 8);
        ui->comboBox_ExposureTime->addItem("1/2000", 9);
        ui->comboBox_ExposureTime->addItem("1/4000", 10);
        ui->comboBox_ExposureTime->addItem("1/10000", 11);
        ui->comboBox_ExposureTime->addItem("1/100000", 12);
        ui->comboBox_ExposureTime->setCurrentIndexFromData(2);
    }
}

void ImagePageEnhancement::initEnhancement()
{
    ui->comboBox_IrBalanceMode->setCurrentIndex(m_info.smartir);
    ui->comboBox_reduceVideoStuttering->setCurrentIndex(m_info.reduce_stuttering);
    ui->comboBox_WhiteBalance->setCurrentIndex(m_info.whiteblance);
    ui->comboBox_ExposureMode->setCurrentIndex(m_info.exposemode);
    ui->comboBox_BlcWdrHlcMode->setCurrentIndex(m_info.imageschemode);
    ui->comboBox_BlcRegion->setCurrentIndex(m_info.regiontype);

    initHighLightCompensation(m_info.imageschemode);
    ui->comboBox_RegionType->setCurrentIndex(m_info.exclude);
    ui->comboBox_WideDynamicRange->setCurrentIndexFromData(m_info.wdrenable);
    wdrIsOn = (m_info.wdrenable != 0) ? true : false;
    ui->comboBox_WideDynamicLevel->setCurrentIndex(m_info.wdrlevel);
    ui->comboBox_DayEnhancementMode->setCurrentIndex(m_info.hlcday);
    ui->comboBox_NightEnhancementMode->setCurrentIndex(m_info.hlcnight);
    ui->timeEdit_startTime->setTime(QTime(m_info.wdrstarthour, m_info.wdrstartminute));
    ui->timeEdit_endTime->setTime(QTime(m_info.wdrstophour, m_info.wdrstopminute));
    ui->comboBox_reduceBlur->setCurrentIndexFromData(m_info.enblur);
    ui->slider_deblur->setValue(m_info.deblur);
    ui->comboBox_lensType->setEnabled(!qMsNvr->isSigma(m_channel));
    ui->comboBox_lensType->setCurrentIndexFromData(qMsNvr->isSigma(m_channel) ? 0 : m_info.lensType);
    ui->comboBox_BlcWdrHlc->setCurrentIndex(0);

    showBlcWdrHlcMode();
    on_comboBox_BlcWdrHlcMode_activated(ui->comboBox_BlcWdrHlcMode->currentIndex());
    on_comboBox_WhiteBalance_activated(ui->comboBox_WhiteBalance->currentIndex());
    if (m_info.fullcolorSupport && qMsNvr->isSigma(m_channel)) {
        ui->comboBox_ExposureMode->hide();
        ui->label_ExposureMode->hide();
    } else {
        on_comboBox_ExposureMode_activated(ui->comboBox_ExposureMode->currentIndex());
    }

    if (MsSdkVersion("2.4.02") < MsSdkVersion(m_info.sdkversion)) {
        ui->label_reduceBlur->show();
        ui->label_deblur->show();
        ui->comboBox_reduceBlur->show();
        ui->slider_deblur->show();
        on_comboBox_reduceBlur_activated();
    } else {
        ui->label_reduceBlur->hide();
        ui->label_deblur->hide();
        ui->comboBox_reduceBlur->hide();
        ui->slider_deblur->hide();
    }

    initExposureTime();
    ui->comboBox_ExposureTime->setCurrentIndexFromData(m_info.exposetime);
    ui->slider_GainLevel->setValue(m_info.exposegain);
    drawEnhanceArea();
    getCommonParam("redgain");
    getCommonParam("bluegain");
    getCommonParam("defogmode");
    getCommonParam("defoglevel");
    getCommonParam("dismode");
    getCommonParam("hlclevel");
    //getCommonParam("exposetime");
    //getCommonParam("exposegain");
    getCommonParam("wdraflevel");
}

void ImagePageEnhancement::drawEnhanceArea()
{
    int left = 0, top = 0, right = 0, bottom = 0;
    int regiontype = ui->comboBox_BlcRegion->currentIndex();

    m_drawEnhance->clearAll();
    setDrawWidget(m_drawEnhance);
    if (!regiontype) {
        setDrawWidget(nullptr);
    } else if (regiontype == 1) {
        //customize
        QString area = QString("%1").arg(m_info.exposeregion);
        QStringList areaList = area.split(":");
        if (areaList.size() == 4) {
            left = areaList.at(0).toInt();
            top = areaList.at(1).toInt();
            right = areaList.at(2).toInt();
            bottom = areaList.at(3).toInt();
        }
        qDebug() << "yun debug l:" << left << " t:" << top << " r:" << right << " b:" << bottom;
        m_drawEnhance->setBlcType(BlcCustomize);
        m_drawEnhance->addEnhanceArea(left, top, right, bottom);
    } else {
        //centre
        m_drawEnhance->setBlcType(BlcCentre);
        m_drawEnhance->addCentreArea();
    }
}

void ImagePageEnhancement::on_comboBox_WhiteBalance_activated(int index)
{
    const int mode = ui->comboBox_WhiteBalance->itemData(index).toInt();
    setCommonParam(m_channel, "whiteblance", mode);
    switch (mode) {
    case 0: //Auto White Balance
    case 2: //Incandescent Lamp
    case 3: //Warm Light Lamp
    case 4: //Natural Light
    case 5: //Fluorescent Lamp
        ui->label_RedGainLevel->hide();
        ui->slider_RedGainLevel->hide();
        ui->label_BlueGainLevel->hide();
        ui->slider_BlueGainLevel->hide();
        ui->label_WhiteBalanceSchedule->hide();
        ui->widget_WhiteBalanceSchedule->hide();
        break;
    case 1: //Manual White Balance
        ui->label_RedGainLevel->show();
        ui->slider_RedGainLevel->show();
        ui->label_BlueGainLevel->show();
        ui->slider_BlueGainLevel->show();
        ui->label_WhiteBalanceSchedule->hide();
        ui->widget_WhiteBalanceSchedule->hide();
        break;
    case 6: //Schedule Mode
        ui->label_RedGainLevel->show();
        ui->slider_RedGainLevel->show();
        ui->label_BlueGainLevel->show();
        ui->slider_BlueGainLevel->show();
        ui->label_WhiteBalanceSchedule->show();
        ui->widget_WhiteBalanceSchedule->show();
        break;
    default:
        break;
    }
}

void ImagePageEnhancement::on_pushButton_WhiteBalanceSchedule_clicked()
{
    m_whiteBalanceSche->showAction(m_channel);
    m_whiteBalanceSche->exec();
}

void ImagePageEnhancement::on_comboBox_DigitalAntifogMode_activated(int index)
{
    const int mode = ui->comboBox_DigitalAntifogMode->itemData(index).toInt();
    switch (mode) {
    case 0: //Off
        ui->label_AntifogIntensity->hide();
        ui->slider_AntifogIntensity->hide();
        break;
    case 1: //On
        ui->label_AntifogIntensity->show();
        ui->slider_AntifogIntensity->show();
        break;
    }
    setCommonParam(m_channel, "defogmode", ui->comboBox_DigitalAntifogMode->currentIndex());
}

void ImagePageEnhancement::on_comboBox_DigitalImageStabilisation_activated(int index)
{
    const int mode = ui->comboBox_DigitalImageStabilisation->itemData(index).toInt();
    setCommonParam(m_channel, "dismode", mode);
}

void ImagePageEnhancement::on_comboBox_ExposureMode_activated(int index)
{
    const int mode = ui->comboBox_ExposureMode->itemData(index).toInt();
    //避免显示超出范围，先把控件隐藏
    ui->label_ExposureTime->hide();
    ui->comboBox_ExposureTime->hide();
    ui->label_GainLevel->hide();
    ui->slider_GainLevel->hide();
    ui->label_ManualModeSettings->hide();
    ui->widget_ManualModeSettings->hide();
    ui->label_ExposureSchedule->hide();
    ui->widget_ExposureSchedule->hide();
    //
    switch (mode) {
    case 0: //Auto Mode
        break;
    case 1: //Manual Mode
        ui->label_ExposureTime->show();
        ui->comboBox_ExposureTime->show();
        ui->label_GainLevel->show();
        ui->slider_GainLevel->show();
        break;
    case 2: //Schedule Mode
        ui->label_ManualModeSettings->show();
        ui->widget_ManualModeSettings->show();
        ui->label_ExposureSchedule->show();
        ui->widget_ExposureSchedule->show();
        break;
    }
}

void ImagePageEnhancement::on_comboBox_reduceBlur_activated()
{
    if (ui->comboBox_reduceBlur->currentData().toInt() == 0) {
        ui->label_deblur->hide();
        ui->slider_deblur->hide();
    } else {
        ui->label_deblur->show();
        ui->slider_deblur->show();
    }
}

void ImagePageEnhancement::on_pushButton_ManualModeSettings_clicked()
{
    if (!m_manualModeSetting) {
        m_manualModeSetting = new ManualModeSettings(this);
    }
    m_manualModeSetting->setExposureCtrl(m_info.exposurectrl);
    m_manualModeSetting->initializeData(m_channel);
    m_manualModeSetting->show();

    ui->pushButton_ManualModeSettings->clearUnderMouse();
}

void ImagePageEnhancement::on_pushButton_ExposureSchedule_clicked()
{
    m_exposureSche->showAction(m_channel);
    m_exposureSche->exec();

    ui->pushButton_ExposureSchedule->clearUnderMouse();
}

void ImagePageEnhancement::on_pushButton_BlcWdrHlcSchedule_clicked()
{
    m_BlcWdrHlcSche->showAction(m_channel);
    m_BlcWdrHlcSche->exec();

    ui->pushButton_BlcWdrHlcSchedule->clearUnderMouse();
}

void ImagePageEnhancement::initHighLightCompensation(int type)
{
    int value = m_info.hlcmode;
    if (type == 0) {
        ui->comboBox_HighLightCompensation->clear();
        ui->comboBox_HighLightCompensation->addItem(GET_TEXT("COMMON/1013", "Off"), 0);
        ui->comboBox_HighLightCompensation->addItem(GET_TEXT("IMAGE/37239", "General Mode"), 1);
        ui->comboBox_HighLightCompensation->addItem(GET_TEXT("IMAGE/37240", "Enhanced Mode"), 2);
    } else {
        ui->comboBox_HighLightCompensation->clear();
        ui->comboBox_HighLightCompensation->addItem(GET_TEXT("IMAGE/37239", "General Mode"), 1);
        ui->comboBox_HighLightCompensation->addItem(GET_TEXT("IMAGE/37240", "Enhanced Mode"), 2);
        value = m_info.hlcmode == 0 ? 1 : m_info.hlcmode;
    }

    ui->comboBox_HighLightCompensation->setCurrentIndexFromData(value);
}

void ImagePageEnhancement::on_comboBox_BlcWdrHlcMode_activated(int index)
{
    const int mode = ui->comboBox_BlcWdrHlcMode->itemData(index).toInt();
    hideBWHAll();
    initHighLightCompensation(mode);
    switch (mode) {
    case 0: //Single Mode
        ui->label_BlcWdrHlc->show();
        ui->comboBox_BlcWdrHlc->show();
        ui->label_BlcRegion->show();
        ui->comboBox_BlcRegion->show();
        on_comboBox_BlcWdrHlc_activated(ui->comboBox_BlcWdrHlc->currentIndex());
        break;
    case 1: //Day/Night Mode
        ui->label_BlcRegion->show();
        ui->comboBox_BlcRegion->show();
        ui->label_WideDynamicLevel->show();
        ui->comboBox_WideDynamicLevel->show();
        ui->label_AntiflickerLevel->show();
        ui->slider_AntiflickerLevel->show();
        ui->label_HighLightCompensation->show();
        ui->comboBox_HighLightCompensation->show();
        ui->label_HlcLevel->show();
        ui->slider_HlcLevel->show();
        ui->label_DayEnhancementMode->show();
        ui->comboBox_DayEnhancementMode->show();
        ui->label_NightEnhancementMode->show();
        ui->comboBox_NightEnhancementMode->show();
        break;
    case 2: //Schedule Mode
        ui->label_BlcRegion->show();
        ui->comboBox_BlcRegion->show();
        ui->label_WideDynamicLevel->show();
        ui->comboBox_WideDynamicLevel->show();
        ui->label_AntiflickerLevel->show();
        ui->slider_AntiflickerLevel->show();
        ui->label_HighLightCompensation->show();
        ui->comboBox_HighLightCompensation->show();
        ui->label_HlcLevel->show();
        ui->slider_HlcLevel->show();
        ui->label_BlcWdrHlcSchedule->show();
        ui->pushButton_BlcWdrHlcSchedule->show();
        break;
    }
    on_comboBox_BlcRegion_activated(ui->comboBox_BlcRegion->currentIndex());
}

void ImagePageEnhancement::on_comboBox_BlcRegion_activated(int index)
{
    if (!ui->comboBox_BlcRegion->isVisible())
        return;

    const int mode = ui->comboBox_BlcRegion->itemData(index).toInt();
    switch (mode) {
    case 0: //off
        ui->label_RegionType->hide();
        ui->comboBox_RegionType->hide();
        break;
    case 1: //customize
        ui->label_RegionType->show();
        ui->comboBox_RegionType->show();
        break;
    case 2: //centre
        ui->label_RegionType->hide();
        ui->comboBox_RegionType->hide();
        break;
    }
    drawEnhanceArea();
}

void ImagePageEnhancement::on_comboBox_BlcWdrHlc_activated(int index)
{
    const int mode = ui->comboBox_BlcWdrHlc->itemData(index).toInt();
    hideBWHAll();
    ui->label_BlcWdrHlc->show();
    ui->comboBox_BlcWdrHlc->show();
    switch (mode) {
    case 0: //blc
        ui->label_BlcRegion->show();
        ui->comboBox_BlcRegion->show();
        break;
    case 1: //wdr
        ui->label_WideDynamicRange->show();
        ui->comboBox_WideDynamicRange->show();
        //MSHN-5657
        //1、电源频率为60Hz时，当帧率达到30fps以上（不含30），WDR & HLC的普通模式需禁用
        //2、电源频率为50Hz时，当帧率达到25fps以上（不含25），WDR & HLC的普通模式需禁用
        if ((m_powerLineFrequency == Frequency_60Hz && m_mainFrameRate > 30) || (m_powerLineFrequency == Frequency_50Hz && m_mainFrameRate > 25)) {
            ui->comboBox_WideDynamicRange->setCurrentIndexFromData(0);
            ui->comboBox_WideDynamicRange->setEnabled(false);
        } else {
            ui->comboBox_WideDynamicRange->setEnabled(true);
            initWideDynamicRange(ui->comboBox_WideDynamicRange->currentData().toInt());
        }
        break;
    case 2: //hlc
        ui->label_HighLightCompensation->show();
        ui->comboBox_HighLightCompensation->show();
        //MSHN-5657
        //1、电源频率为60Hz时，当帧率达到30fps以上（不含30），WDR & HLC的普通模式需禁用
        //2、电源频率为50Hz时，当帧率达到25fps以上（不含25），WDR & HLC的普通模式需禁用
        if ((m_powerLineFrequency == Frequency_60Hz && m_mainFrameRate > 30) || (m_powerLineFrequency == Frequency_50Hz && m_mainFrameRate > 25)) {
            int value = ui->comboBox_HighLightCompensation->currentIntData();
            if (value == 1) {
                value = 0;
            }
            ui->comboBox_HighLightCompensation->clear();
            ui->comboBox_HighLightCompensation->addItem(GET_TEXT("COMMON/1013", "Off"), 0);
            ui->comboBox_HighLightCompensation->addItem(GET_TEXT("IMAGE/37240", "Enhanced Mode"), 2);
            ui->comboBox_HighLightCompensation->setCurrentIndexFromData(value);
        }
        if (ui->comboBox_HighLightCompensation->currentData().toInt() != 0) {
            ui->label_HlcLevel->show();
            ui->slider_HlcLevel->show();
        }
        break;
    }
    on_comboBox_BlcRegion_activated(ui->comboBox_BlcRegion->currentIndex());
}

void ImagePageEnhancement::on_comboBox_HighLightCompensation_activated()
{
    if (ui->comboBox_HighLightCompensation->currentData().toInt() == 0) {
        ui->label_HlcLevel->hide();
        ui->slider_HlcLevel->hide();
    } else {
        ui->label_HlcLevel->show();
        ui->slider_HlcLevel->show();
    }
}

void ImagePageEnhancement::on_comboBox_WideDynamicRange_activated(int index)
{
    const int mode = ui->comboBox_WideDynamicRange->itemData(index).toInt();
    ui->label_startTime->hide();
    ui->label_endTime->hide();
    ui->timeEdit_startTime->hide();
    ui->timeEdit_endTime->hide();
    ui->label_WideDynamicLevel->hide();
    ui->comboBox_WideDynamicLevel->hide();
    ui->label_AntiflickerLevel->hide();
    ui->slider_AntiflickerLevel->hide();
    initWideDynamicRange(mode);
}

void ImagePageEnhancement::initWideDynamicRange(int index)
{
    switch (index) {
    case 0: //off
        break;
    case 1: //on
        ui->label_WideDynamicLevel->show();
        ui->comboBox_WideDynamicLevel->show();
        ui->label_AntiflickerLevel->show();
        ui->slider_AntiflickerLevel->show();
        break;
    case 3: //customize
        ui->label_startTime->show();
        ui->label_endTime->show();
        ui->timeEdit_startTime->show();
        ui->timeEdit_endTime->show();
        ui->label_WideDynamicLevel->show();
        ui->comboBox_WideDynamicLevel->show();
        ui->label_AntiflickerLevel->show();
        ui->slider_AntiflickerLevel->show();
        break;
    }
}

void ImagePageEnhancement::hideBWHAll()
{
    ui->label_BlcWdrHlc->hide();
    ui->comboBox_BlcWdrHlc->hide();
    ui->label_BlcRegion->hide();
    ui->comboBox_BlcRegion->hide();
    ui->label_RegionType->hide();
    ui->comboBox_RegionType->hide();
    ui->label_WideDynamicRange->hide();
    ui->comboBox_WideDynamicRange->hide();
    ui->label_WideDynamicLevel->hide();
    ui->comboBox_WideDynamicLevel->hide();
    ui->label_AntiflickerLevel->hide();
    ui->slider_AntiflickerLevel->hide();
    ui->label_HighLightCompensation->hide();
    ui->comboBox_HighLightCompensation->hide();
    ui->label_HlcLevel->hide();
    ui->slider_HlcLevel->hide();
    ui->label_DayEnhancementMode->hide();
    ui->comboBox_DayEnhancementMode->hide();
    ui->label_NightEnhancementMode->hide();
    ui->comboBox_NightEnhancementMode->hide();
    ui->label_BlcWdrHlcSchedule->hide();
    ui->pushButton_BlcWdrHlcSchedule->hide();
    ui->label_startTime->hide();
    ui->label_endTime->hide();
    ui->timeEdit_startTime->hide();
    ui->timeEdit_endTime->hide();
}

void ImagePageEnhancement::onLanguageChanged()
{
    ui->label_IrBalanceMode->setText(GET_TEXT("IMAGE/37200", "IR Balance Mode"));
    ui->label_reduceVideoStuttering->setText(GET_TEXT("IMAGE/37008", "Reduce Video Stuttering"));
    ui->label_WhiteBalance->setText(GET_TEXT("IMAGE/37233", "White Balance"));
    ui->label_RedGainLevel->setText(GET_TEXT("IMAGE/37204", "Red Gain Level"));
    ui->label_BlueGainLevel->setText(GET_TEXT("IMAGE/37205", "Blue Gain Level"));
    ui->label_WhiteBalanceSchedule->setText(GET_TEXT("IMAGE/37206", "White Balance Schedule"));

    ui->label_DigitalAntifogMode->setText(GET_TEXT("IMAGE/37207", "Defog Mode"));
    ui->label_AntifogIntensity->setText(GET_TEXT("IMAGE/37234", "Anti-fog Intensity"));
    ui->label_DigitalImageStabilisation->setText(GET_TEXT("IMAGE/37208", "Digital Image Stabilisation"));

    ui->label_ExposureMode->setText(GET_TEXT("IMAGE/37209", "Exposure Mode"));
    ui->label_ExposureTime->setText(GET_TEXT("IMAGE/37210", "Exposure Time"));
    ui->label_GainLevel->setText(GET_TEXT("IMAGE/37211", "Gain Level"));
    ui->label_ManualModeSettings->setText(GET_TEXT("IMAGE/37212", "Manual Mode Settings"));
    ui->label_ExposureSchedule->setText(GET_TEXT("IMAGE/37214", "Exposure Schedule"));

    ui->pushButton_WhiteBalanceSchedule->setText(GET_TEXT("IMAGE/37314", "Edit"));
    ui->pushButton_ManualModeSettings->setText(GET_TEXT("IMAGE/37314", "Edit"));
    ui->pushButton_ExposureSchedule->setText(GET_TEXT("IMAGE/37314", "Edit"));
    ui->pushButton_BlcWdrHlcSchedule->setText(GET_TEXT("IMAGE/37314", "Edit"));

    ui->label_BlcWdrHlcMode->setText(GET_TEXT("IMAGE/37215", "BLC/WDR/HLC Mode"));
    ui->label_BlcWdrHlc->setText(GET_TEXT("IMAGE/37218", "BLC/WDR/HLC"));
    ui->label_BlcRegion->setText(GET_TEXT("IMAGE/37219", "BLC Region"));
    ui->label_RegionType->setText(GET_TEXT("IMAGE/37220", "Region Type"));
    ui->label_WideDynamicRange->setText(GET_TEXT("IMAGE/37235", "Wide Dynamic Range"));
    ui->label_WideDynamicLevel->setText(GET_TEXT("IMAGE/37223", "Wide Dynamic Level"));
    ui->label_AntiflickerLevel->setText(GET_TEXT("IMAGE/37224", "Anti-flicker Level"));
    ui->label_HighLightCompensation->setText(GET_TEXT("IMAGE/37225", "High Light Compensation"));
    ui->label_HlcLevel->setText(GET_TEXT("IMAGE/37226", "HLC Level"));
    ui->label_DayEnhancementMode->setText(GET_TEXT("IMAGE/37228", "Day Enhancement Mode"));
    ui->label_NightEnhancementMode->setText(GET_TEXT("IMAGE/37229", "Night Enhancement Mode"));
    ui->label_BlcWdrHlcSchedule->setText(GET_TEXT("IMAGE/37227", "BLC/WDR/HLC Schedule"));

    ui->label_startTime->setText(GET_TEXT("IMAGE/37320", "Start Time"));
    ui->label_endTime->setText(GET_TEXT("IMAGE/37321", "End Time"));

    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_copy->setText(GET_TEXT("COMMON/1005", "Copy"));

    ui->label_reduceBlur->setText(GET_TEXT("IMAGE/37130", "Reduce Motion Blur"));
    ui->label_deblur->setText(GET_TEXT("IMAGE/37131", "Deblur Level"));

    ui->label_lensType->setText(GET_TEXT("IMAGE/37369", "Lens Type"));
}

void ImagePageEnhancement::ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message)
{
    Q_UNUSED(message)
}

void ImagePageEnhancement::ON_RESPONSE_FLAG_GET_SINGLE_IPCTYPE(MessageReceive *message)
{
    Q_UNUSED(message)
}

void ImagePageEnhancement::ON_RESPONSE_FLAG_GET_IPCPARAM(MessageReceive *message)
{
    resp_get_ipc_param *ipc_param = (resp_get_ipc_param *)message->data;
    if (ipc_param) {
        m_mainFrameRate = ipc_param->main_range.framerate;
    }
}

void ImagePageEnhancement::getCommonParam(const QString &param)
{
    struct req_http_common_param common_param;
    memset(&common_param, 0, sizeof(struct req_http_common_param));

    common_param.chnid = m_channel;
    if (strcmp(param.toStdString().c_str(), "getadcvalue") == 0) {
        snprintf(common_param.info.httpname, sizeof(common_param.info.httpname), "%s", "camera_display.html");
        common_param.info.noparatest = 1;
    } else {
        snprintf(common_param.info.httpname, sizeof(common_param.info.httpname), "%s", "camera_extra.html");
        snprintf(common_param.info.pagename, sizeof(common_param.info.pagename), "%s", "image");
        common_param.info.noparatest = 0;
    }
    snprintf(common_param.info.key, sizeof(common_param.info.key), "%s", param.toStdString().c_str());

    sendMessage(REQUEST_FLAG_GET_IPC_COMMON_PARAM, &common_param, sizeof(struct req_http_common_param));
}

void ImagePageEnhancement::setCommonParam(int channel, const QString &param, int value)
{
    struct req_http_common_param common_param;
    memset(&common_param, 0, sizeof(struct req_http_common_param));

    common_param.chnid = channel;
    snprintf(common_param.info.httpname, sizeof(common_param.info.httpname), "%s", "camera_extra.html");
    snprintf(common_param.info.pagename, sizeof(common_param.info.pagename), "%s", "image");
    snprintf(common_param.info.key, sizeof(common_param.info.key), "%s", param.toStdString().c_str());
    snprintf(common_param.info.value, sizeof(common_param.info.value), "%d", value);

    sendMessage(REQUEST_FLAG_SET_IPC_COMMON_PARAM, &common_param, sizeof(struct req_http_common_param));
}

void ImagePageEnhancement::saveEnhancement(int channel, bool isNightMode)
{
    char exposeregion[64] = { 0 };
    struct req_image_enhancement req;
    memset(&req, 0, sizeof(struct req_image_enhancement));
    memcpy(&req.image, &m_info, sizeof(struct image_enhancement));
    struct image_exposure_area area;
    memset(&area, 0, sizeof(struct image_exposure_area));

    m_drawEnhance->getEnhanceArea(&area);
    snprintf(exposeregion, sizeof(exposeregion), "%d:%d:%d:%d", area.left, area.top, area.right, area.bottom);

    req.chnid = channel;
    req.image.smartir = ui->comboBox_IrBalanceMode->currentIndex();
    req.image.reduce_stuttering = ui->comboBox_reduceVideoStuttering->currentIndex();
    req.image.whiteblance = ui->comboBox_WhiteBalance->currentIndex();
    req.image.enblur = ui->comboBox_reduceBlur->currentData().toInt();
    if (req.image.enblur == 1) {
        req.image.deblur = ui->slider_deblur->value();
    }
    if (req.image.whiteblance == 1 || req.image.whiteblance == 6) //manual white balance || schedule mode
    {
        req.image.redgain = ui->slider_RedGainLevel->value();
        req.image.bluegain = ui->slider_BlueGainLevel->value();
    }
    req.image.defogmode = ui->comboBox_DigitalAntifogMode->currentIndex();
    req.image.defoglevel = ui->slider_AntifogIntensity->value();
    req.image.exposemode = ui->comboBox_ExposureMode->currentIndex();
    if (req.image.exposemode == 1) //manual Mode
    {
        req.image.exposetime = ui->comboBox_ExposureTime->currentData().toInt();
        req.image.exposegain = ui->slider_GainLevel->value();
    }

    req.image.imageschemode = ui->comboBox_BlcWdrHlcMode->currentIndex();
    if (req.image.imageschemode == 0) //single Mode
    {
        if (ui->comboBox_BlcWdrHlc->currentIndex() == 0) //blc
        {
            req.image.regiontype = ui->comboBox_BlcRegion->currentIndex();
            if (req.image.regiontype == 0) //off
            {
                req.image.wdrenable = 0;
                req.image.hlcmode = 0;
                snprintf(req.image.exposeregion, sizeof(req.image.exposeregion), "%s", "0:0:0:0");
            } else {
                req.image.wdrenable = 0;
                req.image.hlcmode = 0;
                if (req.image.regiontype == 1) //customize
                {
                    snprintf(req.image.exposeregion, sizeof(req.image.exposeregion), "%s", exposeregion);
                    req.image.exclude = ui->comboBox_RegionType->currentIndex();
                } else //centre
                {
                    snprintf(req.image.exposeregion, sizeof(req.image.exposeregion), "%s", "0:0:0:0");
                }
            }
        } else if (ui->comboBox_BlcWdrHlc->currentIndex() == 1) //wdr
        {
            req.image.wdrenable = ui->comboBox_WideDynamicRange->currentData().toInt();
            if (req.image.wdrenable != 0) //off
            {
                req.image.regiontype = 0;
                req.image.hlcmode = 0;
                if (req.image.wdrenable == 1) //on
                {
                    req.image.wdrlevel = ui->comboBox_WideDynamicLevel->currentIndex();
                    req.image.wdraflevel = ui->slider_AntiflickerLevel->value();
                    //setCommonParam(channel, "aflevel", ui->slider_AntiflickerLevel->value());
                } else //customize
                {
                    QTime timeStart = ui->timeEdit_startTime->time();
                    QTime timeEnd = ui->timeEdit_endTime->time();
                    req.image.wdrstarthour = timeStart.hour();
                    req.image.wdrstartminute = timeStart.minute();
                    req.image.wdrstophour = timeEnd.hour();
                    req.image.wdrstopminute = timeEnd.minute();
                    req.image.wdrlevel = ui->comboBox_WideDynamicLevel->currentIndex();
                    req.image.wdraflevel = ui->slider_AntiflickerLevel->value();
                    //setCommonParam(channel, "aflevel", ui->slider_AntiflickerLevel->value());
                }
            }
        } else //hlc
        {
            req.image.hlcmode = ui->comboBox_HighLightCompensation->currentData().toInt();
            if (req.image.hlcmode != 0) {
                req.image.regiontype = 0;
                req.image.wdrenable = 0;
                req.image.hlclevel = ui->slider_HlcLevel->value();
            }
        }
    } else if (req.image.imageschemode == 1) //day/night mode
    {
        req.image.regiontype = ui->comboBox_BlcRegion->currentIndex();
        if (req.image.regiontype == 0) //off
        {
            snprintf(req.image.exposeregion, sizeof(req.image.exposeregion), "%s", "0:0:0:0");
        } else if (req.image.regiontype == 1) //customize
        {
            snprintf(req.image.exposeregion, sizeof(req.image.exposeregion), "%s", exposeregion);
            req.image.exclude = ui->comboBox_RegionType->currentIndex();
        } else //centre
        {
            snprintf(req.image.exposeregion, sizeof(req.image.exposeregion), "%s", "0:0:0:0");
        }
        req.image.wdrlevel = ui->comboBox_WideDynamicLevel->currentIndex();
        req.image.wdraflevel = ui->slider_AntiflickerLevel->value();
        //setCommonParam(channel, "aflevel", ui->slider_AntiflickerLevel->value());
        req.image.hlcmode = ui->comboBox_HighLightCompensation->currentData().toInt();
        req.image.hlclevel = ui->slider_HlcLevel->value();
        //setCommonParam(channel, "hlclevel", ui->slider_HlcLevel->value());
        req.image.hlcday = ui->comboBox_DayEnhancementMode->currentIndex();
        req.image.hlcnight = ui->comboBox_NightEnhancementMode->currentIndex();

    } else //schedule mode
    {
        req.image.regiontype = ui->comboBox_BlcRegion->currentIndex();
        if (req.image.regiontype == 0) //off
        {
            snprintf(req.image.exposeregion, sizeof(req.image.exposeregion), "%s", "0:0:0:0");
        } else if (req.image.regiontype == 1) //customize
        {
            snprintf(req.image.exposeregion, sizeof(req.image.exposeregion), "%s", exposeregion);
            req.image.exclude = ui->comboBox_RegionType->currentIndex();
        } else //centre
        {
            snprintf(req.image.exposeregion, sizeof(req.image.exposeregion), "%s", "0:0:0:0");
        }
        req.image.wdrlevel = ui->comboBox_WideDynamicLevel->currentIndex();
        req.image.wdraflevel = ui->slider_AntiflickerLevel->value();
        //setCommonParam(channel, "aflevel", ui->slider_AntiflickerLevel->value());
        req.image.hlcmode = ui->comboBox_HighLightCompensation->currentData().toInt();
        req.image.hlclevel = ui->slider_HlcLevel->value();
        //setCommonParam(channel, "hlclevel", ui->slider_HlcLevel->value());
    }
    if (ui->comboBox_lensType->isVisible()) {
        req.image.lensType = ui->comboBox_lensType->currentData().toInt();
    }

    qDebug() << "isNightMode: " << isNightMode;
    qDebug() << "wdrenable: " << req.image.wdrenable;
    qDebug() << "regiontype: " << req.image.regiontype;
    qDebug() << "hlcmode: " << req.image.hlcmode;
    qDebug() << "region" << req.image.exposeregion;
    if (isNightMode && (req.image.wdrenable || req.image.regiontype || req.image.hlcmode)) {
        ShowMessageBox(GET_TEXT("IMAGE/37417", "Please turn off LPR Night Mode in ANPR settings interface first."));
        return;
    }

    sendMessage(REQUEST_FLAG_SET_IPCIMAGE_ENHANCEMENT, &req, sizeof(struct req_image_enhancement));

    //schedule white Balance
    if (m_whiteBalanceSche) {
        struct white_balance_schedule whiteBalanceData;
        memset(&whiteBalanceData, 0, sizeof(struct white_balance_schedule));
        whiteBalanceData.chanid = channel;
        m_whiteBalanceSche->getSchedule(whiteBalanceData.schedule_day);
        sendMessageOnly(REQUEST_FLAG_SET_WHITE_BALANCE_SCHE, &whiteBalanceData, sizeof(struct white_balance_schedule));
    }
    //schedule exposure
    if (m_exposureSche) {
        struct exposure_schedule exposureData;
        memset(&exposureData, 0, sizeof(struct exposure_schedule));
        exposureData.chanid = channel;
        m_exposureSche->getSchedule(exposureData.schedule_day);
        sendMessageOnly(REQUEST_FLAG_SET_EXPOSURE_SCHE, &exposureData, sizeof(struct exposure_schedule));
    }

    //schedule blc/wdr/hlc
    if (m_BlcWdrHlcSche) {
        struct bwh_schedule bwhData;
        memset(&bwhData, 0, sizeof(struct bwh_schedule));
        bwhData.chanid = channel;
        m_BlcWdrHlcSche->getSchedule(bwhData.schedule_day);
        sendMessageOnly(REQUEST_FLAG_SET_BWH_SCHE, &bwhData, sizeof(struct bwh_schedule));
    }
}

void ImagePageEnhancement::copyEnhancement()
{
    int chnid = m_copyChannelList.takeFirst();
    Q_UNUSED(chnid)
}

void ImagePageEnhancement::on_pushButton_back_clicked()
{
    back();
}

void ImagePageEnhancement::on_pushButton_apply_clicked()
{
    //MsWaitting::showGlobalWait();
    if (ui->comboBox_ExposureMode->currentIndex() == 1 && ui->comboBox_WideDynamicRange->currentIndex() != 0) {
        QString questionText;
        if (wdrIsOn) {
            questionText = GET_TEXT("IMAGE/37245", "WDR will be disabled if Exposure Mode is Manual Mode, continue?");
        } else {
            questionText = GET_TEXT("IMAGE/37244", "Exposure Mode will be set to auto mode if WDR is enabled, continue?");
        }
        const int result = MessageBox::question(this, questionText);
        if (result == MessageBox::Yes) {
            if (wdrIsOn) {
                ui->comboBox_WideDynamicRange->setCurrentIndexFromData(0);
                on_comboBox_WideDynamicRange_activated(0);
            } else {
                ui->comboBox_ExposureMode->setCurrentIndexFromData(0);
                on_comboBox_ExposureMode_activated(0);
            }
        } else {
            //MsWaitting::closeGlobalWait();
            return;
        }
    }

    if (m_whiteBalanceSche) {
        m_whiteBalanceSche->apply();
    }
    if (m_exposureSche) {
        m_exposureSche->apply();
    }
    if (m_BlcWdrHlcSche) {
        m_BlcWdrHlcSche->apply();
    }
    if (m_manualModeSetting) {
        m_manualModeSetting->apply();
    }

    if (m_copyChannelList.isEmpty()) {
        m_copyChannelList.append(m_channel);
    }
    do {
        copyEnhancement();
        gEventLoopExec();
    } while (!m_copyChannelList.isEmpty());

    initializeData(m_channel);
}

void ImagePageEnhancement::on_pushButton_copy_clicked()
{
    m_copyChannelList.clear();

    ChannelCopyDialog copy(this);
    copy.setCurrentChannel(m_channel);
    int result = copy.exec();
    if (result == QDialog::Accepted) {
        m_copyChannelList = copy.checkedList();
        QTimer::singleShot(0, this, SLOT(on_pushButton_apply_clicked()));
    }
}

void ImagePageEnhancement::onChangeDefog()
{
    setCommonParam(m_channel, "defoglevel", ui->slider_AntifogIntensity->value());
}

void ImagePageEnhancement::changeRedGain()
{
    setCommonParam(m_channel, "redgain", ui->slider_RedGainLevel->value());
}

void ImagePageEnhancement::changeBlueGain()
{
    setCommonParam(m_channel, "bluegain", ui->slider_BlueGainLevel->value());
}

void ImagePageEnhancement::changeHlcLevel()
{
    if (m_info.hlcmode) {
        setCommonParam(m_channel, "hlclevel", ui->slider_HlcLevel->value());
    }
}

void ImagePageEnhancement::settingFinish()
{
    if (m_manualModeSetting && m_manualModeSetting->channel() == m_channel) {
        m_exposureSche->setManualMode(m_manualModeSetting->getManualModeList());
        m_exposureSche->setExposureShce(m_manualModeSetting->exposureShce());
    }
}

void ImagePageEnhancement::scheduleAccept()
{
    if (m_exposureSche && m_exposureSche->getChannel() == m_channel) {
        m_manualModeSetting->setExposureShce(m_exposureSche->getExposureShce());
    }
}
