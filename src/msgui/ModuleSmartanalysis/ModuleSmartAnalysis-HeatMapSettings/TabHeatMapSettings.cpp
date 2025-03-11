#include "TabHeatMapSettings.h"
#include "ui_TabHeatMapSettings.h"
#include "DrawSceneMotion.h"
#include "DrawView.h"
#include "EffectiveTimePeopleCounting.h"
#include "FaceData.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsCameraVersion.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "channelcopydialog.h"
#include "heatmapschedule.h"
#include "ptzdatamanager.h"

extern "C" {
#include "msg.h"

}

TabHeatMapSettings::TabHeatMapSettings(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::HeatMapSettings)
{
    ui->setupUi(this);

    ui->buttonGroup_channel->setCount(qMsNvr->maxChannel());
    connect(ui->buttonGroup_channel, SIGNAL(buttonClicked(int)), this, SLOT(onChannelGroupClicked(int)));

    connect(ui->slider_objectSize, SIGNAL(valueChanged(int)), this, SLOT(onSliderObjectSizeChanged(int)));
    connect(ui->slider_objectSize, SIGNAL(sliderMoved(int)), this, SLOT(onSliderObjectSizeChanged(int)));

    m_drawView = new DrawView(this);
    m_drawScene = new DrawSceneMotion(this);
    m_drawView->setScene(m_drawScene);
    ui->commonVideo->showDrawWidget(m_drawView);

    ui->lineEditDwellTime->setCheckMode(MyLineEdit::RangeCheck, 1, 300);
    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

TabHeatMapSettings::~TabHeatMapSettings()
{
    delete ui;
}

void TabHeatMapSettings::initializeData()
{
    ui->buttonGroup_channel->setCurrentIndex(0);
}

void TabHeatMapSettings::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPC_HEATMAP_SUPPORT:
        ON_RESPONSE_FLAG_GET_IPC_HEATMAP_SUPPORT(message);
        break;
    case RESPONSE_FLAG_GET_SINGLE_IPCTYPE:
        ON_RESPONSE_FLAG_GET_SINGLE_IPCTYPE(message);
        break;
    case RESPONSE_FLAG_GET_IPC_MODEL_TYPE:
        ON_RESPONSE_FLAG_GET_IPC_MODEL_TYPE(message);
        break;
    case RESPONSE_FLAG_GET_FISHEYE_MODE:
        ON_RESPONSE_FLAG_GET_FISHEYE_MODE(message);
        break;
    case RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY:
        ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(message);
        break;
    case RESPONSE_FLAG_GET_HEAT_MAP_SETTING:
        ON_RESPONSE_FLAG_GET_HEAT_MAP_SETTING(message);
        break;
    case RESPONSE_FLAG_SET_HEAT_MAP_SETTING:
        ON_RESPONSE_FLAG_SET_HEAT_MAP_SETTING(message);
        break;
    default:
        break;
    }
}

void TabHeatMapSettings::ON_RESPONSE_FLAG_GET_IPC_HEATMAP_SUPPORT(MessageReceive *message)
{
    int result = *((int *)message->data);
    m_eventLoop.exit(result);
}

void TabHeatMapSettings::ON_RESPONSE_FLAG_GET_SINGLE_IPCTYPE(MessageReceive *message)
{
    CAM_MODEL_INFO *cam_model_info = (CAM_MODEL_INFO *)message->data;

    memset(&m_model, 0, sizeof(CAM_MODEL_INFO));
    if (cam_model_info) {
        memcpy(&m_model, cam_model_info, sizeof(CAM_MODEL_INFO));
        qDebug() << "----model:" << m_model.model;
    } else {
        qWarning() << QString("HeatMapSettings::ON_RESPONSE_FLAG_GET_SINGLE_IPCTYPE, data is null.");
    }
    m_eventLoop.exit();
}

void TabHeatMapSettings::ON_RESPONSE_FLAG_GET_IPC_MODEL_TYPE(MessageReceive *message)
{
    int *type = (int *)message->data;
    if (type) {
        m_modelType = *type;
        qDebug() << "----model_type:" << m_modelType;
    } else {
        qWarning() << QString("HeatMapSettings::ON_RESPONSE_FLAG_GET_IPC_MODEL_TYPE, data is null.");
    }
    m_eventLoop.exit();
}

void TabHeatMapSettings::ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message)
{
    struct resp_fishmode_param *fishmode_param = (struct resp_fishmode_param *)message->data;

    memset(&m_fishmode, 0, sizeof(resp_fishmode_param));
    if (fishmode_param) {
        memcpy(&m_fishmode, fishmode_param, sizeof(resp_fishmode_param));
        qDebug() << "----fishmount" << m_fishmode.fishmount;
        qDebug() << "----fishdisplay" << m_fishmode.fishdisplay;
        qDebug() << "----fishcorrect" << m_fishmode.fishcorrect;
    } else {
        qWarning() << QString("HeatMapSettings::ON_RESPONSE_FLAG_GET_FISHEYE_MODE");
    }
    m_eventLoop.exit();
}

void TabHeatMapSettings::ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(MessageReceive *message)
{
    resp_image_display *image_display = (resp_image_display *)message->data;

    memset(&m_image_display, 0, sizeof(resp_image_display));
    if (image_display) {
        memcpy(&m_image_display, image_display, sizeof(resp_image_display));
        qDebug() << "----image.lencorrect" << m_image_display.image.lencorrect;
    } else {
        qWarning() << QString("HeatMapSettings::ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY, data is null.");
    }
    m_eventLoop.exit();
}

void TabHeatMapSettings::ON_RESPONSE_FLAG_GET_HEAT_MAP_SETTING(MessageReceive *message)
{
    ms_heat_map_setting *heat_map_setting = (ms_heat_map_setting *)message->data;

    memset(&m_heat_map_setting, 0, sizeof(ms_heat_map_setting));
    if (heat_map_setting) {
        memcpy(&m_heat_map_setting, heat_map_setting, sizeof(ms_heat_map_setting));

        qDebug() << "====RESPONSE_FLAG_GET_HEAT_MAP_SETTING====";
        qDebug() << "----chnid:" << heat_map_setting->chnid;
        qDebug() << "----enable:" << heat_map_setting->enable;
        qDebug() << "----sensitivity:" << heat_map_setting->sensitivity;
        qDebug() << "----minsize:" << heat_map_setting->minsize;
        qDebug() << "----mintime:" << heat_map_setting->mintime;
        qDebug() << "----adaptability:" << heat_map_setting->adaptability;
        qDebug() << heat_map_setting->sche.schedule_day->wholeday_action_type;

        ui->checkBox_enable->setChecked(heat_map_setting->enable);
        ui->slider_sensitivity->setValue(heat_map_setting->sensitivity);
        ui->slider_objectSize->setValue(heat_map_setting->minsize);
        ui->lineEditDwellTime->setText(QString("%1").arg(heat_map_setting->mintime));
        ui->slider_sceneChange->setValue(heat_map_setting->adaptability);
        m_drawScene->setRegion(heat_map_setting->map_mask);
    } else {
        qWarning() << QString("HeatMapSettings::ON_RESPONSE_FLAG_GET_HEAT_MAP_SETTING, data is null.");
    }
    m_eventLoop.exit();
}

void TabHeatMapSettings::ON_RESPONSE_FLAG_SET_HEAT_MAP_SETTING(MessageReceive *message)
{
    Q_UNUSED(message)

    m_eventLoop.exit();
}

void TabHeatMapSettings::setSettingEnable()
{
    ui->checkBox_enable->setEnabled(m_isSupported);
    if (!ui->checkBox_enable->isEnabled()) {
        ui->checkBox_enable->setChecked(false);
    }
    m_drawView->setVisible(ui->checkBox_enable->isEnabled());
    m_drawView->setEnabled(ui->checkBox_enable->isEnabled() && ui->checkBox_enable->isChecked());
    ui->slider_sensitivity->setEnabled(ui->checkBox_enable->isEnabled() && ui->checkBox_enable->isChecked());
    ui->slider_objectSize->setEnabled(ui->checkBox_enable->isEnabled() && ui->checkBox_enable->isChecked());
    ui->lineEditDwellTime->setEnabled(ui->checkBox_enable->isEnabled() && ui->checkBox_enable->isChecked());
    ui->slider_sceneChange->setEnabled(ui->checkBox_enable->isEnabled() && ui->checkBox_enable->isChecked());
    ui->pushButton_setAll->setEnabled(ui->checkBox_enable->isEnabled() && ui->checkBox_enable->isChecked());
    ui->pushButtonDeleteAll->setEnabled(ui->checkBox_enable->isEnabled() && ui->checkBox_enable->isChecked());
    ui->pushButtonHeatmapSchedule->setEnabled(ui->checkBox_enable->isEnabled() && ui->checkBox_enable->isChecked());
    ui->pushButton_copy->setEnabled(ui->checkBox_enable->isEnabled());
    ui->pushButton_apply->setEnabled(ui->checkBox_enable->isEnabled());
}

void TabHeatMapSettings::saveSetting()
{
    m_heat_map_setting.enable = ui->checkBox_enable->isChecked();
    m_heat_map_setting.sensitivity = ui->slider_sensitivity->value();
    m_heat_map_setting.minsize = ui->slider_objectSize->value();
    m_heat_map_setting.mintime = ui->lineEditDwellTime->text().toInt();
    m_heat_map_setting.adaptability = ui->slider_sceneChange->value();
    m_drawScene->getRegion(m_heat_map_setting.map_mask);
}

void TabHeatMapSettings::onLanguageChanged()
{
    ui->label_heatMap->setText(GET_TEXT("HEATMAP/104000", "Heat Map"));
    ui->label_sensitivity->setText(GET_TEXT("HEATMAP/104001", "Sensitivity"));
    ui->label_objectSize->setText(GET_TEXT("HEATMAP/104002", "Min. Object Size"));
    ui->label_dwellTime->setText(GET_TEXT("HEATMAP/104003", "Min. Dwell Time"));
    ui->label_time->setText(QString("1-300%1").arg(GET_TEXT("HEATMAP/104024", "s")));
    ui->label_sceneChange->setText(GET_TEXT("HEATMAP/104004", "Scene Change Adaptability"));
    ui->labelRegion->setText(GET_TEXT("ANPR/103021", "Region"));
    ui->labelHeatmapSchedule->setText(GET_TEXT("REGIONAL_PEOPLECOUNTING/158006", "Heatmap Schedule"));

    ui->checkBox_enable->setText(GET_TEXT("COMMON/1009", "Enable"));
    ui->pushButton_setAll->setText(GET_TEXT("MOTION/51004", "Set All"));
    ui->pushButtonDeleteAll->setText(GET_TEXT("PLAYBACK/80119", "Delete All"));
    ui->pushButtonHeatmapSchedule->setText(GET_TEXT("COMMON/1019", "Edit"));

    ui->pushButton_copy->setText(GET_TEXT("COMMON/1005", "Copy"));
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
}

void TabHeatMapSettings::onChannelGroupClicked(int channel)
{
    if (m_effectiveTime) {
        m_effectiveTime->clearCache();
    }
    ui->lineEditDwellTime->setValid(true);
    ui->widgetMessage->hideMessage();

    m_isSupported = false;
    setSettingEnable();

    m_currentChannel = channel;

    ui->commonVideo->playVideo(m_currentChannel);
    if (!LiveView::instance()->isChannelConnected(m_currentChannel)) {
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
        return;
    }

    //showWait();

    sendMessage(REQUEST_FLAG_GET_IPC_HEATMAP_SUPPORT, &m_currentChannel, sizeof(int));
    // int result = m_eventLoop.exec();
    // switch (result) {
    // case HEATMAP_SUPPORT_NO:
    //     ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
    //     break;
    // case HEATMAP_SUPPORT_NO_BY_DEWARPING_MODE:
    //     ui->widgetMessage->showMessage(GET_TEXT("HEATMAP/104019", "Heat Map is only available when dewarping mode is 1O."));
    //     break;
    // case HEATMAP_SUPPORT_NO_BY_DISTORT_CORRECT:
    //     ui->widgetMessage->showMessage(GET_TEXT("HEATMAP/104020", "Heat Map is only available when Distort Correct is On. Please set it in Image interface."));
    //     break;
    // case HEATMAP_SUPPORT_YES:
    //     sendMessage(REQUEST_FLAG_GET_HEAT_MAP_SETTING, &m_currentChannel, sizeof(int));
    //     m_eventLoop.exec();
    //     m_isSupported = true;
    //     setSettingEnable();
    //     break;
    // default:
    //     qMsWarning() << "invalid result:" << result;
    //     break;
    // }

    //closeWait();
}

void TabHeatMapSettings::onSliderObjectSizeChanged(int value)
{
    m_drawScene->setObjectSize(value);
}

void TabHeatMapSettings::on_pushButton_setAll_clicked()
{
    m_drawScene->selectAll();
}

void TabHeatMapSettings::on_pushButtonDeleteAll_clicked()
{
    m_drawScene->clearAll();
}

void TabHeatMapSettings::on_pushButton_copy_clicked()
{
    if (ui->checkBox_enable->isChecked() && !ui->lineEditDwellTime->checkValid()) {
        return;
    }
    gFaceData.getFaceConfig(m_currentChannel);
    if (qMsNvr->isNT(m_currentChannel) && ui->checkBox_enable->isChecked() && gFaceData.isFaceConflict()) {
        const int &result = MessageBox::question(this, GET_TEXT("FACE/141053", "Face Detection will be disabled, continue?"));
        if (result == MessageBox::Yes) {
            gFaceData.setFaceDisable();
        } else {
            return;
        }
    }
    ChannelCopyDialog channelCopyDialog(this);
    channelCopyDialog.setCurrentChannel(m_currentChannel);
    int result = channelCopyDialog.exec();
    if (result == QDialog::Accepted) {
        saveSetting();
        //showWait();
        QList<int> m_copyList = channelCopyDialog.checkedList();
        for (int i = 0; i < m_copyList.size(); ++i) {
            ms_heat_map_setting temp_heat_map_setting;
            memset(&temp_heat_map_setting, 0, sizeof(ms_heat_map_setting));
            memcpy(&temp_heat_map_setting, &m_heat_map_setting, sizeof(ms_heat_map_setting));
            temp_heat_map_setting.chnid = m_copyList.at(i);
            qDebug() << QString("REQUEST_FLAG_SET_HEAT_MAP_SETTING, channel: %1").arg(temp_heat_map_setting.chnid);
            sendMessage(REQUEST_FLAG_SET_HEAT_MAP_SETTING, &temp_heat_map_setting, sizeof(ms_heat_map_setting));
            //m_eventLoop.exec();
        }
        //closeWait();
    }
}

void TabHeatMapSettings::on_pushButton_apply_clicked()
{
    if (ui->checkBox_enable->isChecked() && !ui->lineEditDwellTime->checkValid()) {
        return;
    }
    gFaceData.getFaceConfig(m_currentChannel);
    if (qMsNvr->isNT(m_currentChannel) && ui->checkBox_enable->isChecked() && gFaceData.isFaceConflict()) {
        const int &result = MessageBox::question(this, GET_TEXT("FACE/141053", "Face Detection will be disabled, continue?"));
        if (result == MessageBox::Yes) {
            gFaceData.setFaceDisable();
        } else {
            return;
        }
    }

    saveSetting();
    //showWait();

    qDebug() << "----REQUEST_FLAG_SET_HEAT_MAP_SETTING----";
    qDebug() << "----chnid:" << m_heat_map_setting.chnid;
    qDebug() << "----enable:" << m_heat_map_setting.enable;
    qDebug() << "----sensitivity:" << m_heat_map_setting.sensitivity;
    qDebug() << "----minsize:" << m_heat_map_setting.minsize;
    qDebug() << "----mintime:" << m_heat_map_setting.mintime;
    qDebug() << "----adaptability:" << m_heat_map_setting.adaptability;
    sendMessage(REQUEST_FLAG_SET_HEAT_MAP_SETTING, &m_heat_map_setting, sizeof(ms_heat_map_setting));
    //m_eventLoop.exec();

    //closeWait();
}

void TabHeatMapSettings::on_pushButton_back_clicked()
{
    back();
}

void TabHeatMapSettings::on_checkBox_enable_clicked(bool checked)
{
    Q_UNUSED(checked);

    setSettingEnable();
}

void TabHeatMapSettings::on_pushButtonHeatmapSchedule_clicked()
{
    if (!m_effectiveTime) {
        m_effectiveTime = new EffectiveTimePeopleCounting(HeatMapMode, this);
    }
    m_effectiveTime->setSchedule(m_heat_map_setting.sche.schedule_day);
    m_effectiveTime->showEffectiveTime(m_currentChannel);
    int result = m_effectiveTime->exec();
    if (result == QDialog::Accepted) {
        m_effectiveTime->getSchedule(m_heat_map_setting.sche.schedule_day);
    }
    ui->pushButtonHeatmapSchedule->clearUnderMouse();
}
