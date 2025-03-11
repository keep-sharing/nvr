#include "TabAnprSettings.h"
#include "ui_TabAnprSettings.h"
#include "DrawItemAnprControl.h"
#include "DrawSceneAnpr.h"
#include "DrawView.h"
#include "EffectiveTimeAnpr.h"
#include "EventLoop.h"
#include "MessageBox.h"
#include "MsCameraVersion.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "anprdetectionsettings.h"
#include "anprroiedit.h"
#include "centralmessage.h"

extern "C" {

}

TabAnprSettings::TabAnprSettings(QWidget *parent)
    : AnprBasePage(parent)
    , ui(new Ui::AnprSettingPage)
{
    ui->setupUi(this);

    m_drawView = new DrawView(this);
    m_anprDraw = new DrawSceneAnpr(this);
    m_drawView->setScene(m_anprDraw);
    m_drawView->hide();

    //initialize table
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("ANPR/103029", "ID");
    headerList << GET_TEXT("ANPR/103030", "Name");
    headerList << GET_TEXT("ANPR/103031", "Edit");
    headerList << GET_TEXT("ANPR/103032", "Delete");
    ui->tableView_anprRegion->setHorizontalHeaderLabels(headerList);
    ui->tableView_anprRegion->setColumnCount(headerList.size());
    ui->tableView_anprRegion->setColumnHidden(ColumnCheck, true);
    connect(ui->tableView_anprRegion, SIGNAL(itemClicked(int, int)), this, SLOT(onTableItemClicked(int, int)));
    //delegate
    ui->tableView_anprRegion->setItemDelegateForColumn(ColumnEdit, new ItemButtonDelegate(QPixmap(":/common/common/edit.png"), this));
    ui->tableView_anprRegion->setItemDelegateForColumn(ColumnDelete, new ItemButtonDelegate(QPixmap(":/common/common/delete.png"), this));
    //sort
    ui->tableView_anprRegion->setSortingEnabled(false);
    ui->tableView_anprRegion->setMaximumHeight(152);

    ui->comboBox_processingResolution->clear();
    ui->comboBox_processingResolution->addItem("1920*1080", "1920*1080");
    ui->comboBox_processingResolution->addItem("1280*720", "1280*720");

    ui->lineEdit_license->setCheckMode(MyLineEdit::EmptyCheck);

    ui->comboBox_LPRNightModeEffectiveTime->clear();
    ui->comboBox_LPRNightModeEffectiveTime->addItem(GET_TEXT("WIZARD/11021", "Auto"), 0);
    ui->comboBox_LPRNightModeEffectiveTime->addItem(GET_TEXT("IMAGE/37105", "Customize"), 1);

    ui->comboBox_lprNightMode->clear();
    ui->comboBox_lprNightMode->addItem(GET_TEXT("COMMON/1009", "Enable"), true);
    ui->comboBox_lprNightMode->addItem(GET_TEXT("COMMON/1018", "Disable"), false);

    setNeedLPRNightMode(false);

    connect(ui->slider_DayToNightValue, SIGNAL(valueChanged(int)), this, SLOT(onSliderDayToNightValueChange()));
    connect(ui->slider_NightToDayValue, SIGNAL(valueChanged(int)), this, SLOT(onSliderNightToDayValueChange()));

    m_anprTrapeziform = new DrawItemAnprControl();
    addGraphicsItem(m_anprTrapeziform);
    connect(m_anprTrapeziform, SIGNAL(conflicted()), this, SLOT(onDrawPolygonConflicted()), Qt::QueuedConnection);

    onLanguageChanged();
}

TabAnprSettings::~TabAnprSettings()
{
    delete ui;
}

void TabAnprSettings::getCommonParam(const QString &param)
{
    struct req_http_common_param common_param;
    memset(&common_param, 0, sizeof(struct req_http_common_param));

    common_param.chnid = m_currentChannel;
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

void TabAnprSettings::initializeData(int channel)
{
    clearSetting();
    if (m_effectiveTime) {
        m_effectiveTime->clearCache();
    }
    m_currentChannel = channel;
    if (isChannelConnected()) {
        ui->widgetMessage->hide();
    } else {
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
        showEnable(false);
        return;
    }
    //showWait();
    sendMessage(REQUEST_FLAG_GET_LPR_SUPPORT, &m_currentChannel, sizeof(m_currentChannel));
    //m_eventLoop.exec();

    if (!isAnprSupport()) {
        //closeWait();
        return;
    }

    //get_channel_version_name(m_currentChannel, version, sizeof(version));
    MsCameraVersion cameraVersion(version);

    memset(&m_lpr_settings, 0, sizeof(ms_lpr_settings));
    sendMessage(REQUEST_FLAG_GET_LPR_SETTINGS, (void *)&m_currentChannel, sizeof(m_currentChannel));
    sendMessage(REQUEST_FLAG_GET_IPC_LPR_WILDCARDS, (void *)&m_currentChannel, sizeof(m_currentChannel));
    sendMessage(REQUEST_FLAG_GET_IPCIMAGE_ENHANCEMENT, (void *)&m_currentChannel, sizeof(int));
    if (cameraVersion >= MsCameraVersion(7, 77)) {
        sendMessage(REQUEST_FLAG_GET_IPCIMAGE_DISPLAY, &m_currentChannel, sizeof(int));
        //m_eventLoop.exec();
        sendMessage(REQUEST_FLAG_GET_IPC_MODEL_TYPE, &m_currentChannel, sizeof(int));
        getCommonParam("getadcvalue");
    }
    //closeWait();
}

void TabAnprSettings::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_LPR_SUPPORT:
        ON_RESPONSE_FLAG_GET_LPR_SUPPORT(message);
        break;
    case RESPONSE_FLAG_GET_LPR_SETTINGS:
        ON_RESPONSE_FLAG_GET_LPR_SETTINGS(message);
        break;
    case RESPONSE_FLAG_SET_LPR_SETTINGS:
        ON_RESPONSE_FLAG_SET_LPR_SETTINGS(message);
        break;
    case RESPONSE_FLAG_GET_IPC_LPR_WILDCARDS:
        ON_RESPONSE_FLAG_GET_IPC_LPR_WILDCARDS(message);
        break;
    case RESPONSE_FLAG_SET_IPC_LPR_WILDCARDS:
        ON_RESPONSE_FLAG_SET_IPC_LPR_WILDCARDS(message);
        break;
    case RESPONSE_FLAG_GET_IPCIMAGE_ENHANCEMENT:
        ON_RESPONSE_FLAG_GET_IPCIMAGE_ENHANCEMENT(message);
        break;
    case RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY:
        ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(message);
        break;
    case RESPONSE_FLAG_SET_IPCIMAGE_DISPLAY:
        ON_RESPONSE_FLAG_SET_IPCIMAGE_DISPLAY(message);
        break;
    case RESPONSE_FLAG_GET_IPC_COMMON_PARAM:
        ON_RESPONSE_FLAG_GET_IPC_COMMON_PARAM(message);
        break;
    case RESPONSE_FLAG_GET_IPC_MODEL_TYPE:
        ON_RESPONSE_FLAG_GET_IPC_MODEL_TYPE(message);
        break;
    case RESPONSE_FLAG_SET_ANPR_EVENT:
        ON_RESPONSE_FLAG_SET_ANPR_EVENT(message);
        break;
    }
}

void TabAnprSettings::ON_RESPONSE_FLAG_GET_IPC_MODEL_TYPE(MessageReceive *message)
{
    if (!message->data) {
        qWarning() << "ImagePageDisplay::ON_RESPONSE_FLAG_GET_IPC_MODEL_TYPE, data is null.";
        //        m_eventLoop.exit(-1);
        return;
    }

    int type = *((int *)message->data);
    m_currentChnType = type;

    //    //IPC需为73版本或者以上才支持该功能，低于73版本的IPC不显示该项
    //    //除去鱼眼+全景筒，其他所有机型都支持该功能
    //    char version[50];
    //    //get_channel_version_name(m_channel, version, sizeof(version));
    //    MsCameraVersion cameraVersion(version);

    //    setNeedZoomLimit(false);
    //    setNeedDNSensitivity(false);
    //    if (cameraVersion.majorVersion() >= 7 && cameraVersion.minorVersion() >= 74) {
    //        if (m_model.opticalZoom() >= 20) setNeedZoomLimit(true);
    //        if (m_model.isType65()) setNeedDNSensitivity(true);
    //        if (m_model.isType63()) setNeedDNSensitivity(true);
    //    }

    //    if (m_currentChnType != FISHEYE_IPC && !m_model.model().contains("5365") && cameraVersion.minorVersion() > 72) {
    //        ui->label_keepCorrectAspectRatio->setVisible(true);
    //        ui->comboBox_keepCorrectAspectRatio->setVisible(true);
    //    } else {
    //        ui->label_keepCorrectAspectRatio->setVisible(false);
    //        ui->comboBox_keepCorrectAspectRatio->setVisible(false);
    //    }

    //    updateCorridor(type);
    //    m_eventLoop.exit();
}

void TabAnprSettings::ON_RESPONSE_FLAG_SET_ANPR_EVENT(MessageReceive *message)
{
    Q_UNUSED(message)
    gEventLoopExit(0);
}
void TabAnprSettings::ON_RESPONSE_FLAG_GET_IPC_COMMON_PARAM(MessageReceive *message)
{
    struct resp_http_common_param *param = (struct resp_http_common_param *)message->data;
    if (!param) {
        qWarning() << "ImagePageDisplay::ON_RESPONSE_FLAG_GET_IPC_COMMON_PARAM, data is null.";
        return;
    }
    if (QString(param->info.key) == "getadcvalue") {
        int value = QString(param->info.value).toInt();
        ui->lineEdit_IrLightSensorValue->setText(QString("%1").arg((value / 1.0) > 100 ? 100 : (value / 1.0)));
    }
}

void TabAnprSettings::ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(MessageReceive *message)
{
    m_eventLoop.exit();
    resp_image_display *data = (resp_image_display *)message->data;
    if (!data) {
        qWarning() << "ImagePageDisplay::ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY, data is null.";
        return;
    }
    memcpy(&m_info, &(data->image), sizeof(image_display));

    qDebug() << "\nchipninterface:" << QString("%1").arg(QString(m_info.chipninterface).toInt(), 0, 2)
             << "\nsmartIRType:" << m_info.smartIRType
             << "\nlenCorrectType:" << m_info.lenCorrectType
             << "\ncolorkiller:" << m_info.colorkiller;

    on_comboBox_lprNightMode_currentIndexChanged(!m_lpr_settings.night_mode_info.enable);
}

void TabAnprSettings::ON_RESPONSE_FLAG_SET_IPCIMAGE_DISPLAY(MessageReceive *message)
{
    Q_UNUSED(message)
    gEventLoopExit(0);
}

void TabAnprSettings::ON_RESPONSE_FLAG_GET_IPCIMAGE_ENHANCEMENT(MessageReceive *message)
{
    if (Q_UNLIKELY(!message->data)) {
        qWarning() << __FUNCTION__ << ": received data is null.";
        return;
    }

    resp_image_enhancement *info = reinterpret_cast<resp_image_enhancement *>(message->data);
    memcpy(&imageEnhancement.data, &info->image, sizeof(image_enhancement));
    imageEnhancement.p = &imageEnhancement.data;
}

void TabAnprSettings::ON_RESPONSE_FLAG_GET_IPC_LPR_WILDCARDS(MessageReceive *message)
{
    if (!message->data) {
        qWarning() << __FUNCTION__ << " data is null.";
        return;
    }
    memcpy(&m_lpr_wildcards, message->data, sizeof(m_lpr_wildcards));
}

void TabAnprSettings::ON_RESPONSE_FLAG_SET_IPC_LPR_WILDCARDS(MessageReceive *message)
{
    Q_UNUSED(message)
    gEventLoopExit(0);
}

void TabAnprSettings::showEvent(QShowEvent *)
{
    int w = ui->tableView_anprRegion->width();
    ui->tableView_anprRegion->setColumnWidth(ColumnId, 100);
    ui->tableView_anprRegion->setColumnWidth(ColumnName, w - 300);
    ui->tableView_anprRegion->setColumnWidth(ColumnEdit, 100);
}

void TabAnprSettings::updateTable()
{
    ui->tableView_anprRegion->clearContent();
    int availableCount = 0;
    int row = 0;
    if (!m_isTrapeziform) {
        memset(m_lpr_settings.roi, 0, sizeof(ms_lpr_position_info) * 4);
        m_anprDraw->getRegionData(m_lpr_settings.roi, m_lpr_settings.jsonSupport);
        for (int i = 0; i < 4; ++i) {
            const ms_lpr_position_info &info = m_lpr_settings.roi[i];
            if (info.endX <= 0 || info.endY <= 0) {
                continue;
            }
            ui->tableView_anprRegion->setItemIntValue(row, ColumnId, i + 1);
            ui->tableView_anprRegion->setItemText(row, ColumnName, info.name);
            row++;
            availableCount++;
        }

    } else {
        m_anprTrapeziform->getShield(m_lpr_settings.normalPolygons);
        for (int i = 0; i < MAX_FACE_SHIELD; i++) {
            MS_POLYGON &shieldItem = m_lpr_settings.normalPolygons[i];
            QStringList xs = QString(shieldItem.polygonX).split(":", QString::SkipEmptyParts);
            QStringList ys = QString(shieldItem.polygonY).split(":", QString::SkipEmptyParts);
            if (xs.isEmpty() || ys.isEmpty()) {
                return;
            }
            int x = xs.at(0).toInt();
            int y = ys.at(0).toInt();
            if (x < 0 || y < 0) {
                continue;
            }
            ui->tableView_anprRegion->setItemIntValue(row, ColumnId, i + 1);
            if (QString(m_lpr_settings.normalPolygons[i].name).isEmpty()) {
                snprintf(m_lpr_settings.normalPolygons[i].name, sizeof(m_lpr_settings.normalPolygons[i].name), "ROI_%d", i + 1);
            }
            ui->tableView_anprRegion->setItemText(row, ColumnName, m_lpr_settings.normalPolygons[i].name);
            row++;
            availableCount++;
        }
    }
    ui->pushButton_add->setEnabled(availableCount < 4 && ui->pushButton_clear->isEnabled() && ui->checkBox_enable->isChecked());
    ui->pushButton_clear->clearFocus();
}

void TabAnprSettings::ON_RESPONSE_FLAG_GET_LPR_SUPPORT(MessageReceive *message)
{
    ms_lpr_support_info *lpr_support_info = (ms_lpr_support_info *)message->data;
    if (lpr_support_info) {
        qDebug() << QString("AnprSetting::ON_RESPONSE_FLAG_GET_LPR_SUPPORT, channel: %1, support: %2, version: %3")
                        .arg(lpr_support_info->chanid)
                        .arg(lpr_support_info->vca_support)
                        .arg(lpr_support_info->vca_version);
    } else {
        qWarning() << QString("AnprSetting::ON_RESPONSE_FLAG_GET_LPR_SUPPORT, data is null.");
    }

    AnprBasePage::setAnprSupportInfo(lpr_support_info);

    if (isAnprSupport()) {
        ui->widgetMessage->hide();
    } else {
        showEnable(false);
        if (isChannelConnected()) {
            ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
        }
    }
    switch (anprVersion()) { //lpr4 还有自研待处理
    case PARAM_MS_ANPR_VERSION1:
        ui->label_license->setVisible(false);
        ui->lineEdit_license->setVisible(false);
        ui->label_licenseStatus->setVisible(false);
        ui->lineEdit_licenseStatus->setVisible(false);
        ui->label_countryRegion->setVisible(true);
        ui->comboBox_countryRegion->setVisible(true);
        break;
    case PARAM_MS_ANPR_VERSION2:
    case PARAM_MS_ANPR_VERSION3:
        ui->label_license->setVisible(true);
        ui->lineEdit_license->setVisible(true);
        ui->label_licenseStatus->setVisible(true);
        ui->lineEdit_licenseStatus->setVisible(true);
        ui->label_countryRegion->setVisible(false);
        ui->comboBox_countryRegion->setVisible(false);
        break;
    }
    m_eventLoop.exit();
}

void TabAnprSettings::ON_RESPONSE_FLAG_GET_LPR_SETTINGS(MessageReceive *message)
{
    struct ms_lpr_settings *lpr_settings = static_cast<struct ms_lpr_settings *>(message->data);
    if (!lpr_settings) {
        qWarning() << QString("AnprSettingPage::ON_RESPONSE_FLAG_GET_LPR_SETTINGS, data is null.");
        return;
    }
    memcpy(&m_lpr_settings, lpr_settings, sizeof(ms_lpr_settings));
    qDebug() << QString("AnprSettingPage::ON_RESPONSE_FLAG_GET_LPR_SETTINGS");
    qDebug() << QString("resolution: %1 x %2").arg(m_lpr_settings.resolution_width).arg(m_lpr_settings.resolution_height);
    qDebug() << QString("confidence: %1").arg(m_lpr_settings.confidence);
    qDebug() << QString("cur_area: %1").arg(m_lpr_settings.cur_area);

    m_isTrapeziform = m_lpr_settings.jsonSupport && isMsLpr(m_lpr_settings.aitype);
    if (m_isTrapeziform) {
        setDrawWidget(nullptr);
        m_anprTrapeziform->init();
        m_anprTrapeziform->show();
    } else {
        setDrawWidget(m_drawView);
    }
    showEnable(true);

    ui->checkBox_enable->setChecked(m_lpr_settings.enable);
    m_hasEnable = m_lpr_settings.enable;
    ui->lineEdit_license->setText(m_lpr_settings.license.license_value);
    ui->lineEdit_licenseStatus->setText(m_lpr_settings.license.license_status == 0 ? GET_TEXT("SYSTEMNETWORK/71148", "Invalid") : GET_TEXT("SYSTEMNETWORK/71147", "Valid"));
    ui->comboBox_processingResolution->setCurrentIndexFromData(QString("%1*%2").arg(m_lpr_settings.resolution_width).arg(m_lpr_settings.resolution_height));
    ui->comboBox_countryRegion->clear();
    for (int i = 0; i < MAX_LEN_64; ++i) {
        QString region(lpr_settings->area_list[i]);
        if (!region.isEmpty()) {
            ui->comboBox_countryRegion->addItem(region, region);
        }
    }
    ui->comboBox_countryRegion->setCurrentIndexFromData(QString(lpr_settings->cur_area));

    if (!m_isTrapeziform) {
        m_anprDraw->setBaseRegionResolution(m_lpr_settings.resolution_width, m_lpr_settings.resolution_height);
        for (int i = 0; i < 4; ++i) {
            const ms_lpr_position_info &roi = m_lpr_settings.roi[i];
            qDebug() << QString("roi, index: %1, name: %2, (%3, %4) - (%5, %6)").arg(i).arg(roi.name).arg(roi.startX).arg(roi.startY).arg(roi.endX).arg(roi.endY);
        }
        m_anprDraw->setRegionData(m_lpr_settings.roi);
    } else {
        for (int i = 0; i < MAX_FACE_SHIELD; ++i) {
            m_anprTrapeziform->addShield(m_lpr_settings.normalPolygons[i], i);
        }
        m_anprTrapeziform->refreshstack();
        m_anprTrapeziform->setItemsSeleteFalse();
    }
    updateTable();

    //get_channel_version_name(m_currentChannel, version, sizeof(version));
    MsCameraVersion cameraVersion(version);

    if (cameraVersion >= MsCameraVersion(7, 74) && cameraVersion < MsCameraVersion(8, 2)) {
        setNeedLPRNightMode(true);
        ms_lpr_night_mode *lprNightMode = &m_lpr_settings.night_mode_info;
        ui->comboBox_lprNightMode->setCurrentIndex(!lprNightMode->enable);
        ui->slider_level->setValue(lprNightMode->level);
        ui->timeEdit_startTime->setTime(QTime(lprNightMode->starthour, lprNightMode->startminute));
        ui->timeEdit_endTime->setTime(QTime(lprNightMode->stophour, lprNightMode->stopminute));
    } else {
        ui->comboBox_lprNightMode->setCurrentIndex(1);
    }
    ui->label_LPRNightMode->setVisible(needLPRNightMode());
    ui->comboBox_lprNightMode->setVisible(needLPRNightMode());

    if (cameraVersion >= MsCameraVersion(8, 2)) { //判断下是不是交通IPC
        ui->labelLevel->show();
        ui->labelLPRImageMode->show();
        ui->checkBoxLPRImageMode->show();
        ui->horizontalSliderLevel->show();
        ui->checkBoxLPRImageMode->setChecked(m_lpr_settings.night_mode_info.enable);
        ui->horizontalSliderLevel->setValue(m_lpr_settings.night_mode_info.level);

        bool licenseShow, countryShow, processingShow;
        switch (m_lpr_settings.aitype) {
        case IPC_AITYPE_LPR1:
        case IPC_AITYPE_LPR4:
            licenseShow = false;
            countryShow = true;
            processingShow = true;
            break;
        case IPC_AITYPE_LPR2:
        case IPC_AITYPE_LPR3:
            licenseShow = true;
            countryShow = false;
            processingShow = true;
            break;
        case IPC_AITYPE_LPR_EU:
        case IPC_AITYPE_LPR_AP:
        case IPC_AITYPE_LPR_AM:
        case IPC_AITYPE_LPR_ME:
            licenseShow = true;
            countryShow = true;
            processingShow = false;
            break;
        default:
            licenseShow = true;
            countryShow = true;
            processingShow = true;
            break;
        }
        ui->label_license->setVisible(licenseShow);
        ui->lineEdit_license->setVisible(licenseShow);
        ui->label_licenseStatus->setVisible(licenseShow);
        ui->lineEdit_licenseStatus->setVisible(licenseShow);
        ui->label_countryRegion->setVisible(countryShow);
        ui->comboBox_countryRegion->setVisible(countryShow);
        ui->label_processingResolution->setVisible(processingShow);
        ui->comboBox_processingResolution->setVisible(processingShow);
    }
    if (m_lpr_settings.irLedExternalSupport) {
        ui->labelVehicleImageEnhancement->show();
        ui->checkBoxVehicleImageEnhancement->show();
        ui->checkBoxVehicleImageEnhancement->setChecked(m_lpr_settings.vehicleEnhancement);
    }
}

void TabAnprSettings::ON_RESPONSE_FLAG_SET_LPR_SETTINGS(MessageReceive *message)
{
    Q_UNUSED(message)
    sendMessage(REQUEST_FLAG_GET_LPR_SETTINGS, (void *)&m_currentChannel, sizeof(m_currentChannel));
    gEventLoopExit(0);
}

void TabAnprSettings::showEnable(bool enable)
{
    if (!enable) {
        m_anprDraw->clearAll();
        ui->tableView_anprRegion->clearContent();
        ui->checkBox_enable->setChecked(false);
        ui->lineEdit_license->clear();
        ui->lineEdit_licenseStatus->setText("Invalid");
    }
    ui->checkBox_enable->setEnabled(enable);
    on_checkBox_enable_stateChanged(enable);
    ui->pushButton_effectiveTime->setEnabled(ui->checkBox_enable->isEnabled() && ui->checkBox_enable->isChecked());
    ui->pushButton_apply->setEnabled(enable);
    m_drawView->setEnabled(ui->checkBox_enable->isEnabled() && ui->checkBox_enable->isChecked());
    m_anprTrapeziform->setEnabled(ui->checkBox_enable->isEnabled() && ui->checkBox_enable->isChecked() && m_isTrapeziform);
    setLPRNightModePanelEnabled(ui->checkBox_enable->isEnabled() && ui->checkBox_enable->isChecked());
}

void TabAnprSettings::onLanguageChanged()
{
    ui->checkBox_enable->setText(GET_TEXT("COMMON/1009", "Enable"));
    ui->label_note->setText(GET_TEXT("ANPR/103024", "Please draw the screen for settings."));
    ui->pushButton_add->setText(GET_TEXT("ANPR/103026", "Add"));
    ui->pushButton_clear->setText(GET_TEXT("ANPR/103027", "Clear"));
    ui->pushButton_clearAll->setText(GET_TEXT("PLAYBACK/80119", "Delete All"));

    ui->label_enable->setText(GET_TEXT("ANPR/103005", "ANPR"));
    ui->label_license->setText(GET_TEXT("ANPR/103010", "License"));
    ui->label_licenseStatus->setText(GET_TEXT("ANPR/103011", "License Status"));
    ui->label_processingResolution->setText(GET_TEXT("ANPR/103012", "Processing Resolution"));
    ui->label_countryRegion->setText(GET_TEXT("ANPR/103013", "Country / Region"));
    ui->label_detectionSettings->setText(GET_TEXT("ANPR/103014", "Detection Settings"));
    ui->label_effectiveTime->setText(GET_TEXT("SMARTEVENT/55015", "Effective Time"));
    ui->pushButton_effectiveTime->setText(GET_TEXT("ANPR/103031", "Edit"));
    ui->pushButton_detectionSettings->setText(GET_TEXT("ANPR/103031", "Edit"));
    ui->label_note_2->setText(GET_TEXT("ANPR/103072", "Please config the Action in Black List / White List or Visitor Mode."));

    ui->label_LPRNightModeEffectiveTime->setText(GET_TEXT("ANPR/103095", "LPR Night Mode Effective Time"));
    ui->label_DayToNightValue->setText(GET_TEXT("IMAGE/37106", "Day to Night Value"));
    ui->label_NightToDayValue->setText(GET_TEXT("IMAGE/37107", "Night to Day Value"));
    ui->pushButton_DayToNightValue->setText(GET_TEXT("COMMON/1057", "Reset"));
    ui->pushButton_NightToDayValue->setText(GET_TEXT("COMMON/1057", "Reset"));
    ui->label_IrLightSensorValue->setText(GET_TEXT("IMAGE/37108", "IR Light Sensor Value"));

    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->labelLPRImageMode->setText(GET_TEXT("ANPR/169000", "LPR Image Mode"));
    ui->checkBoxLPRImageMode->setText(GET_TEXT("COMMON/1009", "Enable"));
    ui->labelLevel->setText(GET_TEXT("ANPR/103091", "Level"));
    ui->labelVehicleImageEnhancement->setText(GET_TEXT("ANPR/169001", "Vehicle Image Enhancement"));
    ui->checkBoxVehicleImageEnhancement->setText(GET_TEXT("COMMON/1009", "Enable"));

    ui->label_LPRNightMode->setText(GET_TEXT("ANPR/103088", "LPR Night Mode"));
    ui->label_startTime->setText(GET_TEXT("ANPR/103089", "Start Time"));
    ui->label_endTime->setText(GET_TEXT("ANPR/103090", "End Time"));
    ui->label_level->setText(GET_TEXT("ANPR/103091", "Level"));
}

void TabAnprSettings::onTableItemClicked(int row, int column)
{
    int index = ui->tableView_anprRegion->itemIntValue(row, ColumnId) - 1;
    switch (column) {
    case ColumnEdit: {
        AnprRoiEdit edit(this);
        edit.showEdit(ui->tableView_anprRegion->itemText(row, ColumnName), m_isTrapeziform);
        int result = edit.exec();
        if (result == AnprRoiEdit::Accepted) {
            const QString &name = edit.name();
            if (!m_isTrapeziform) {
                m_anprDraw->setRegionName(index, name);
            } else {
                strcpy(m_lpr_settings.normalPolygons[index].name, name.toStdString().c_str());
            }
            updateTable();
        }
        break;
    }
    case ColumnDelete: {
        if (index != -1) {
            if (!m_isTrapeziform) {
                ms_lpr_position_info &info = m_lpr_settings.roi[index];
                ms_lpr_position_info newInfo;
                memset(&newInfo, 0, sizeof(ms_lpr_position_info));
                info = newInfo;
                for (int i = 0; i < ui->tableView_anprRegion->rowCount(); ++i) {
                    if (ui->tableView_anprRegion->itemIntValue(i, ColumnId) == (index + 1)) {
                        ui->tableView_anprRegion->removeRow(i);
                        break;
                    }
                }
                m_anprDraw->clearAt(index);
            } else {
                MS_POLYGON &shieldItem = m_lpr_settings.normalPolygons[index];
                QString strEmpty = "-1:-1:-1:-1";
                snprintf(shieldItem.polygonX, sizeof(shieldItem.polygonX), "%s", strEmpty.toStdString().c_str());
                snprintf(shieldItem.polygonY, sizeof(shieldItem.polygonY), "%s", strEmpty.toStdString().c_str());
                ui->tableView_anprRegion->removeRow(row);
                m_anprTrapeziform->clear(index);
            }
            ui->pushButton_add->setEnabled(ui->tableView_anprRegion->rowCount() < 4 && ui->checkBox_enable->isEnabled() && ui->checkBox_enable->isChecked());
        }
        break;
    }
    default:
        break;
    }
}

void TabAnprSettings::on_pushButton_add_clicked()
{
    updateTable();
}

void TabAnprSettings::on_pushButton_clear_clicked()
{
    if (!m_isTrapeziform) {
        int index = m_anprDraw->getSelectedRegionIndex();
        if (index != -1) {
            ms_lpr_position_info &info = m_lpr_settings.roi[index];
            ms_lpr_position_info newInfo;
            memset(&newInfo, 0, sizeof(ms_lpr_position_info));
            info = newInfo;
            for (int i = 0; i < ui->tableView_anprRegion->rowCount(); ++i) {
                if (ui->tableView_anprRegion->itemIntValue(i, ColumnId) == (index + 1)) {
                    ui->tableView_anprRegion->removeRow(i);
                    break;
                }
            }
            m_anprDraw->clearCurrent();
        }
    } else {
        int index = m_anprTrapeziform->clear();
        if (index > -1) {
            MS_POLYGON &shieldItem = m_lpr_settings.normalPolygons[index];
            QString strEmpty = "-1:-1:-1:-1";
            snprintf(shieldItem.polygonX, sizeof(shieldItem.polygonX), "%s", strEmpty.toStdString().c_str());
            snprintf(shieldItem.polygonY, sizeof(shieldItem.polygonY), "%s", strEmpty.toStdString().c_str());
            for (int i = 0; i < ui->tableView_anprRegion->rowCount(); i++) {
                if (ui->tableView_anprRegion->itemIntValue(i, ColumnId) == index + 1) {
                    ui->tableView_anprRegion->removeRow(i);
                    break;
                }
            }
        }
    }
    ui->pushButton_add->setEnabled(ui->tableView_anprRegion->rowCount() < 4 && ui->checkBox_enable->isEnabled() && ui->checkBox_enable->isChecked());
}

void TabAnprSettings::on_pushButton_clearAll_clicked()
{
    if (!m_isTrapeziform) {
        m_anprDraw->clearAll();
    } else {
        QString strEmpty = "-1:-1:-1:-1";
        for (int i = 0; i < MAX_FACE_SHIELD; ++i) {
            snprintf(m_lpr_settings.normalPolygons[i].polygonX, sizeof(m_lpr_settings.normalPolygons[i].polygonX), "%s", strEmpty.toStdString().c_str());
            snprintf(m_lpr_settings.normalPolygons[i].polygonY, sizeof(m_lpr_settings.normalPolygons[i].polygonY), "%s", strEmpty.toStdString().c_str());
        }
        m_anprTrapeziform->clearAll();
    }

    updateTable();
}

void TabAnprSettings::on_pushButton_effectiveTime_clicked()
{
    if (!m_effectiveTime) {
        m_effectiveTime = new EffectiveTimeAnpr(this);
    }
    m_effectiveTime->showEffectiveTime(m_currentChannel, ANPR_ENABLE);
    m_effectiveTime->show();
}

void TabAnprSettings::on_pushButton_detectionSettings_clicked()
{
    AnprDetectionSettings settings(this);
    settings.showSettings(anprVersion(), &m_lpr_settings, &m_lpr_wildcards);
    settings.exec();
}

void TabAnprSettings::on_comboBox_processingResolution_currentIndexChanged(int index)
{
    const QString &text = ui->comboBox_processingResolution->itemData(index).toString();
    //1920*1080
    QRegExp rx("(\\d+)\\*(\\d+)");
    if (rx.indexIn(text) != -1) {
        int width = rx.cap(1).toInt();
        int height = rx.cap(2).toInt();
        m_anprDraw->setBaseRegionResolution(width, height);

        m_lpr_settings.resolution_width = width;
        m_lpr_settings.resolution_height = height;
    }
}

void TabAnprSettings::on_pushButton_apply_clicked()
{
    if (ui->checkBox_enable->isChecked() && !ui->lineEdit_license->checkValid()) {
        return;
    }
    if (!m_hasEnable && ui->checkBox_enable->isChecked()) {
        MessageBox::information(this, GET_TEXT("FACE/141030", "Save failed. Reach the maximum channel of ANPR and Face Detection supported."));
        return;
    }

    //MsWaitting::showGlobalWait();
    m_anprDraw->updateRegionData(m_lpr_settings.roi);
    m_lpr_settings.enable = ui->checkBox_enable->isChecked();
    snprintf(m_lpr_settings.license.license_value, sizeof(m_lpr_settings.license.license_value), "%s", ui->lineEdit_license->text().toStdString().c_str());
    snprintf(m_lpr_settings.cur_area, sizeof(m_lpr_settings.cur_area), "%s", ui->comboBox_countryRegion->currentData().toString().toStdString().c_str());

    if (needLPRNightMode()) {
        auto nightModeInfo = &m_lpr_settings.night_mode_info;
        nightModeInfo->enable = !ui->comboBox_lprNightMode->currentIndex();
        auto time = ui->timeEdit_startTime->time();
        nightModeInfo->starthour = time.hour();
        nightModeInfo->startminute = time.minute();
        time = ui->timeEdit_endTime->time();
        nightModeInfo->stophour = time.hour();
        nightModeInfo->stopminute = time.minute();
        nightModeInfo->level = ui->slider_level->value();
        int cur_mode = ui->comboBox_LPRNightModeEffectiveTime->currentIndex();

        if (ui->comboBox_LPRNightModeEffectiveTime->isVisible()) {
            //原先不是auto改成是auto
            if (m_lpr_settings.night_mode_info.effective_mode && !cur_mode) {
                int result = MessageBox::question(this, GET_TEXT("IMAGE/37242", "The Day/Night Mode in Display of Image interface will be switched to Auto Mode, continue?"));
                if (result == MessageBox::Cancel) {
                    //MsWaitting::closeGlobalWait();
                    return;
                }
            }
        }

        if (imageEnhancement.p
            && (imageEnhancement.p->regiontype || imageEnhancement.p->wdrenable || imageEnhancement.p->hlcmode)
            && !ui->comboBox_lprNightMode->currentIndex()) {
            auto ret = MessageBox::question(this, GET_TEXT("IMAGE/37241", "BLC/WDR/HLC will be temporarily unavailable when LPR Night mode is enabled, continue?"));
            if (ret != MessageBox::Yes) {
                //MsWaitting::closeGlobalWait();
                return;
            }
        }
        if (ui->comboBox_LPRNightModeEffectiveTime->isVisible()) {
            //现在是auto
            if (!cur_mode) {
                struct req_image_display data;
                memset(&data, 0, sizeof(struct req_image_display));
                data.chnid = m_currentChannel;
                memcpy(&(data.image), &m_info, sizeof(m_info));

                data.image.colorkiller = 2; //cur_mode == 1 ? 2 : 3;
                data.image.ircutaddaytonight = ui->slider_DayToNightValue->value() * 5.5 + 0.5;
                data.image.ircutadnighttoday = ui->slider_NightToDayValue->value() * 5.5 + 0.5;
                m_info.colorkiller = data.image.colorkiller;

                sendMessage(REQUEST_FLAG_SET_IPCIMAGE_DISPLAY, &data, sizeof(struct req_image_display));
                gEventLoopExec();
            }
            nightModeInfo->effective_mode = cur_mode;
        }
    }
    if (ui->checkBoxLPRImageMode->isVisible()) {
        m_lpr_settings.night_mode_info.enable = ui->checkBoxLPRImageMode->isChecked();
        m_lpr_settings.night_mode_info.level = static_cast<int>(ui->horizontalSliderLevel->value());
    }
    if (m_lpr_settings.irLedExternalSupport) {
        m_lpr_settings.vehicleEnhancement = ui->checkBoxVehicleImageEnhancement->isChecked();
    }
    if (m_isTrapeziform) {
        m_anprTrapeziform->updataShield(m_lpr_settings.normalPolygons);
    }
    sendMessage(REQUEST_FLAG_SET_LPR_SETTINGS, &m_lpr_settings, sizeof(ms_lpr_settings));
    gEventLoopExec();
    sendMessage(REQUEST_FLAG_SET_IPC_LPR_WILDCARDS, &m_lpr_wildcards, sizeof(ms_lpr_wildcards));
    gEventLoopExec();
    //effective time
    if (m_effectiveTime) {
        m_effectiveTime->saveEffectiveTime(ANPR_ENABLE);
    }
    req_anpr_event anpr_event;
    anpr_event.chnid = m_currentChannel;
    anpr_event.modeType = ANPR_ENABLE;
    Q_UNUSED(anpr_event)
    gEventLoopExec();
    //
    //MsWaitting::closeGlobalWait();
}

void TabAnprSettings::on_pushButton_back_clicked()
{
    back();
}

bool TabAnprSettings::needLPRNightMode()
{
    return m_needLPRNightMode;
}

void TabAnprSettings::setNeedLPRNightMode(bool b)
{
    m_needLPRNightMode = b;
}

void TabAnprSettings::setAutoModeVisible(bool vsb)
{
    bool enable = !ui->comboBox_lprNightMode->currentIndex();
    bool visible = enable && vsb;

    ui->label_DayToNightValue->setVisible(visible);
    ui->widget_DayToNightValue->setVisible(visible);

    ui->label_NightToDayValue->setVisible(visible);
    ui->widget_NightToDayValue->setVisible(visible);

    ui->label_IrLightSensorValue->setVisible(visible);
    ui->lineEdit_IrLightSensorValue->setVisible(visible);
}

void TabAnprSettings::setCustomModeVisible(bool vsb)
{
    int enable = !ui->comboBox_lprNightMode->currentIndex();
    bool visible = enable && vsb;

    ui->label_startTime->setVisible(visible);
    ui->timeEdit_startTime->setVisible(visible);
    ui->label_endTime->setVisible(visible);
    ui->timeEdit_endTime->setVisible(visible);
}

void TabAnprSettings::on_comboBox_LPRNightModeEffectiveTime_activated(int index)
{
    ui->comboBox_LPRNightModeEffectiveTime->setCurrentIndex(index);
    bool visible = !ui->comboBox_LPRNightModeEffectiveTime->itemData(index).toBool();
    qDebug() << "on_comboBox_LPRNightModeEffectiveTime_activated:" << index << "  visible:" << visible;

    if (index) {
        setAutoModeVisible(visible);
        setCustomModeVisible(!visible);
    } else {
        setCustomModeVisible(!visible);
        setAutoModeVisible(visible);
    }

    ui->slider_DayToNightValue->setValue(m_info.ircutaddaytonight / 5.5 + 0.5);
    ui->slider_NightToDayValue->setValue(m_info.ircutadnighttoday / 5.5 + 0.5);
}

void TabAnprSettings::on_comboBox_lprNightMode_currentIndexChanged(int idx)
{
    qDebug() << "on_comboBox_lprNightMode_currentIndexChanged:" << idx;
    MsCameraVersion cameraVersion(version);
    bool visible = ui->comboBox_lprNightMode->itemData(idx).toBool() && cameraVersion < MsCameraVersion(8, 2);

    ui->label_level->setVisible(visible);
    ui->slider_level->setVisible(visible);

    if (cameraVersion >= MsCameraVersion(7, 77)) {
        ui->label_LPRNightModeEffectiveTime->setVisible(visible);
        ui->comboBox_LPRNightModeEffectiveTime->setVisible(visible);
        if (m_lpr_settings.night_mode_info.enable)
            on_comboBox_LPRNightModeEffectiveTime_activated(m_lpr_settings.night_mode_info.effective_mode); //0:auto  1:customize
        else
            on_comboBox_LPRNightModeEffectiveTime_activated(m_info.colorkiller == 2 ? 0 : 1);
    } else {
        ui->label_LPRNightModeEffectiveTime->setVisible(false);
        ui->comboBox_LPRNightModeEffectiveTime->setVisible(false);
        setAutoModeVisible(false);
        setCustomModeVisible(visible);
    }
}

void TabAnprSettings::setLPRNightModePanelEnabled(bool enable)
{
    ui->label_LPRNightMode->setEnabled(enable);
    ui->comboBox_lprNightMode->setEnabled(enable);
    ui->label_startTime->setEnabled(enable);
    ui->timeEdit_startTime->setEnabled(enable);
    ui->label_endTime->setEnabled(enable);
    ui->timeEdit_endTime->setEnabled(enable);
    ui->label_level->setEnabled(enable);
    ui->slider_level->setEnabled(enable);
}

bool TabAnprSettings::isProDome(char *productmodel)
{
    QString model = QString("%1").arg(productmodel);
    QString sMini = model.mid(6, 2);
    if (sMini == "72")
        return true;
    return false;
}

bool TabAnprSettings::isSmokedDomeCoverSensor(char *sensortype)
{
    QString sensor = QString("%1").arg(sensortype);
    if (sensor.contains("imx385") || sensor.contains("imx290")
        || sensor.contains("imx291") || sensor.contains("imx123"))
        return true;
    else
        return false;
}

bool TabAnprSettings::isMiniPtzDome(char *productmodel)
{
    QString model = QString("%1").arg(productmodel);
    int sBullet1 = model.mid(6, 1).toInt();
    int sBullet2 = model.mid(7, 1).toInt();
    if (sBullet1 == 7 && sBullet2 == 1)
        return true;
    return false;
}

void TabAnprSettings::clearSetting()
{
    ui->lineEdit_license->setValid(true);
    ui->labelLevel->hide();
    ui->labelLPRImageMode->hide();
    ui->labelVehicleImageEnhancement->hide();
    ui->checkBoxLPRImageMode->hide();
    ui->horizontalSliderLevel->hide();
    ui->checkBoxVehicleImageEnhancement->hide();
    ui->comboBox_lprNightMode->setCurrentIndex(1);
    ui->comboBox_LPRNightModeEffectiveTime->setCurrentIndex(0);
    ui->comboBox_processingResolution->show();
    ui->label_processingResolution->show();
    m_isTrapeziform = false;
    m_anprTrapeziform->hide();
    setNeedLPRNightMode(false);
}

bool TabAnprSettings::isMsLpr(IPC_AITYPE_E aitype)
{
    if (aitype == IPC_AITYPE_LPR_AM
        || aitype == IPC_AITYPE_LPR_AP
        || aitype == IPC_AITYPE_LPR_EU
        || aitype == IPC_AITYPE_LPR_ME) {
        return true;
    }
    return false;
}

void TabAnprSettings::on_pushButton_DayToNightValue_clicked()
{
    int value = 36;
    int nightValue = ui->slider_NightToDayValue->value();
    if (qMsNvr->isFisheye(m_currentChannel)) {
        value = 20;
    }
    ui->slider_DayToNightValue->setValue(value);
    if (nightValue < (value + 10)) {
        ui->slider_NightToDayValue->setValue(value + 10);
    }
}

void TabAnprSettings::on_pushButton_NightToDayValue_clicked()
{
    int dayValue = ui->slider_DayToNightValue->value();
    if (qMsNvr->isFisheye(m_currentChannel)) {
        ui->slider_NightToDayValue->setValue(60);
        if (dayValue > 50) {
            ui->slider_DayToNightValue->setValue(50);
        }
    } else if ((isMiniPtzDome(m_info.productmodel)
                || (isProDome(m_info.productmodel)
                    && isSmokedDomeCoverSensor(m_info.sensortype)))
               && m_info.smokeddomecover == 1) {
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

void TabAnprSettings::onSliderDayToNightValueChange()
{
    int dayValue = ui->slider_DayToNightValue->value();
    int nightValue = ui->slider_NightToDayValue->value();
    qDebug() << "AnprSettingPage onSliderDayToNightValueChange, dayValue:" << dayValue << " nightValue:" << nightValue << " sub:" << (nightValue - dayValue);
    if (dayValue > 90) {
        ui->slider_DayToNightValue->setValue(90);
        ui->slider_NightToDayValue->setValue(100);
    } else if ((nightValue - dayValue) < 10) {
        ui->slider_NightToDayValue->setValue(ui->slider_DayToNightValue->value() + 10);
    }
}

void TabAnprSettings::onSliderNightToDayValueChange()
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

void TabAnprSettings::on_checkBox_enable_stateChanged(int arg1)
{
    ui->tableView_anprRegion->setEnabled(arg1);
    m_drawView->setEnabled(arg1);
    if (m_isTrapeziform) {
        m_anprTrapeziform->clearSelect();
        m_anprTrapeziform->setItemEnable(arg1);
    }
    if (!arg1) {
        m_anprDraw->cancelSelected();
    }
    bool isEnable = ui->checkBox_enable->isEnabled() && ui->checkBox_enable->isChecked();
    ui->pushButton_add->setEnabled(isEnable && ui->tableView_anprRegion->rowCount() < 4);
    ui->pushButton_clear->setEnabled(isEnable);
    ui->pushButton_clearAll->setEnabled(isEnable);

    ui->lineEdit_license->setEnabled(isEnable);
    ui->lineEdit_licenseStatus->setEnabled(isEnable);
    ui->comboBox_processingResolution->setEnabled(isEnable);
    ui->comboBox_countryRegion->setEnabled(isEnable);
    ui->comboBox_lprNightMode->setEnabled(isEnable);
    ui->comboBox_LPRNightModeEffectiveTime->setEnabled(isEnable);
    ui->widget_DayToNightValue->setEnabled(isEnable);
    ui->widget_NightToDayValue->setEnabled(isEnable);
    ui->pushButton_effectiveTime->setEnabled(isEnable);
    ui->pushButton_detectionSettings->setEnabled(isEnable);
    ui->timeEdit_startTime->setEnabled(isEnable);
    ui->timeEdit_endTime->setEnabled(isEnable);
    ui->slider_level->setEnabled(isEnable);
    ui->checkBoxLPRImageMode->setEnabled(isEnable);
    ui->horizontalSliderLevel->setEnabled(ui->checkBoxLPRImageMode->isChecked() && isEnable);
    ui->checkBoxVehicleImageEnhancement->setEnabled(isEnable);
}

void TabAnprSettings::on_comboBox_countryRegion_activated(const QString &arg1)
{
    Q_UNUSED(arg1)
    snprintf(m_lpr_settings.cur_area, sizeof(m_lpr_settings.cur_area), "%s", ui->comboBox_countryRegion->currentData().toString().toStdString().c_str());
}

void TabAnprSettings::on_checkBoxLPRImageMode_stateChanged(int arg1)
{
    Q_UNUSED(arg1)
    ui->horizontalSliderLevel->setEnabled(ui->checkBoxLPRImageMode->isChecked());
}

void TabAnprSettings::onDrawPolygonConflicted()
{
    MessageBox::information(this, GET_TEXT("SMARTEVENT/55066", "The boundaries of the area cannot intersect. Please redraw."));
    m_anprTrapeziform->clear();
}
