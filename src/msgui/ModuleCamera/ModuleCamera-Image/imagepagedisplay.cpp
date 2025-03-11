#include "imagepagedisplay.h"
#include "ui_imagepagedisplay.h"
#include "FaceData.h"
#include "MessageBox.h"
#include "MsCameraVersion.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsSdkVersion.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "channelcopydialog.h"
#include <QListView>
#include <QScopedValueRollback>
#include <QtCore/qmath.h>
#include <cmath>
#include "qjson/include/parser.h"


ImagePageDisplay::ImagePageDisplay(QWidget *parent)
    : AbstractImagePage(parent)
    , ui(new Ui::ImagePageDisplay)
{
    ui->setupUi(this);

    ui->comboBox_PowerLineFrequency->clear();
    ui->comboBox_PowerLineFrequency->addItem("60Hz");
    ui->comboBox_PowerLineFrequency->addItem("50Hz");

    m_oneIRModel = 0;
    ui->comboBox_SmartIrMode->clear();
    ui->comboBox_SmartIrMode->addItem(GET_TEXT("IMAGE/37104", "Auto Mode"), 0);
    ui->comboBox_SmartIrMode->addItem(GET_TEXT("COMMON/1015", "Customize"), 1);

    ui->comboBox_OutdoorIndoorMode->clear();
    ui->comboBox_OutdoorIndoorMode->addItem(GET_TEXT("IMAGE/37115", "Outdoor"));
    ui->comboBox_OutdoorIndoorMode->addItem(GET_TEXT("IMAGE/37116", "Indoor"));

    ui->comboBox_LensDistortCorrect->clear();
    ui->comboBox_LensDistortCorrect->addItem(GET_TEXT("IMAGE/37316", "Off"), 0);
    ui->comboBox_LensDistortCorrect->addItem(GET_TEXT("IMAGE/37317", "On"), 1);

    ui->comboBox_DayNightMode->clear();
    ui->comboBox_DayNightMode->addItem(GET_TEXT("IMAGE/37103", "Night Mode"), 0);
    ui->comboBox_DayNightMode->addItem(GET_TEXT("IMAGE/37102", "Day Mode"), 1);
    ui->comboBox_DayNightMode->addItem(GET_TEXT("IMAGE/37104", "Auto Mode"), 2);
    ui->comboBox_DayNightMode->addItem(GET_TEXT("IMAGE/37105", "Customize"), 3);

    ui->comboBox_CorridorMode->clear();
    ui->comboBox_CorridorMode->addItem(GET_TEXT("IMAGE/37316", "Off"));
    ui->comboBox_CorridorMode->addItem(GET_TEXT("IMAGE/37118", "Clockwise 90°"));
    ui->comboBox_CorridorMode->addItem(GET_TEXT("IMAGE/37119", "Anticlockwise 90°"));

    ui->comboBox_ImageRotation->clear();
    ui->comboBox_ImageRotation->addItem(GET_TEXT("IMAGE/37316", "Off"));
    ui->comboBox_ImageRotation->addItem(GET_TEXT("IMAGE/37122", "Rotating 180°"));
    ui->comboBox_ImageRotation->addItem(GET_TEXT("IMAGE/37123", "Flip Horizontal"));
    ui->comboBox_ImageRotation->addItem(GET_TEXT("IMAGE/37124", "Flip Vertical"));

    ui->comboBox_smokedDome->clear();
    ui->comboBox_smokedDome->addItem(GET_TEXT("IMAGE/37316", "Off"));
    ui->comboBox_smokedDome->addItem(GET_TEXT("IMAGE/37317", "On"));

    ui->comboBox_orientation->clear();
    ui->comboBox_orientation->addItem(GET_TEXT("DISKMANAGE/72099", "Normal"), 0);
    ui->comboBox_orientation->addItem(GET_TEXT("IMAGE/37358", "FlipHorizontal"), 1);
    ui->comboBox_orientation->addItem(GET_TEXT("IMAGE/37359", "FlipVertical"), 2);
    ui->comboBox_orientation->addItem(GET_TEXT("IMAGE/37360", "Rotating90"), 4);
    ui->comboBox_orientation->addItem(GET_TEXT("IMAGE/37361", "Rotating180"), 3);
    ui->comboBox_orientation->addItem(GET_TEXT("IMAGE/37362", "Rotating270"), 7);

    ui->comboBox_localDisplay->clear();
    ui->comboBox_localDisplay->addItem(GET_TEXT("IMAGE/37316", "Off"), 0);
    ui->comboBox_localDisplay->addItem(GET_TEXT("IMAGE/37374", "NTSC"), 1);
    ui->comboBox_localDisplay->addItem(GET_TEXT("IMAGE/37375", "PAL"), 2);

    ui->comboBox_keepCorrectAspectRatio->clear();
    ui->comboBox_keepCorrectAspectRatio->addItem(GET_TEXT("IMAGE/37316", "Off"), 0);
    ui->comboBox_keepCorrectAspectRatio->addItem(GET_TEXT("IMAGE/37317", "On"), 1);

    ui->comboBoxWhiteLEDLightControl->clear();
    ui->comboBoxWhiteLEDLightControl->addItem(GET_TEXT("IMAGE/162004", "Auto Mode"), WhiteLEDAutoMode);
    ui->comboBoxWhiteLEDLightControl->addItem(GET_TEXT("IMAGE/162005", "Always On"), WhiteLEDAlwaysOn);
    ui->comboBoxWhiteLEDLightControl->addItem(GET_TEXT("IMAGE/162006", "Off"), WhiteLEDOFF);
    ui->comboBoxWhiteLEDLightControl->addItem(GET_TEXT("IMAGE/162007", "Customize"), WhiteLEDCustomize);

    ui->comboBoxBrightnessControl->clear();
    ui->comboBoxBrightnessControl->addItem(GET_TEXT("IMAGE/162007", "Customize"), 1);
    ui->comboBoxBrightnessControl->addItem(GET_TEXT("IMAGE/162004", "Auto Mode"), 0);

    ui->slider_fovAdjust->setRange(1, 10);
    ui->slider_DayToNightSensitivity->setRange(1, 10);
    ui->slider_NightToDaySensitivity->setRange(1, 10);

    ui->slider_DayToNightValue->setRange(1, 100);
    ui->slider_NightToDayValue->setRange(1, 100);
    ui->horizontalSliderWhiteLEDSensitivity->setRange(1, 5);
    ui->horizontalSliderBrightness->setRange(1, 100);

    QRegExp regx("[0-9]+$");
    QValidator *validator = new QRegExpValidator(regx, ui->lineEditDelayTime);
    ui->lineEditDelayTime->setValidator(validator);
    ui->lineEditDelayTime->setCheckMode(MyLineEdit::RangeCheck, 1, 60);

    connect(ui->comboBox_SmartIrMode, SIGNAL(currentIndexChanged(int)), this, SLOT(onModeIRChange(int)));
    connect(ui->comboBox_DayNightMode, SIGNAL(currentIndexChanged(int)), this, SLOT(onDayNightModeChange(int)));
    connect(ui->slider_DayToNightValue, SIGNAL(valueChanged(int)), this, SLOT(onSliderDayToNightValueChange()));
    connect(ui->slider_NightToDayValue, SIGNAL(valueChanged(int)), this, SLOT(onSliderNightToDayValueChange()));
    connect(ui->slider_DayToNightSensitivity, SIGNAL(valueChanged(int)), this, SLOT(onDSensitivityChanged(int)));
    connect(ui->slider_NightToDaySensitivity, SIGNAL(valueChanged(int)), this, SLOT(onNSensitivityChanged(int)));
    connect(ui->slider_NearViewIrValue, SIGNAL(valueEdited(int)), this, SLOT(onChangeNearValue()));
    connect(ui->slider_FarViewIrValue, SIGNAL(valueEdited(int)), this, SLOT(onChangeFarValue()));
    connect(this, SIGNAL(needDNSensitivityChanged()), this, SLOT(onNeedDNSensitivityChanged()));
    connect(this, SIGNAL(needZoomLimitChanged()), this, SLOT(onNeedZoomLimitChanged()));
    connect(ui->timeEditWhiteLEDStartTime, SIGNAL(timeChanged(QTime)), this, SLOT(onWhiteLEDLightTimeChange(QTime)));
    connect(ui->timeEditWhiteLEDEndTime, SIGNAL(timeChanged(QTime)), this, SLOT(onWhiteLEDLightTimeChange(QTime)));
    connect(ui->timeEdit_StartTimeOfNight, SIGNAL(timeChanged(QTime)), this, SLOT(onDayNightTimeChange(QTime)));
    connect(ui->timeEdit_EndTimeOfNight, SIGNAL(timeChanged(QTime)), this, SLOT(onDayNightTimeChange(QTime)));

    onLanguageChanged();
}

ImagePageDisplay::~ImagePageDisplay()
{
    delete ui;
}

void ImagePageDisplay::getCommonParam(const QString &param)
{
    struct req_http_common_param common_param;
    memset(&common_param, 0, sizeof(struct req_http_common_param));

    common_param.chnid = m_channel;
    if (param.compare("getadcvalue") == 0) {
        snprintf(common_param.info.httpname, sizeof(common_param.info.httpname), "%s", "camera_display.html");
        common_param.info.noparatest = 1;
    } else if (param.compare("getsmartirvalue") == 0) {
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

void ImagePageDisplay::ON_RESPONSE_FLAG_GET_SINGLE_IPCTYPE(MessageReceive *message)
{
    CAM_MODEL_INFO *cam_model_info = (CAM_MODEL_INFO *)message->data;
    if (!cam_model_info) {
        m_eventLoop.exit(-1);
        return;
    }
    m_model.setModel(cam_model_info->model);
    m_eventLoop.exit();
}

void ImagePageDisplay::initializeData(int channel)
{
    msprintf("gsjt ImagePageDisplay initializeData");
    m_channel = channel;

    m_isInitDisplay = true;
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

    //MsWaitting::showGlobalWait();
    sendMessage(REQUEST_FLAG_GET_SINGLE_IPCTYPE, &m_channel, sizeof(int));
    //m_eventLoop.exec();
    sendMessage(REQUEST_FLAG_GET_IPC_CAP_IMAGE, &m_channel, sizeof(int));
    //m_eventLoop.exec();
    sendMessage(REQUEST_FLAG_GET_IPCIMAGE_DISPLAY, &m_channel, sizeof(int));
    //m_eventLoop.exec();
    sendMessage(REQUEST_FLAG_GET_IPC_MODEL_TYPE, &m_channel, sizeof(int));
    //m_eventLoop.exec();
    sendMessage(REQUEST_FLAG_GET_LPR_SETTINGS, &m_channel, sizeof(int));
    //m_eventLoop.exec();
    getCommonParam("getadcvalue");
    //m_eventLoop.exec();
    //MsWaitting::closeGlobalWait();
}

void ImagePageDisplay::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_SINGLE_IPCTYPE:
        ON_RESPONSE_FLAG_GET_SINGLE_IPCTYPE(message);
        break;
    case RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY:
        ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(message);
        break;
    case RESPONSE_FLAG_GET_IPC_MODEL_TYPE:
        ON_RESPONSE_FLAG_GET_IPC_MODEL_TYPE(message);
        break;
    case RESPONSE_FLAG_GET_IPC_COMMON_PARAM:
        ON_RESPONSE_FLAG_GET_IPC_COMMON_PARAM(message);
        break;
    case RESPONSE_FLAG_GET_IPC_LPR_NIGHT_MODE:
        ON_RESPONSE_FLAG_GET_IPC_LPR_NIGHT_MODE(message);
        break;
    case RESPONSE_FLAG_GET_LPR_SETTINGS:
        ON_RESPONSE_FLAG_GET_LPR_SETTINGS(message);
        break;
    case RESPONSE_FLAG_GET_IPC_CAP_IMAGE:
        ON_RESPONSE_FLAG_GET_IPC_CAP_IMAGE(message);
        break;
    default:
        break;
    }
}

int ImagePageDisplay::defaultNearViewIRValue(const MsCameraModel &model)
{
    int value = 50;
    if (model.isMatch(QRegExp("MS-C..75-EPB"))) {
        value = 100;
    } else if (model.isMatch(QRegExp("MS-C9674-PB"))) {
        value = 80;
    } else if (model.isMatch(QRegExp("MS-C5374-PB"))) {
        value = 80;
    }
    return value;
}

int ImagePageDisplay::defaultMiddleViewIRValue(const MsCameraModel &model)
{
    Q_UNUSED(model)

    int value = 50;
    return value;
}

int ImagePageDisplay::defaultFarViewIRValue(const MsCameraModel &model)
{
    int value = 50;
    if (model.isMatch(QRegExp("MS-C9674-PB"))) {
        value = 60;
    } else if (model.isMatch(QRegExp("MS-C5374-PB"))) {
        value = 60;
    } else if (model.isMatch(QRegExp("MS-C..75-F.*"))) {
        value = 100;
    }
    return value;
}

int ImagePageDisplay::defaultIRLEDLevel(const MsCameraModel &model)
{
    Q_UNUSED(model)

    int value = 100;
    return value;
}

void ImagePageDisplay::ON_RESPONSE_FLAG_GET_IPC_LPR_NIGHT_MODE(MessageReceive *message)
{
    resp_lpr_night_mode *data = reinterpret_cast<resp_lpr_night_mode *>(message->data);
    if (!data) {
        qWarning() << __FUNCTION__ << ": data is null";
        m_eventLoop.exit(-1);
        qDebug() << "@@@@@ failed isNightMode:" << isNightMode;
        return;
    }
    isNightMode = data->nightMode;
    qDebug() << "######ON_RESPONSE_FLAG_GET_IPC_LPR_NIGHT_MODE isNightMode:" << isNightMode;
    m_eventLoop.exit();
}

void ImagePageDisplay::ON_RESPONSE_FLAG_GET_LPR_SETTINGS(MessageReceive *message)
{
    struct ms_lpr_settings *lpr_settings = (struct ms_lpr_settings *)message->data;
    if (!lpr_settings) {
        qWarning() << QString("ImagePageDisplay::ON_RESPONSE_FLAG_GET_LPR_SETTINGS, data is null. isNightMode:[%1]").arg(isNightMode);
        m_eventLoop.exit();
        return;
    }
    isNightMode = lpr_settings->in_night_mode;
    m_nightModeEnable = lpr_settings->night_mode_info.enable;
    m_nightModeEffectiveMode = lpr_settings->night_mode_info.effective_mode;
    qDebug() << "###### ON_RESPONSE_FLAG_GET_LPR_SETTINGS  isNightMode:" << isNightMode;
    m_eventLoop.exit();
}

void ImagePageDisplay::ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(MessageReceive *message)
{
    resp_image_display *data = (resp_image_display *)message->data;
    if (!data) {
        qWarning() << "ImagePageDisplay::ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY, data is null.";
        m_eventLoop.exit(-1);
        return;
    }
    memcpy(&m_info, &(data->image), sizeof(image_display));

    qMsDebug() << "\nchipninterface:" << QString("%1").arg(QString(m_info.chipninterface).toInt(), 0, 2)
               << "\nsmartIRType:" << m_info.smartIRType
               << "\nlenCorrectType:" << m_info.lenCorrectType;

    initDisplay();

    m_eventLoop.exit();
}

void ImagePageDisplay::ON_RESPONSE_FLAG_GET_IPC_COMMON_PARAM(MessageReceive *message)
{
    struct resp_http_common_param *param = (struct resp_http_common_param *)message->data;
    if (!param) {
        qWarning() << "ImagePageDisplay::ON_RESPONSE_FLAG_GET_IPC_COMMON_PARAM, data is null.";
        m_eventLoop.exit(-1);
        return;
    }
    if (QString(param->info.key) == "getadcvalue") {
        int value = QString(param->info.value).toInt();
        ui->lineEdit_IrLightSensorValue->setText(QString("%1").arg((value / 1.0) > 100 ? 100 : (value / 1.0)));
    }
    if (QString(param->info.key) == "getsmartirvalue") {
        QString buf;
        QStringList tmp = QString(param->info.value).split(":");
        if (tmp.count() < 3) {
            m_eventLoop.exit(-1);
            return;
        }
        buf = QString(GET_TEXT("IMAGE/162015", "Near") + tmp[0]);
        ui->lineEdit_IrStrengthValue_Near->setText(buf);
        buf = QString(GET_TEXT("IMAGE/162016", "Far") + tmp[2]);
        ui->lineEdit_IrStrengthValue_Far->setText(buf);
        buf = QString(GET_TEXT("IMAGE/162017", "Middle") + tmp[1]);
        ui->lineEdit_IrStrengthValue_Middle->setText(buf);
    }
    m_eventLoop.exit();
}

void ImagePageDisplay::ON_RESPONSE_FLAG_SET_IPCIMAGE_DISPLAY(MessageReceive *message)
{
    Q_UNUSED(message)
    if (m_copyChannelList.isEmpty()) {
        //MsWaitting::closeGlobalWait();
    }
}

void ImagePageDisplay::ON_RESPONSE_FLAG_GET_IPC_MODEL_TYPE(MessageReceive *message)
{
    if (!message->data) {
        qWarning() << "ImagePageDisplay::ON_RESPONSE_FLAG_GET_IPC_MODEL_TYPE, data is null.";
        m_eventLoop.exit(-1);
        return;
    }

    int type = *((int *)message->data);
    m_currentChnType = type;

    //IPC需为73版本或者以上才支持该功能，低于73版本的IPC不显示该项
    //除去鱼眼+全景筒，其他所有机型都支持该功能
    char version[50];
    //get_channel_version_name(m_channel, version, sizeof(version));
    MsCameraVersion cameraVersion(version);

    setNeedZoomLimit(false);
    setNeedDNSensitivity(false);
    if (cameraVersion >= MsCameraVersion(7, 74)) {
        if (m_model.opticalZoom() > 20) {
            setNeedZoomLimit(true);
        }
        //sensitivity拉条显示
        if (qMsNvr->isSigma(m_channel) && cameraVersion >= MsCameraVersion(7, 77)) {
            setNeedDNSensitivity(m_info.system_image_slider_type == 2);
        } else {
            if (isNewFixProBullet() || isMiniProBulletFull(m_info.productmodel)) {
                setNeedDNSensitivity(true);
            }
        }
    }
    if (!qMsNvr->isFisheye(m_channel) && !m_model.model().contains("5365") && cameraVersion > MsCameraVersion(7, 72) && !qMsNvr->isNT(m_channel) && !qMsNvr->isSigma(m_channel)) {
        ui->label_keepCorrectAspectRatio->setVisible(true);
        ui->comboBox_keepCorrectAspectRatio->setVisible(true);
    } else {
        ui->label_keepCorrectAspectRatio->setVisible(false);
        ui->comboBox_keepCorrectAspectRatio->setVisible(false);
    }

    updateCorridor(type);
    m_eventLoop.exit();
}

void ImagePageDisplay::ON_RESPONSE_FLAG_GET_IPC_CAP_IMAGE(MessageReceive *message)
{
    ui->comboBox_DayNightRefocus->clear();
    if (!message->data) {
        ui->comboBox_DayNightRefocus->addItem(GET_TEXT("IMAGE/37316", "Off"), 0);
        ui->comboBox_DayNightRefocus->addItem(GET_TEXT("IMAGE/37317", "On"), 1);
        m_eventLoop.exit(-1);
        return;
    }
    QByteArray array = QByteArray(static_cast<char *>(message->data), message->header.size);
    QJson::Parser parser;
    bool ok;
    auto result = parser.parse(array, &ok);
    if (ok) {
        auto rootMap = result.toMap();
        auto rootDnSwitchRefocus = rootMap.value("dnSwitchRefocus").toMap();
        QMap<int, QString> dnSwitchRefocusMap;
        //直接解析Map会按String排序导致乱序，重新反过来导入Map让排序正确
        for(auto i = rootDnSwitchRefocus.begin(); i != rootDnSwitchRefocus.end(); i++) {
            dnSwitchRefocusMap.insert(i.value().toInt(), i.key());
        }
        for (auto i = dnSwitchRefocusMap.begin(); i!=dnSwitchRefocusMap.end(); i++) {
            if (i.value().compare("Off") == 0) {
                ui->comboBox_DayNightRefocus->addItem(GET_TEXT("IMAGE/37316", "Off"), i.key());
            } else if (i.value().compare("On") == 0) {
                ui->comboBox_DayNightRefocus->addItem(GET_TEXT("IMAGE/37317", "On"), i.key());
            } else if (i.value().compare("Dynamic") == 0) {
                ui->comboBox_DayNightRefocus->addItem(GET_TEXT("IMAGE/170001", "Dynamic"), i.key());
            } else if (i.value().compare("Static") == 0) {
                ui->comboBox_DayNightRefocus->addItem(GET_TEXT("IMAGE/170000", "Static"), i.key());
            } else {
                ui->comboBox_DayNightRefocus->addItem(i.value(), i.key());
            }
        }
    }
    m_eventLoop.exit();
}

bool ImagePageDisplay::checkCorridor(int type)
{
    bool supportCorridor = false;
    char version[50];
    //get_channel_version_name(m_channel, version, sizeof(version));
    MsCameraVersion cameraVersion(version);
    if (type == AMBA_IPC) {
        if (cameraVersion >= MsCameraVersion(7, 65) && !qMsNvr->isFisheye(m_channel)) {
            supportCorridor = true;
        }
    } else {
        if (cameraVersion >= MsCameraVersion(7, 60)) {
            supportCorridor = true;
        }
    }
    return supportCorridor;
}

void ImagePageDisplay::updateCorridor(int type)
{
    m_supportCorridor = checkCorridor(type);
    if (m_supportCorridor) {
        ui->label_orientation->hide();
        ui->comboBox_orientation->hide();
        ui->label_CorridorMode->show();
        ui->comboBox_CorridorMode->show();
        ui->label_ImageRotation->show();
        ui->comboBox_ImageRotation->show();
    } else {
        ui->label_orientation->show();
        ui->comboBox_orientation->show();
        ui->label_CorridorMode->hide();
        ui->comboBox_CorridorMode->hide();
        ui->label_ImageRotation->hide();
        ui->comboBox_ImageRotation->hide();
    }
}

void ImagePageDisplay::isOneIRModel()
{
    int chipninterface = atoi(m_info.chipninterface);
    int tmpSmart = whiteandIRPWMInterface(chipninterface);
    int tmpMode = 0;

    if (isLessOldPWMInterface(chipninterface)) {
        if (tmpSmart == PWM_TYPE_1IR_1W || tmpSmart == PWM_TYPE_1IR || tmpSmart == PWM_TYPE_1IR_2W) {
            tmpMode = 1;
        }
    }

    //MSHN-9066 QT-Image:Display，鱼眼机型，Smart IR Mode应去掉Auto Mode选项
    if (qMsNvr->isFisheye(m_channel) && m_model.getMaximumResolution() == RES_12M) {
        tmpMode = 1;
    }

    if (m_oneIRModel != tmpMode) {
        m_oneIRModel = tmpMode;
        ui->comboBox_SmartIrMode->clear();
        if (m_oneIRModel == 0) {
            ui->comboBox_SmartIrMode->addItem(GET_TEXT("IMAGE/37104", "Auto Mode"), 0);
            ui->comboBox_SmartIrMode->addItem(GET_TEXT("COMMON/1015", "Customize"), 1);
        } else {
            ui->comboBox_SmartIrMode->addItem(GET_TEXT("COMMON/1015", "Customize"), 1);
        }
    }
}

void ImagePageDisplay::initDisplay()
{
    ui->label_whiteLedLevel->hide();
    ui->widget_whiteLedLevel->setVisible(false);

    isOneIRModel();

    ui->comboBox_PowerLineFrequency->setCurrentIndex(m_info.exposurectrl);
    if (m_oneIRModel == 1) {
        ui->comboBox_SmartIrMode->setCurrentIndexFromData(1);
    } else {
        ui->comboBox_SmartIrMode->setCurrentIndexFromData(m_info.smartIRType);
    }
    ui->comboBox_OutdoorIndoorMode->setCurrentIndex(m_info.lighttype);
    ui->comboBox_DayNightMode->setCurrentIndex(m_info.colorkiller);
    ui->slider_DayToNightValue->setValue(m_info.ircutaddaytonight / 5.5 + 0.5);
    ui->slider_NightToDayValue->setValue(m_info.ircutadnighttoday / 5.5 + 0.5);
    ui->slider_DayToNightSensitivity->setValue(m_info.d2n_sensitivity);
    ui->slider_NightToDaySensitivity->setValue(m_info.n2d_sensitivity);
    ui->timeEdit_StartTimeOfNight->setTime(QTime(m_info.modestarthour, m_info.modestartminute));
    ui->timeEdit_EndTimeOfNight->setTime(QTime(m_info.modestophour, m_info.modestopminute));
    ui->comboBox_CorridorMode->setCurrentIndex(m_info.corridormode);
    ui->comboBox_ImageRotation->setCurrentIndex(m_info.imagerotation);
    ui->comboBox_orientation->setCurrentIndexFromData(m_info.mirctrl);
    ui->comboBox_DayNightRefocus->setCurrentIndexFromData(m_info.dnRefocus);
    ui->label_note->hide();
    ui->label_SensitivitySanityNote->hide();
    ui->slider_whiteLedLevel->setValue(m_info.smartwhitestrength);
    ui->comboBox_localDisplay->setCurrentIndex(m_info.localdisplay);
    ui->comboBox_keepCorrectAspectRatio->setCurrentIndex(m_info.aspect_ratio);
    ui->comboBox_ZoomLimit->setEnabled(!m_info.aspect_ratio);

    int chipninterface = atoi(m_info.chipninterface);
    //AF镜头 或变焦镜头7~22mm的机型支持。
    if (!isSupportDayNightSwitchRefocus()) {
        ui->label_DayNightRefocus->hide();
        ui->comboBox_DayNightRefocus->hide();
    } else {
        ui->label_DayNightRefocus->show();
        ui->comboBox_DayNightRefocus->show();
    }

    if (!isLessBncInterface(chipninterface)) {
        ui->label_localDisplay->show();
        ui->comboBox_localDisplay->show();
    } else {
        ui->label_localDisplay->hide();
        ui->comboBox_localDisplay->hide();
    }

    if (m_info.lenCorrectType == 0) {
        ui->comboBox_LensDistortCorrect->hide();
        ui->label_LensDistortCorrect->hide();
        ui->label_fovAdjust->hide();
        ui->widget_fovAdjust->setVisible(false);
    } else {
        ui->comboBox_LensDistortCorrect->show();
        ui->label_LensDistortCorrect->show();
        if (m_info.lencorrect == 1 && !m_info.isNtPlatform) {
            ui->label_fovAdjust->show();
            ui->widget_fovAdjust->setVisible(true);
        } else {
            ui->label_fovAdjust->hide();
            ui->widget_fovAdjust->setVisible(false);
        }
        ui->comboBox_LensDistortCorrect->setCurrentIndex(m_info.lencorrect);
        ui->slider_fovAdjust->setValue(m_info.fovadjust);
    }
    if (isMiniPtzDome(m_info.productmodel)
        || isProDome(m_info.productmodel)) {
        ui->comboBox_smokedDome->show();
        ui->label_smokedDome->show();
        ui->comboBox_smokedDome->setCurrentIndex(m_info.smokeddomecover);
    } else {
        ui->comboBox_smokedDome->hide();
        ui->label_smokedDome->hide();
        ui->comboBox_smokedDome->setCurrentIndex(0);
    }

    QString buf;
    buf = GET_TEXT("IMAGE/162015", "Near") + QString(":%1").arg(m_info.nearSmartirValue);
    ui->lineEdit_IrStrengthValue_Near->setText(buf);
    buf = GET_TEXT("IMAGE/162016", "Far") + QString(":%1").arg(m_info.farSmartirValue);
    ui->lineEdit_IrStrengthValue_Far->setText(buf);
    buf = GET_TEXT("IMAGE/162017", "Middle") + QString(":%1").arg(m_info.midSmartirValue);
    ui->lineEdit_IrStrengthValue_Middle->setText(buf);
    ui->slider_IRLedValue->setValue(m_info.farIRLevel);
    ui->slider_NearViewIrValue->setValue(m_info.nearIRLevel);
    ui->slider_FarViewIrValue->setValue(m_info.farIRLevel);
    ui->slider_MiddleViewIrValue->setValue(m_info.midIRLevel);

    onModeIRChange(ui->comboBox_SmartIrMode->currentIndex());
    onDayNightModeChange(ui->comboBox_DayNightMode->currentIndex());
    mayDisplayZoomLimitPanel(true, m_info.zoom_limit);

    onShowWhite();
    onCompatiblePhotosensitive();
    //全彩机型白灯功能
    fullColorWhiteLEDLightInit();
    m_isInitDisplay = false;
}

void ImagePageDisplay::onSliderDayToNightChanged(int value)
{
    Q_UNUSED(value);
    int day_value = ui->slider_DayToNightValue->value();
    int night_value = ui->slider_NightToDayValue->value();
    if (day_value + 10 > night_value) {
        if (day_value + 10 >= 100)
            ui->slider_NightToDayValue->setValue(100);
        else
            ui->slider_NightToDayValue->setValue(day_value + 10);
    }
}

void ImagePageDisplay::onSliderNightToDayChanged(int value)
{
    Q_UNUSED(value);
    int day_value = ui->slider_DayToNightValue->value();
    int night_value = ui->slider_NightToDayValue->value();

    if (day_value + 10 == night_value || night_value >= 100) {
        return;
    }
    if (night_value - 10 < day_value) {
        if (night_value - 10 < 0)
            ui->slider_DayToNightValue->setValue(0);
        else
            ui->slider_DayToNightValue->setValue(night_value - 10);
    }
}

void ImagePageDisplay::onDSensitivityChanged(int)
{
    sensitivitySanityCheck(true);
}

void ImagePageDisplay::onNSensitivityChanged(int)
{
    sensitivitySanityCheck(false);
}

void ImagePageDisplay::sensitivitySanityCheck(bool DPreferred)
{
    if (inAdjustSensitivity)
        return;

    bool showSanityNote = false;

    if (ui->slider_DayToNightSensitivity->value() + ui->slider_NightToDaySensitivity->value() <= dnSensitivityLimit) {
        ui->label_SensitivitySanityNote->setVisible(showSanityNote);
        return;
    }

    showSanityNote = true;
    ui->label_SensitivitySanityNote->setVisible(showSanityNote);
    m_daySensitivityPreferred = DPreferred;
    adjustSensitivity();
}

void ImagePageDisplay::adjustSensitivity()
{
    QScopedValueRollback<bool> cleanup(inAdjustSensitivity);
    inAdjustSensitivity = true;

    int dSensitivity = ui->slider_DayToNightSensitivity->value();
    int nSensitivity = ui->slider_NightToDaySensitivity->value();

    if (m_daySensitivityPreferred)
        nSensitivity = dnSensitivityLimit - dSensitivity;
    else
        dSensitivity = dnSensitivityLimit - nSensitivity;

    ui->slider_DayToNightSensitivity->setValue(dSensitivity);
    ui->slider_NightToDaySensitivity->setValue(nSensitivity);
    ui->label_SensitivitySanityNote->setVisible(true);
}

void ImagePageDisplay::mayDisplayDNSensitivityPanel(bool visible)
{
    ui->label_DayToNightSensitivity->setVisible(visible && needDNSensitivity());
    ui->widget_DayToNightSensitivity->setVisible(visible && needDNSensitivity());
    ui->label_NightToDaySensitivity->setVisible(visible && needDNSensitivity());
    ui->widget_NightToDaySensitivity->setVisible(visible && needDNSensitivity());
}

void ImagePageDisplay::mayDisplayZoomLimitPanel(bool visible, int zoomLimit)
{
    ui->comboBox_ZoomLimit->clear();
    int originalZoom = m_model.opticalZoom();
    ui->comboBox_ZoomLimit->beginEdit();
    for (int i = 0; i < 5; i++) {
        ui->comboBox_ZoomLimit->addItem(QString::number(originalZoom * qPow(2, i)), i);
    }
    ui->comboBox_ZoomLimit->endEdit();

    ui->label_ZoomLimit->setVisible(visible && needZoomLimit());
    ui->comboBox_ZoomLimit->setVisible(visible && needZoomLimit());

    int newIndex = 0;
    if (zoomLimit != -1) {
        for (int i = 0; i < ui->comboBox_ZoomLimit->count(); i++)
            if (ui->comboBox_ZoomLimit->itemText(i).toInt() == originalZoom * qPow(2, zoomLimit))
                newIndex = i;
    }
    ui->comboBox_ZoomLimit->setCurrentIndex(newIndex);
    if (ui->comboBox_ZoomLimit->isVisible()) {
        ui->comboBox_keepCorrectAspectRatio->setEnabled(!newIndex);
    } else {
        ui->comboBox_keepCorrectAspectRatio->setEnabled(true);
    }
}

bool ImagePageDisplay::needDNSensitivity()
{
    return m_needDNSensitivity;
}

void ImagePageDisplay::setNeedDNSensitivity(bool b)
{
    if (m_needDNSensitivity == b)
        return;
    m_needDNSensitivity = b;
    emit needDNSensitivityChanged();
}

bool ImagePageDisplay::needZoomLimit()
{
    return m_needZoomLimit;
}

void ImagePageDisplay::setNeedZoomLimit(bool b)
{
    if (m_needZoomLimit == b) {
        return;
    }
    m_needZoomLimit = b;
    emit needZoomLimitChanged();
}

bool ImagePageDisplay::isNewFixProBullet()
{
    int chipninterface = atoi(m_info.chipninterface);
    int isPtzBulletFree = chipninterface & (1 << 19);
    if ((m_model.isType63() && isPtzBulletFree)) {
        return true;
    }
    return false;
}

bool ImagePageDisplay::isAFLens3X()
{
    struct ipc_system_info system_info;
    memset(&system_info, 0, sizeof(system_info));
    //get_ipc_system_info(m_channel, &system_info);
    if (system_info.chipinfo[16] == '6') {
        return true;
    }
    return false;
}

bool ImagePageDisplay::isSpeedLens36Or42()
{
    struct ipc_system_info system_info;
    memset(&system_info, 0, sizeof(system_info));
    //get_ipc_system_info(m_channel, &system_info);
    if (system_info.chipinfo[16] == '7' || system_info.chipinfo[16] == '8') {
        return true;
    }
    return false;
}

bool ImagePageDisplay::isHisiIPC()
{
    return false;
}

bool ImagePageDisplay::isPoe(char *productmodel)
{
    QString model = QString("%1").arg(productmodel);

    if (model.contains('P')) {
        return true;
    }
    return false;
}

bool ImagePageDisplay::isSupportDayNightSwitchRefocus()
{
    //AF镜头 或变焦镜头7~22mm的机型支持。
    //加密芯片第17位为S\W\Y\Z\a\f\g\5\l\J
    QList<char> list;
    list << 'S' << 'W' << 'Y' << 'Z' << 'a' << 'f' << 'g' << '5' << 'l' << 'J' << 'V';

    struct ipc_system_info system_info;
    memset(&system_info, 0, sizeof(system_info));
    //get_ipc_system_info(m_channel, &system_info);
    if (list.contains(system_info.chipinfo[16])) {
        return true;
    }
    return false;
}

void ImagePageDisplay::fullColorWhiteLEDLightInit()
{
    QListView *view = qobject_cast<QListView *>(ui->comboBox_DayNightMode->view());
    if (m_info.whiteled.support) {
        ui->comboBoxBrightnessControl->setCurrentIndexFromData(m_info.whiteled.brightnessCtrl);
        ui->comboBoxWhiteLEDLightControl->setCurrentIndexFromData(m_info.whiteled.lightCtrl);
        ui->horizontalSliderWhiteLEDSensitivity->setValue(m_info.whiteled.sensitivity);
        ui->lineEditDelayTime->setText(QString("%1").arg(m_info.whiteled.delay));
        ui->timeEditWhiteLEDStartTime->setTime(QTime().addSecs(m_info.whiteled.start));
        ui->timeEditWhiteLEDEndTime->setTime(QTime().addSecs(m_info.whiteled.stop));
        ui->horizontalSliderBrightness->setValue(m_info.whiteled.brightness);

        ui->labelWhiteLEDLightControl->show();
        ui->comboBoxWhiteLEDLightControl->show();
        if (m_info.whiteled.lightCtrl != 2) {
            ui->labelBrightnessControl->show();
            ui->comboBoxBrightnessControl->show();
        }

        view->setRowHidden(2, true);
    } else {
        view->setRowHidden(2, false);
    }
}

void ImagePageDisplay::onNeedDNSensitivityChanged()
{
    onDayNightModeChange(ui->comboBox_DayNightMode->currentIndex());
}

void ImagePageDisplay::onNeedZoomLimitChanged()
{
    mayDisplayZoomLimitPanel(true, m_info.zoom_limit);
}

void ImagePageDisplay::onDayNightModeChange(int index)
{
    qDebug() << "onDayNightModeChange   isNightMode:" << isNightMode << "  index:" << index;
    if (m_nightModeEnable && isNightMode == 1 && index != 0) {
        ShowMessageBox(GET_TEXT("IMAGE/37417", "Please turn off LPR Night Mode in ANPR settings interface first."));
        ui->comboBox_DayNightMode->setCurrentIndex(0);
        return;
    }
    if (m_info.colorkiller == 2 && index != 2 /*auto 改成非auto*/
        && isNightMode == 0 /*夜间模式生效*/
        && m_nightModeEffectiveMode == 0 && m_nightModeEnable && !m_isInitDisplay) /*anpr界面enable 且为auto*/
    {
        int result = MessageBox::question(this, GET_TEXT("IMAGE/37134", "The Effective Time of LPR Night Mode will be switched to Customize, continue?"));
        if (result == MessageBox::Cancel) {
            ui->comboBox_DayNightMode->setCurrentIndex(2);
            return;
        }
    }

    mayDisplayDNSensitivityPanel(false);
    ui->label_DayToNightValue->hide();
    ui->widget_DayToNightValue->setVisible(false);
    ui->label_NightToDayValue->hide();
    ui->widget_NightToDayValue->setVisible(false);
    ui->label_IrLightSensorValue->hide();
    ui->lineEdit_IrLightSensorValue->hide();
    ui->label_StartTimeOfNight->hide();
    ui->timeEdit_StartTimeOfNight->hide();
    ui->label_EndTimeOfNight->hide();
    ui->timeEdit_EndTimeOfNight->hide();

    if (index == 2) {
        mayDisplayDNSensitivityPanel(true);
        ui->label_DayToNightValue->show();
        ui->widget_DayToNightValue->setVisible(true);
        ui->label_NightToDayValue->show();
        ui->widget_NightToDayValue->setVisible(true);
        ui->label_IrLightSensorValue->show();
        ui->lineEdit_IrLightSensorValue->show();
    } else if (index == 3) {
        ui->label_StartTimeOfNight->show();
        ui->timeEdit_StartTimeOfNight->show();
        ui->label_EndTimeOfNight->show();
        ui->timeEdit_EndTimeOfNight->show();
    }

    onCompatiblePhotosensitive();
}

void ImagePageDisplay::onModeIRChange(int index)
{
    int mode = ui->comboBox_SmartIrMode->itemData(index).toInt();
    ui->label_note->hide();

    int chipninterface = atoi(m_info.chipninterface);
    m_smartIrMode = whiteandIRPWMInterface(chipninterface);
    ui->label_SmartIrMode->hide();
    ui->comboBox_SmartIrMode->hide();
    ui->label_IrStrengthValue->hide();
    ui->widgetIrStrengthValue->hide();

    ui->label_IRLedValue->hide();
    ui->widget_IRLedValue->setVisible(false);

    ui->label_NearViewIrValue->hide();
    ui->widget_NearViewIrValue->setVisible(false);

    ui->label_MiddleViewIrValue->hide();
    ui->widget_MiddleViewIrValue->setVisible(false);

    ui->label_FarViewIrValue->hide();
    ui->widget_FarViewIrValue->setVisible(false);

    if (m_smartIrMode != PWM_TYPE_NONE) {
        ui->label_SmartIrMode->show();
        ui->comboBox_SmartIrMode->show();
        if (m_oneIRModel == 0) {
            ui->label_IrStrengthValue->show();
            ui->widgetIrStrengthValue->show();
            ui->lineEdit_IrStrengthValue_Middle->hide();
        }

        if (mode) {
            ui->label_IrStrengthValue->hide();
            ui->widgetIrStrengthValue->hide();

            if (isLessOldPWMInterface(chipninterface)) {
                if (m_smartIrMode == PWM_TYPE_1IR_1W || m_smartIrMode == PWM_TYPE_1IR || m_smartIrMode == PWM_TYPE_1IR_2W) {
                    ui->label_IRLedValue->show();
                    ui->widget_IRLedValue->setVisible(true);
                } else if (m_smartIrMode == PWM_TYPE_2IR || m_smartIrMode == PWM_TYPE_2IR_1NW || m_smartIrMode == PWM_TYPE_2IR_1W) {
                    ui->label_NearViewIrValue->show();
                    ui->widget_NearViewIrValue->setVisible(true);

                    ui->label_FarViewIrValue->show();
                    ui->widget_FarViewIrValue->setVisible(true);

                    ui->label_IrStrengthValue->show();
                    ui->widgetIrStrengthValue->show();
                    ui->lineEdit_IrStrengthValue_Middle->hide();
                } else if (m_smartIrMode == PWM_TYPE_3IR) {
                    ui->label_NearViewIrValue->show();
                    ui->widget_NearViewIrValue->setVisible(true);

                    ui->label_MiddleViewIrValue->show();
                    ui->widget_MiddleViewIrValue->setVisible(true);

                    ui->label_FarViewIrValue->show();
                    ui->widget_FarViewIrValue->setVisible(true);

                    ui->label_SmartIrMode->show();
                    ui->comboBox_SmartIrMode->show();
                    ui->label_IrStrengthValue->show();
                    ui->widgetIrStrengthValue->show();
                    ui->lineEdit_IrStrengthValue_Middle->show();
                    onChangeNearValue();
                }
            } else {
                ui->label_NearViewIrValue->show();
                ui->widget_NearViewIrValue->setVisible(true);

                ui->label_FarViewIrValue->show();
                ui->widget_FarViewIrValue->setVisible(true);
            }
        } else {
            if (isLessOldPWMInterface(chipninterface)) {
                if (m_smartIrMode == PWM_TYPE_3IR) {
                    ui->lineEdit_IrStrengthValue_Middle->show();
                    onChangeNearValue();
                }
            }
        }
    }
}

void ImagePageDisplay::onLanguageChanged()
{
    ui->label_PowerLineFrequency->setText(GET_TEXT("IMAGE/37100", "Power Line Frequency"));
    ui->label_SmartIrMode->setText(GET_TEXT("IMAGE/37109", "Smart IR Mode"));
    ui->label_NearViewIrValue->setText(GET_TEXT("IMAGE/37110", "Near View IR Level"));
    ui->label_MiddleViewIrValue->setText(GET_TEXT("IMAGE/37126", "Middle View IR Level"));
    ui->label_FarViewIrValue->setText(GET_TEXT("IMAGE/37111", "Far View IR Level"));

    ui->label_IrStrengthValue->setText(GET_TEXT("IMAGE/37112", "IR Strength Value"));
    ui->label_OutdoorIndoorMode->setText(GET_TEXT("IMAGE/37114", "Outdoor/Indoor Mode"));
    ui->label_DayNightRefocus->setText(GET_TEXT("IMAGE/37129", "Day/Night Switch Refocus"));
    ui->label_LensDistortCorrect->setText(GET_TEXT("IMAGE/37113", "Lens Distort Correct"));

    ui->label_DayNightMode->setText(GET_TEXT("IMAGE/37101", "Day/Night Mode"));
    ui->label_DayToNightValue->setText(GET_TEXT("IMAGE/37106", "Day to Night Value"));
    ui->label_NightToDayValue->setText(GET_TEXT("IMAGE/37107", "Night to Day Value"));
    ui->label_DayToNightSensitivity->setText(GET_TEXT("IMAGE/37411", "Day to Night Sensitivity"));
    ui->label_NightToDaySensitivity->setText(GET_TEXT("IMAGE/37412", "Night to Day Sensitivity"));
    ui->label_SensitivitySanityNote->setText(GET_TEXT("IMAGE/37413", "Note: The sum of sensitivity should be within 15."));
    ui->label_IrLightSensorValue->setText(GET_TEXT("IMAGE/37108", "IR Light Sensor Value"));
    ui->label_StartTimeOfNight->setText(GET_TEXT("IMAGE/37127", "Start time of Night"));
    ui->label_EndTimeOfNight->setText(GET_TEXT("IMAGE/37128", "End time of Night"));
    ui->label_CorridorMode->setText(GET_TEXT("IMAGE/37117", "Corridor Mode"));
    ui->label_ImageRotation->setText(GET_TEXT("IMAGE/37120", "Image Rotation"));
    ui->label_fovAdjust->setText(GET_TEXT("IMAGE/37354", "FoV Adjustment"));
    ui->label_smokedDome->setText(GET_TEXT("IMAGE/37355", "Smoked Dome Cover"));
    ui->label_whiteLedLevel->setText(GET_TEXT("IMAGE/37356", "White LED Level"));
    ui->label_orientation->setText(GET_TEXT("IMAGE/37357", "Video Orientationl"));
    ui->label_keepCorrectAspectRatio->setText(GET_TEXT("IMAGE/37007", "Keep Correct Aspect Ratio"));

    ui->pushButton_NearViewIrValue->setText(GET_TEXT("COMMON/1057", "Reset"));
    ui->pushButton_MiddleViewIrValue->setText(GET_TEXT("COMMON/1057", "Reset"));
    ui->pushButton_FarViewIrValue->setText(GET_TEXT("COMMON/1057", "Reset"));
    ui->pushButton_IRLedValue->setText(GET_TEXT("COMMON/1057", "Reset"));
    ui->pushButton_whiteLedLevel->setText(GET_TEXT("COMMON/1057", "Reset"));
    ui->pushButton_fovAdjust->setText(GET_TEXT("COMMON/1057", "Reset"));
    ui->pushButton_DayToNightValue->setText(GET_TEXT("COMMON/1057", "Reset"));
    ui->pushButton_NightToDayValue->setText(GET_TEXT("COMMON/1057", "Reset"));
    ui->pushButton_DayToNightSensitivity->setText(GET_TEXT("COMMON/1057", "Reset"));
    ui->pushButton_NightToDaySensitivity->setText(GET_TEXT("COMMON/1057", "Reset"));
    ui->label_note->setText(GET_TEXT("IMAGE/37363", "Note: Improper setting! The option has adjusted to the proper one!"));

    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_copy->setText(GET_TEXT("COMMON/1005", "Copy"));

    ui->label_localDisplay->setText(GET_TEXT("IMAGE/37373", "Local Display Video"));
    ui->label_ZoomLimit->setText(GET_TEXT("IMAGE/37414", "Zoom Limit"));

    ui->labelWhiteLEDLightControl->setText(GET_TEXT("IMAGE/162008", "White LED Light Control"));
    ui->labelWhiteLEDSensitivity->setText(GET_TEXT("IMAGE/162009", "Sensitivity"));
    ui->labelDelayTime->setText(GET_TEXT("IMAGE/162010", "Delay Time"));
    ui->labelWhiteLEDStartTime->setText(GET_TEXT("IMAGE/162011", "Start Time"));
    ui->labelWhiteLEDEndTime->setText(GET_TEXT("IMAGE/162012", "End Time"));
    ui->labelBrightnessControl->setText(GET_TEXT("IMAGE/162013", "Brightness Control"));
    ui->labelBrightness->setText(GET_TEXT("IMAGE/162014", "Brightness"));
}

void ImagePageDisplay::clearSettings()
{
    ui->widgetMessage->hideMessage();
    //初始化UI
    ui->pushButton_test->hide();
    ui->comboBox_SmartIrMode->setCurrentIndex(0);
    ui->comboBox_DayNightMode->setCurrentIndex(0);
    ui->comboBox_PowerLineFrequency->setCurrentIndex(0);
    ui->comboBox_OutdoorIndoorMode->setCurrentIndex(0);
    ui->comboBox_CorridorMode->setCurrentIndex(0);
    ui->comboBox_ImageRotation->setCurrentIndex(0);
    ui->comboBox_DayNightRefocus->setCurrentIndex(0);
    onModeIRChange(0);
    onDayNightModeChange(0);
    ui->label_SmartIrMode->hide();
    ui->comboBox_SmartIrMode->hide();
    ui->label_IrStrengthValue->hide();
    ui->widgetIrStrengthValue->hide();
    ui->label_ZoomLimit->hide();
    ui->comboBox_ZoomLimit->hide();
    ui->label_note->hide();
    ui->label_SensitivitySanityNote->hide();
    ui->label_whiteLedLevel->hide();
    ui->widget_whiteLedLevel->setVisible(false);

    ui->label_localDisplay->hide();
    ui->comboBox_localDisplay->hide();
    ui->label_keepCorrectAspectRatio->hide();
    ui->comboBox_keepCorrectAspectRatio->hide();
    ui->comboBox_LensDistortCorrect->hide();
    ui->label_LensDistortCorrect->hide();
    ui->label_fovAdjust->hide();
    ui->widget_fovAdjust->setVisible(false);
    ui->comboBox_smokedDome->hide();
    ui->label_smokedDome->hide();

    ui->labelWhiteLEDLightControl->hide();
    ui->comboBoxWhiteLEDLightControl->hide();
    ui->labelWhiteLEDSensitivity->hide();
    ui->horizontalSliderWhiteLEDSensitivity->hide();
    ui->labelDelayTime->hide();
    ui->widgetDelayTime->hide();
    ui->labelBrightnessControl->hide();
    ui->comboBoxBrightnessControl->hide();
    ui->labelBrightness->hide();
    ui->horizontalSliderBrightness->hide();
    ui->labelWhiteLEDStartTime->hide();
    ui->timeEditWhiteLEDStartTime->hide();
    ui->labelWhiteLEDEndTime->hide();
    ui->timeEditWhiteLEDEndTime->hide();

    ui->label_OutdoorIndoorMode->show();
    ui->comboBox_OutdoorIndoorMode->show();
    ui->label_DayNightRefocus->show();
    ui->comboBox_DayNightRefocus->show();
    ui->label_orientation->show();
    ui->comboBox_orientation->show();
}

void ImagePageDisplay::setSettingsEnable(bool enable)
{
    ui->scrollArea->setEnabled(enable);

    ui->pushButton_copy->setEnabled(enable);
    ui->pushButton_apply->setEnabled(enable);
}

int ImagePageDisplay::whiteandIRPWMInterface(int m_chipninterface)
{
    if (m_chipninterface & m_PWM_1IR_1W)
        return PWM_TYPE_1IR_1W;
    else if (m_chipninterface & m_PWM_2IR)
        return PWM_TYPE_2IR;
    else if (m_chipninterface & m_PWM_2IR_1W)
        return PWM_TYPE_2IR_1W;
    else if (m_chipninterface & m_PWM_2IR_1NW)
        return PWM_TYPE_2IR_1NW;
    else if (m_chipninterface & m_PWM_1IR)
        return PWM_TYPE_1IR;
    else if (m_chipninterface & m_PWM_3IR)
        return PWM_TYPE_3IR;
    else if (m_chipninterface & m_PWM_1IR_2W)
        return PWM_TYPE_1IR_2W;
    else
        return PWM_TYPE_NONE;
}

bool ImagePageDisplay::isLessOldPWMInterface(int m_chipninterface)
{
    bool value;
    if ((m_chipninterface & m_pwm) != 0 || (m_chipninterface & m_pwm_2) != 0)
        value = false;
    else
        value = true;

    return value;
}

bool ImagePageDisplay::isSupDistanceMeasure(int m_chipninterface)
{
    if ((m_chipninterface & m_distance_measure) != 0)
        return true;

    return false;
}

bool ImagePageDisplay::isFixFocus(int m_chipninterface)
{
    if ((m_chipninterface & m_fix_len) != 0)
        return true;

    return false;
}

bool ImagePageDisplay::isLessBncInterface(int m_chipninterface)
{
    if ((m_chipninterface & m_bnc_len) != 0)
        return false;

    return true;
}

bool ImagePageDisplay::isProDome(char *productmodel)
{
    QString model = QString("%1").arg(productmodel);
    QString sMini = model.mid(6, 2);
    if (sMini == "72")
        return true;

    return false;
}

bool ImagePageDisplay::isSpeed(char *productmodel)
{
    QString model = QString("%1").arg(productmodel);
    int sMini = model.mid(6, 1).toInt();
    if (sMini == 4)
        return true;

    return false;
}

bool ImagePageDisplay::isMiniZoomDome(char *productmodel)
{
    QString model = QString("%1").arg(productmodel);
    QString sNewIRmini = model.mid(6, 2);
    if (sNewIRmini == "75")
        return true;

    return false;
}

bool ImagePageDisplay::isSmokedDomeCoverSensor(char *sensortype)
{
    QString sensor = QString("%1").arg(sensortype);
    if (sensor.contains("imx385") || sensor.contains("imx290")
        || sensor.contains("imx291") || sensor.contains("imx123"))
        return true;
    else
        return false;
}
bool ImagePageDisplay::isMiniPtzDome(char *productmodel)
{
    QString model = QString("%1").arg(productmodel);
    int sBullet1 = model.mid(6, 1).toInt();
    int sBullet2 = model.mid(7, 1).toInt();
    if (sBullet1 == 7 && sBullet2 == 1)
        return true;
    return false;
}

bool ImagePageDisplay::isMiniWithoutIR(char *productmodel)
{
    QString model = QString("%1").arg(productmodel);
    QString sMini = model.mid(6, 2);
    struct ipc_system_info system_info;
    memset(&system_info, 0, sizeof(system_info));
    //get_ipc_system_info(m_channel, &system_info);
    if (sMini == "63" && system_info.chipinfo[13] == '2') {
        return true;
    }
    return false;
}

bool ImagePageDisplay::isBox(char *productmodel)
{
    QString model = QString("%1").arg(productmodel);
    int tmp = model.mid(6, 1).toInt();
    if (tmp == 5)
        return true;
    return false;
}

bool ImagePageDisplay::isMiniProBulletFull(char *productmodel)
{
    QString model = QString("%1").arg(productmodel);
    int mini1 = model.mid(6, 1).toInt();
    int mini2 = model.mid(7, 1).toInt();
    if (mini1 == 6 && mini2 == 5) {
        return true;
    } else if (mini1 == 7 && mini2 == 6) {
        return true;
    }
    return false;
}

void ImagePageDisplay::onCompatiblePhotosensitive()
{
    MsCameraVersion cameraVersion = qMsNvr->cameraVersion(m_channel);
    bool isSigmaHideValue = qMsNvr->isSigma(m_channel) && cameraVersion >= MsCameraVersion(7, 77) && m_info.system_image_slider_type != 1;
    if (isMiniWithoutIR(m_info.productmodel) || isBox(m_info.productmodel) || isMiniProBulletFull(m_info.productmodel) || isSigmaHideValue) {
        ui->label_DayToNightValue->hide();
        ui->widget_DayToNightValue->setVisible(false);
        ui->label_NightToDayValue->hide();
        ui->widget_NightToDayValue->setVisible(false);
        ui->label_IrLightSensorValue->hide();
        ui->lineEdit_IrLightSensorValue->hide();
    }

    return;
}

void ImagePageDisplay::onShowWhite()
{
    int chipninterface = atoi(m_info.chipninterface);
    if (isSupDistanceMeasure(chipninterface) || m_smartIrMode == PWM_TYPE_1IR_1W
        || m_smartIrMode == PWM_TYPE_2IR_1W || m_smartIrMode == PWM_TYPE_1IR_2W) {
        ui->label_whiteLedLevel->show();
        ui->widget_whiteLedLevel->setVisible(true);
    }
}

int ImagePageDisplay::saveDisplayInfo()
{
    struct req_image_display data;
    memset(&data, 0, sizeof(struct req_image_display));
    data.chnid = m_copyChannelList.takeFirst();

    data.image.exposurectrl = ui->comboBox_PowerLineFrequency->currentIndex();
    data.image.smartIRType = ui->comboBox_SmartIrMode->currentData().toInt();
    data.image.dnRefocus = ui->comboBox_DayNightRefocus->currentIntData();
    if (ui->slider_whiteLedLevel->isVisible())
        data.image.smartwhitestrength = ui->slider_whiteLedLevel->value();

    if (ui->slider_IRLedValue->isVisible()) {
        data.image.nearIRLevel = ui->slider_NearViewIrValue->value();
        data.image.farIRLevel = ui->slider_IRLedValue->value();
    } else if (ui->slider_MiddleViewIrValue->isVisible()) {
        data.image.nearIRLevel = ui->slider_NearViewIrValue->value();
        data.image.midIRLevel = ui->slider_MiddleViewIrValue->value();
        data.image.farIRLevel = ui->slider_FarViewIrValue->value();
    } else {
        data.image.nearIRLevel = ui->slider_NearViewIrValue->value();
        data.image.farIRLevel = ui->slider_FarViewIrValue->value();
    }

    data.image.lighttype = ui->comboBox_OutdoorIndoorMode->currentIndex();
    data.image.colorkiller = ui->comboBox_DayNightMode->currentIndex();
    data.image.ircutaddaytonight = ui->slider_DayToNightValue->value() * 5.5 + 0.5;
    data.image.ircutadnighttoday = ui->slider_NightToDayValue->value() * 5.5 + 0.5;
    data.image.d2n_sensitivity = ui->slider_DayToNightSensitivity->value();
    data.image.n2d_sensitivity = ui->slider_NightToDaySensitivity->value();
    if (m_model.opticalZoom()) {
        data.image.zoom_limit = log2(ui->comboBox_ZoomLimit->currentText().toInt() / m_model.opticalZoom());
    }
    QTime nightStart = ui->timeEdit_StartTimeOfNight->time();
    QTime nightEnd = ui->timeEdit_EndTimeOfNight->time();
    data.image.modestarthour = nightStart.hour();
    data.image.modestartminute = nightStart.minute();
    data.image.modestophour = nightEnd.hour();
    data.image.modestopminute = nightEnd.minute();
    if (m_info.lenCorrectType) {
        data.image.lencorrect = ui->comboBox_LensDistortCorrect->currentIndex();
        data.image.fovadjust = ui->slider_fovAdjust->value();
    }
    data.image.smokeddomecover = ui->comboBox_smokedDome->currentIndex();
    int smokeddomecover_previous = m_info.smokeddomecover;
    //MSHN-9130 QT-Settings-camera-image-display:开关Smoked Dmoe Cover建议提示是否重启立即生效，同步IPC
    if (!m_isCopy) {
        qMsDebug() << QString("smokeddomecover, previous: %1, now: %2").arg(smokeddomecover_previous).arg(data.image.smokeddomecover);
        if (data.image.smokeddomecover != smokeddomecover_previous) {
            //MsWaitting::closeGlobalWait();
            ui->pushButton_copy->clearUnderMouse();
            ui->pushButton_apply->clearUnderMouse();
            if (data.image.smokeddomecover == 0) {
                //on->off
                int result = MessageBox::question(this, GET_TEXT("IMAGE/37132", "Please ensure that camera is equipped with Clear Dome Cover. Camera will reboot to make settings take effect, continue?"));
                if (result == MessageBox::Cancel) {
                    return -1;
                }
            } else if (data.image.smokeddomecover == 1) {
                //off->on
                int result = MessageBox::question(this, GET_TEXT("IMAGE/37133", "Please ensure that camera is equipped with Smoked Dome Cover. Camera will reboot to make settings take effect, continue?"));
                if (result == MessageBox::Cancel) {
                    return -1;
                }
            }
            //MsWaitting::showGlobalWait();
        }
    }

    data.image.corridormode = ui->comboBox_CorridorMode->currentIndex();
    data.image.imagerotation = ui->comboBox_ImageRotation->currentIndex();
    data.image.mirctrl = ui->comboBox_orientation->currentData().toInt();
    data.image.localdisplay = ui->comboBox_localDisplay->currentData().toInt();
    data.image.aspect_ratio = ui->comboBox_keepCorrectAspectRatio->currentData().toInt();

    if (m_info.whiteled.support) {
        if (!ui->lineEditDelayTime->checkValid()) {
            return -1;
        }
        data.image.whiteled.lightCtrl = static_cast<IPC_DISPLAY_WHITELED_CTRL_E>(ui->comboBoxWhiteLEDLightControl->currentIntData());
        data.image.whiteled.sensitivity = static_cast<int>(ui->horizontalSliderWhiteLEDSensitivity->value());
        data.image.whiteled.delay = ui->lineEditDelayTime->text().toInt();
        QTime time;
        time.addSecs(0);
        data.image.whiteled.start = time.secsTo(ui->timeEditWhiteLEDStartTime->time());
        data.image.whiteled.stop = time.secsTo(ui->timeEditWhiteLEDEndTime->time());
        data.image.whiteled.brightnessCtrl = ui->comboBoxBrightnessControl->currentIntData();
        data.image.whiteled.brightness = static_cast<int>(ui->horizontalSliderBrightness->value());
    }

    if (m_supportCorridor) {
        data.image.saveCorridor = 1;
    } else {
        data.image.saveCorridor = 0;
    }
    sendMessage(REQUEST_FLAG_SET_IPCIMAGE_DISPLAY, &data, sizeof(struct resp_image_display));

    return 0;
}

void ImagePageDisplay::on_pushButton_back_clicked()
{
    back();
}

void ImagePageDisplay::on_pushButton_apply_clicked()
{
    //MsWaitting::showGlobalWait();
    gFaceData.getFaceConfig(m_channel);
    if (ui->comboBox_CorridorMode->isVisible() && ui->comboBox_CorridorMode->currentIndex() != 0 && gFaceData.isFaceConflict()) {
        const int &result = MessageBox::question(this, GET_TEXT("FACE/141053", "Face Detection will be disabled, continue?"));
        if (result == MessageBox::Yes) {
            gFaceData.setFaceDisable();
        } else {
            //MsWaitting::closeGlobalWait();
            return;
        }
    }
    if (m_copyChannelList.isEmpty()) {
        m_copyChannelList.append(m_channel);
    }

    m_isCopy = m_copyChannelList.size() > 1;
    do {
        int result = saveDisplayInfo();
        if (result < 0) {
            //MsWaitting::closeGlobalWait();
            return;
        }

        qApp->processEvents();
    } while (!m_copyChannelList.isEmpty());
    QTimer::singleShot(1000, this, SLOT(onUpdatePlayVideo()));
    sendMessage(REQUEST_FLAG_GET_IPCIMAGE_DISPLAY, &m_channel, sizeof(int));
    //m_eventLoop.exec();
    //MsWaitting::closeGlobalWait();
}

void ImagePageDisplay::on_pushButton_copy_clicked()
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

void ImagePageDisplay::on_pushButton_NearViewIrValue_clicked()
{
    ui->slider_NearViewIrValue->setValue(defaultNearViewIRValue(m_model));
    onChangeNearValue();
}
void ImagePageDisplay::on_pushButton_MiddleViewIrValue_clicked()
{
    ui->slider_MiddleViewIrValue->setValue(defaultMiddleViewIRValue(m_model));
}

void ImagePageDisplay::on_pushButton_FarViewIrValue_clicked()
{
    ui->slider_FarViewIrValue->setValue(defaultFarViewIRValue(m_model));
    onChangeFarValue();
}

void ImagePageDisplay::on_pushButton_IRLedValue_clicked()
{
    ui->slider_IRLedValue->setValue(defaultIRLEDLevel(m_model));
}

void ImagePageDisplay::on_pushButton_whiteLedLevel_clicked()
{
    ui->slider_whiteLedLevel->setValue(50);
}

void ImagePageDisplay::on_pushButton_fovAdjust_clicked()
{
    ui->slider_fovAdjust->setValue(1);
}

void ImagePageDisplay::on_pushButton_DayToNightValue_clicked()
{
    int value = 36;
    int nightValue = ui->slider_NightToDayValue->value();
    if (qMsNvr->isFisheye(m_channel)) {
        value = 20;
    }
    ui->slider_DayToNightValue->setValue(value);
    if (nightValue < (value + 10)) {
        ui->slider_NightToDayValue->setValue(value + 10);
    }
}

void ImagePageDisplay::on_pushButton_NightToDayValue_clicked()
{
    int dayValue = ui->slider_DayToNightValue->value();
    if (qMsNvr->isFisheye(m_channel)) {
        ui->slider_NightToDayValue->setValue(60);
        if (dayValue > 50) {
            ui->slider_DayToNightValue->setValue(50);
        }
    } else if (ui->comboBox_smokedDome->isVisible() && m_info.smokeddomecover == 1) {
        ui->slider_NightToDayValue->setValue(62);
        if (dayValue > 52) {
            ui->slider_DayToNightValue->setValue(52);
        }
    } else {
        ui->slider_NightToDayValue->setValue(82);
        if (ui->slider_DayToNightValue->value() > 72) {
            ui->slider_DayToNightValue->setValue(72);
        }
    }
}

void ImagePageDisplay::on_pushButton_DayToNightSensitivity_clicked()
{
    ui->slider_DayToNightSensitivity->setValue(5);
}

void ImagePageDisplay::on_pushButton_NightToDaySensitivity_clicked()
{
    ui->slider_NightToDaySensitivity->setValue(5);
}

void ImagePageDisplay::on_comboBox_LensDistortCorrect_activated()
{
    if (ui->comboBox_LensDistortCorrect->currentIndex() == 1 && !m_info.isNtPlatform) {
        ui->label_fovAdjust->show();
        ui->widget_fovAdjust->setVisible(true);
    } else {
        ui->label_fovAdjust->hide();
        ui->widget_fovAdjust->setVisible(false);
    }
}

void ImagePageDisplay::onSliderDayToNightValueChange()
{
    int dayValue = ui->slider_DayToNightValue->value();
    int nightValue = ui->slider_NightToDayValue->value();
    //qDebug() << "onSliderDayToNightValueChange, dayValue:"<<dayValue<<" nightValue:"<<nightValue<<" sub:"<<(nightValue - dayValue);
    if (dayValue > 90) {
        ui->slider_DayToNightValue->setValue(90);
        ui->slider_NightToDayValue->setValue(100);
    } else if ((nightValue - dayValue) < 10) {
        ui->slider_NightToDayValue->setValue(ui->slider_DayToNightValue->value() + 10);
    }
}

void ImagePageDisplay::onSliderNightToDayValueChange()
{
    int dayValue = ui->slider_DayToNightValue->value();
    int nightValue = ui->slider_NightToDayValue->value();
    //qDebug() << "onSliderNightToDayValueChange, dayValue:"<<dayValue<<" nightValue:"<<nightValue<<" sub:"<<(nightValue - dayValue);
    if (nightValue < 11) {
        ui->slider_DayToNightValue->setValue(1);
        ui->slider_NightToDayValue->setValue(11);
    } else if ((nightValue - dayValue) < 10) {
        ui->slider_DayToNightValue->setValue(ui->slider_NightToDayValue->value() - 10);
    }
}

void ImagePageDisplay::onChangeNearValue()
{
    int nsum = m_info.smartIrLimit;
    int nearValue = static_cast<int>(ui->slider_NearViewIrValue->value());
    int farValue = static_cast<int>(ui->slider_FarViewIrValue->value());
    ui->label_note->hide();

    if (nsum > 0 && !isSpeed(m_info.productmodel) && !isProDome(m_info.productmodel)
        && !(isMiniZoomDome(m_info.productmodel) && isAFLens3X()) && !qMsNvr->isPtzBullet(m_channel)) {
        if (!qMsNvr->isFisheye(m_channel) && nearValue + farValue > nsum) {
            ui->label_note->show();
            farValue = nsum - nearValue;
        }
    }
    if (isSpeed(m_info.productmodel) && isPoe(m_info.productmodel)) {
        if (nearValue > 0 && farValue != 0) {
            ui->label_note->show();
            farValue = 0;
        }
    }
    if (qMsNvr->isPtzBullet(m_channel)) {
        double x = 0, y = 0;
        if (nearValue > 0) {
            x = 50 + (0.35 * nearValue);
        } else {
            x = 0;
        }
        if (farValue > 0) {
            y = 2 * (50 + (0.35 * farValue));
        } else {
            y = 0;
        }
        if (x + y > nsum) {
            ui->label_note->show();
            farValue = static_cast<int>(floor(((nsum - x) / 2 - 50) / 0.35));
        }
    }
    if (qMsNvr->isSigma(m_channel) || qMsNvr->isAmba(m_channel) || m_info.isNtPlatform) {
        if (nsum > 0 && isProDome(m_info.productmodel)) {
            int sum = 2 * nearValue + farValue;
            if (sum > 212) {
                ui->label_note->show();
                farValue = 212 - (2 * nearValue);
            }
        }
    }
    if (farValue < 0) {
        ui->label_note->show();
        farValue = 0;
    }
    ui->slider_FarViewIrValue->setValue(farValue);
}

void ImagePageDisplay::onChangeFarValue()
{
    int nsum = m_info.smartIrLimit;
    int nearValue = static_cast<int>(ui->slider_NearViewIrValue->value());
    int farValue = static_cast<int>(ui->slider_FarViewIrValue->value());
    ui->label_note->hide();

    if (nsum > 0 && !isSpeed(m_info.productmodel) && !isProDome(m_info.productmodel)
        && !(isMiniZoomDome(m_info.productmodel) && isAFLens3X()) && !qMsNvr->isPtzBullet(m_channel)) {
        if (!qMsNvr->isFisheye(m_channel) && nearValue + farValue > nsum) {
            ui->label_note->show();
            nearValue = nsum - farValue;
        }
    }
    if (isSpeed(m_info.productmodel) && isPoe(m_info.productmodel)) {
        if (farValue > 0 && nearValue != 0) {
            ui->label_note->show();
            nearValue = 0;
        }
        if (isSpeedLens36Or42() && farValue > 50 && nearValue != 0) {
            ui->slider_MiddleViewIrValue->setValue(0);
        }
    }
    if (qMsNvr->isPtzBullet(m_channel)) {
        double x = 0, y = 0;
        if (nearValue > 0) {
            x = 50 + (0.35 * nearValue);
        } else {
            x = 0;
        }
        if (farValue > 0) {
            y = 2 * (50 + (0.35 * farValue));
        } else {
            y = 0;
        }
        if (x + y > nsum) {
            ui->label_note->show();
            nearValue = static_cast<int>(floor((nsum - y - 50) / 0.35));
        }
    }
    if ( qMsNvr->isSigma(m_channel) || qMsNvr->isAmba(m_channel) || m_info.isNtPlatform) {
        if (nsum > 0 && isProDome(m_info.productmodel)) {
            int sum = 2 * nearValue + farValue;
            if (sum > 212) {
                ui->label_note->show();
                nearValue = static_cast<int>(floor((212 - farValue) / 2));
            }
        }
    }
    if (nearValue < 0) {
        ui->label_note->show();
        nearValue = 0;
    }
    ui->slider_NearViewIrValue->setValue(nearValue);
}

void ImagePageDisplay::on_pushButton_test_clicked()
{
    if (!m_timerTest) {
        m_timerTest = new QTimer(this);
        connect(m_timerTest, SIGNAL(timeout()), this, SLOT(onTimerTest()));
        m_timerTest->setSingleShot(true);
        m_timerTest->setInterval(1000);
    }
    if (ui->pushButton_test->text().contains("关闭")) {
        m_timerTest->start();
        ui->pushButton_test->setText("循环获取测试（已开启）");
    } else {
        m_timerTest->stop();
        ui->pushButton_test->setText("循环获取测试（已关闭）");
    }
}

void ImagePageDisplay::onTimerTest()
{
    ////showWait();
    sendMessage(REQUEST_FLAG_GET_IPCIMAGE_DISPLAY, &m_channel, sizeof(int));
    //m_eventLoop.exec();
    ////closeWait();

    if (ui->pushButton_test->text().contains("开启")) {
        m_timerTest->start();
    }
}

void ImagePageDisplay::on_comboBox_ZoomLimit_activated(int index)
{
    Q_UNUSED(index)
    if (ui->comboBox_ZoomLimit->currentIndex() == 0) {
        ui->comboBox_keepCorrectAspectRatio->setEnabled(true);
    } else {
        ui->comboBox_keepCorrectAspectRatio->setEnabled(false);
        ui->comboBox_keepCorrectAspectRatio->setCurrentIndex(0);
    }
}

void ImagePageDisplay::on_comboBox_keepCorrectAspectRatio_activated(int index)
{
    Q_UNUSED(index)
    if (ui->comboBox_keepCorrectAspectRatio->currentIndex() == 0) {
        ui->comboBox_ZoomLimit->setEnabled(true);
    } else {
        ui->comboBox_ZoomLimit->setEnabled(false);
        ui->comboBox_ZoomLimit->setCurrentIndex(0);
    }
}

void ImagePageDisplay::on_toolButtonIrStrengthValue_clicked()
{
    //MsWaitting::showGlobalWait();
    getCommonParam("getsmartirvalue");
    //m_eventLoop.exec();
    //MsWaitting::closeGlobalWait();
}

void ImagePageDisplay::on_comboBoxWhiteLEDLightControl_indexSet(int index)
{
    ui->labelWhiteLEDSensitivity->setVisible(index == WhiteLEDAutoMode);
    ui->horizontalSliderWhiteLEDSensitivity->setVisible(index == WhiteLEDAutoMode);
    ui->labelDelayTime->setVisible(index == WhiteLEDAutoMode);
    ui->widgetDelayTime->setVisible(index == WhiteLEDAutoMode);

    ui->labelWhiteLEDStartTime->setVisible(index == WhiteLEDCustomize);
    ui->timeEditWhiteLEDStartTime->setVisible(index == WhiteLEDCustomize);
    ui->labelWhiteLEDEndTime->setVisible(index == WhiteLEDCustomize);
    ui->timeEditWhiteLEDEndTime->setVisible(index == WhiteLEDCustomize);

    ui->labelBrightnessControl->setVisible(index != WhiteLEDOFF);
    ui->comboBoxBrightnessControl->setVisible(index != WhiteLEDOFF);
    ui->labelBrightness->setVisible(index != WhiteLEDOFF && ui->comboBoxBrightnessControl->currentIntData());
    ui->horizontalSliderBrightness->setVisible(index != WhiteLEDOFF && ui->comboBoxBrightnessControl->currentIntData());
}

void ImagePageDisplay::on_comboBoxBrightnessControl_indexSet(int index)
{
    if (index) {
        ui->labelBrightness->hide();
        ui->horizontalSliderBrightness->hide();
    } else {
        ui->labelBrightness->show();
        ui->horizontalSliderBrightness->show();
    }
}

void ImagePageDisplay::onWhiteLEDLightTimeChange(const QTime &time)
{
    Q_UNUSED(time)
    QTime start = ui->timeEditWhiteLEDStartTime->time();
    QTime stop = ui->timeEditWhiteLEDEndTime->time();
    if (start == stop) {
        stop = start.addSecs(60);
    }
    ui->timeEditWhiteLEDEndTime->setTime(stop);
}

void ImagePageDisplay::onDayNightTimeChange(const QTime &time)
{
    Q_UNUSED(time)
    QTime start = ui->timeEdit_StartTimeOfNight->time();
    QTime stop = ui->timeEdit_EndTimeOfNight->time();
    if (start == stop) {
        stop = start.addSecs(60);
    }
    ui->timeEdit_EndTimeOfNight->setTime(stop);
}

void ImagePageDisplay::onUpdatePlayVideo()
{
    updatePlayVideo(m_channel);
}
