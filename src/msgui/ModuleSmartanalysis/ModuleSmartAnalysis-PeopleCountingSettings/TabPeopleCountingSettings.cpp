#include "TabPeopleCountingSettings.h"
#include "ui_TabPeopleCountingSettings.h"
#include "ActionVCA.h"
#include "DrawSceneObjectSize.h"
#include "DynamicDisplayData.h"
#include "EffectiveTime.h"
#include "EffectiveTimePeopleCounting.h"
#include "FaceData.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "ThresholdsEdit.h"
#include "VideoLineCrossing.h"
#include "centralmessage.h"
#include "channelcopydialog.h"

#define DRAW_LINE 0
#define FINISH_LINE 1

const MsQtVca::ObjectVcaType ObjectType = MsQtVca::PeopleCounting;

TabPeopleCountingSettings::TabPeopleCountingSettings(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::TabPeopleCountingSettings)
{
    ui->setupUi(this);
    ui->widget_buttonGroup->setCount(qMsNvr->maxChannel());
    ui->widget_buttonGroup->setCurrentIndex(0);
    connect(ui->widget_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onChannelButtonClicked(int)));
    m_drawLine = new GraphicsDrawLineCrossing();
    ui->commonVideo->addGraphicsItem(m_drawLine);
    ui->radioButton_minLayout->setAutoExclusive(false);
    ui->radioButton_maxLayout->setAutoExclusive(false);

    m_drawScene = new DrawSceneObjectSize(this);
    connect(m_drawScene, SIGNAL(objectSizeChanged(QRect, MsQtVca::ObjectSizeType)), this, SLOT(onSceneObjectSizeChanged(QRect, MsQtVca::ObjectSizeType)));
    m_settings_info2 = new ms_vca_settings_info2;
    m_peopleCountInfo = new ms_smart_event_people_cnt;

    ui->widgetMinLineBox->setWidthRange(1, 320);
    ui->widgetMinLineBox->setHeightRange(1, 240);
    connect(ui->widgetMinLineBox, SIGNAL(widthChange()), this, SLOT(onLineEditMinWidthEditingFinished()));
    connect(ui->widgetMinLineBox, SIGNAL(heightChange()), this, SLOT(onLineEditMinHeightEditingFinished()));

    ui->widgetMaxLineBox->setWidthRange(1, 320);
    ui->widgetMaxLineBox->setHeightRange(1, 240);
    connect(ui->widgetMaxLineBox, SIGNAL(widthChange()), this, SLOT(onLineEditMaxWidthEditingFinished()));
    connect(ui->widgetMaxLineBox, SIGNAL(heightChange()), this, SLOT(onLineEditMaxHeightEditingFinished()));

    ui->comboBoxLine->beginEdit();
    ui->comboBoxLine->clear();
    ui->comboBoxLine->addItem("1", 0);
    ui->comboBoxLine->addItem("2", 1);
    ui->comboBoxLine->addItem("3", 2);
    ui->comboBoxLine->addItem("4", 3);
    ui->comboBoxLine->endEdit();

    ui->comboBoxDirection->beginEdit();
    ui->comboBoxDirection->clear();
    ui->comboBoxDirection->addItem("A->B", 0);
    ui->comboBoxDirection->addItem("B->A", 1);
    ui->comboBoxDirection->endEdit();

    ui->comboBoxAlarmTrigger->beginEdit();
    ui->comboBoxAlarmTrigger->clear();
    ui->comboBoxAlarmTrigger->addItem(GET_TEXT("REGIONAL_PEOPLECOUNTING/158003", "Total Counting"), 0);
    ui->comboBoxAlarmTrigger->addItem(GET_TEXT("REGIONAL_PEOPLECOUNTING/158004", "Single Counting"), 1);
    ui->comboBoxAlarmTrigger->endEdit();

    ui->horizontalSliderSensitivity->setRange(1, 10);
    connect(&gDynamicDisplayData, SIGNAL(dynamicDataChanged(int, int)), this, SLOT(updateDynamicDisplay(int, int)));

    clearSettings();
    setMultiLineVisisble(true);
    onLanguageChanged();
}

TabPeopleCountingSettings::~TabPeopleCountingSettings()
{
    if (m_peopleCountInfo) {
        delete m_peopleCountInfo;
        m_peopleCountInfo = nullptr;
    }
    if (m_settings_info2) {
        delete m_settings_info2;
        m_settings_info2 = nullptr;
    }
    delete ui;
}

void TabPeopleCountingSettings::initializeData()
{
    ui->widget_buttonGroup->setCurrentIndex(0);
}

void TabPeopleCountingSettings::saveData()
{
    return;
    if (!checkObjectSize()) {
        clearCopyInfo();
        return;
    }
    //showWait();
    gFaceData.getFaceConfig(m_currentChannel);
    if (ui->checkBoxEnable->isChecked() && gFaceData.isFaceConflict()) {
        const int &result = MessageBox::question(this, GET_TEXT("FACE/141053", "Face Detection will be disabled, continue?"));
        if (result == MessageBox::Yes) {
            gFaceData.setFaceDisable();
        } else {
            //closeWait();
            clearCopyInfo();
            return;
        }
    }
    ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateFinished);
    ui->pushButtonLineEdit->setButtonState(PushButtonEditState::StateFinished);

    if (m_action) {
        m_action->saveAction();
    }
    if (m_copyFlag != 0) {
        copy_smart_event_action(PEOPLE_CNT, currentChannel(), m_copyFlag, 1);
    }
    REQ_UPDATE_CHN req = { 0 };
    ms_set_bit(&m_copyFlag, m_currentChannel, 1);
    req.chnMask = m_copyFlag;
    Q_UNUSED(req)

    m_peopleCountInfo->sensitivity = static_cast<int>(ui->horizontalSliderSensitivity->value());
    if (m_peopleCountInfo->lineNum == 1) {
        m_peopleCountInfo->show_osd_enable = ui->checkBox_show->isChecked();
        m_peopleCountInfo->osd_text_position = ui->comboBox_textPos->currentIndex();
        m_peopleCountInfo->triggerEnable = ui->checkBoxAlarmTrigger->isChecked();
    }
    m_drawLine->getPeopleCountInfo(m_peopleCountInfo);
    char str[MAX_LEN_4096];
    char scheStr[IPC_SCHE_TEN_SECTIONS_STR_LEN + 1] = { 0 };
    for (int i = 0; i < MAX_DAY_NUM_IPC; ++i) {
        for (int j = 0; j < IPC_SCHE_TEN_SECTIONS; ++j) {
            snprintf(scheStr + strlen(scheStr), sizeof(scheStr) - strlen(scheStr), "%s:%s;",
                     m_peopleCountInfo->sche.schedule_day[i].schedule_item[j].start_time, m_peopleCountInfo->sche.schedule_day[i].schedule_item[j].end_time);
        }
        snprintf(str + strlen(str), sizeof(str) - strlen(str), "&count_schedule_%d=%s", i, scheStr);
    }
    qDebug() << str;
    sendMessage(REQUEST_FLAG_SET_VCA_PEOPLE_COUNT, m_peopleCountInfo, sizeof(ms_smart_event_people_cnt));
    //m_eventLoop.exec();
    sendMessage(REQUEST_FLAG_SET_VAC_SETTINGS2, m_settings_info2, sizeof(ms_vca_settings_info2));
    //m_eventLoop.exec();
    clearCopyInfo();
    //closeWait();
}

void TabPeopleCountingSettings::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_FISHEYE_MODE:
        ON_RESPONSE_FLAG_GET_FISHEYE_MODE(message);
        break;
    case RESPONSE_FLAG_GET_VAC_SUPPORT:
        ON_RESPONSE_FLAG_GET_VAC_SUPPORT(message);
        break;
    case RESPONSE_FLAG_GET_VCA_LICENSE:
        ON_RESPONSE_FLAG_GET_VCA_LICENSE(message);
        break;
    case RESPONSE_FLAG_GET_VCA_PEOPLE_COUNT:
        ON_RESPONSE_FLAG_GET_VCA_PEOPLE_COUNT(message);
        break;
    case RESPONSE_FLAG_SET_VCA_PEOPLE_COUNT:
        ON_RESPONSE_FLAG_SET_VCA_PEOPLE_COUNT(message);
        break;
    case RESPONSE_FLAG_GET_VAC_SETTINGS2:
        ON_RESPONSE_FLAG_GET_VAC_SETTINGS2(message);
        break;
    case RESPONSE_FLAG_SET_VAC_SETTINGS2:
        ON_RESPONSE_FLAG_SET_VAC_SETTINGS2(message);
        break;
    default:
        break;
    }
}

void TabPeopleCountingSettings::ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message)
{
    Q_UNUSED(message)
}

void TabPeopleCountingSettings::ON_RESPONSE_FLAG_GET_VAC_SUPPORT(MessageReceive *message)
{
    ms_vca_support_info *info = (ms_vca_support_info *)message->data;
    m_isVcaSupport = (info->vca_support == 1);
    m_vcaType = info->vca_type;
    qMsDebug() << QString("channel: %1, vca_support: %2, type: %3").arg(info->chanid).arg(info->vca_support).arg(info->vca_type);

    m_eventLoop.exit();
}

void TabPeopleCountingSettings::ON_RESPONSE_FLAG_GET_VCA_LICENSE(MessageReceive *message)
{
    ms_vca_license *info = (ms_vca_license *)message->data;
    m_isLicenseVaild = (info->vca_license_status == 1);
    qMsDebug() << QString("channel: %1, vca_license_status: %2").arg(info->chanid).arg(info->vca_license_status);

    m_eventLoop.exit();
}

void TabPeopleCountingSettings::ON_RESPONSE_FLAG_GET_VCA_PEOPLE_COUNT(MessageReceive *message)
{
    if (message->isNull()) {
        qMsWarning() << QString("data is null.");
        m_eventLoop.exit(-1);
        return;
    }
    ms_smart_event_people_cnt *info = (ms_smart_event_people_cnt *)message->data;
    memcpy(m_peopleCountInfo, info, sizeof(ms_smart_event_people_cnt));

    //delete down row
    m_drawLine->setPeopleCountInfo(m_peopleCountInfo);
    if (m_peopleCountInfo->lineNum == 4) {
        ui->horizontalSliderSensitivity->setValue(m_peopleCountInfo->sensitivity);
        MS_PEOPLECNT_DATA peopleCntInfo = gDynamicDisplayData.peopleCntData(m_currentChannel);
        m_drawLine->setLineCntData(&peopleCntInfo);
        m_drawLine->setLineCntOsdType(m_peopleCountInfo->osdType);
    } else {
        setMultiLineVisisble(false);
        ui->checkBoxAlarmTrigger->setChecked(m_peopleCountInfo->triggerEnable);
        ui->checkBox_show->setChecked(m_peopleCountInfo->show_osd_enable);
        ui->comboBox_textPos->setCurrentIndex(m_peopleCountInfo->osd_text_position);
    }
    ui->comboBoxAlarmTrigger->setCurrentIndexFromData(m_peopleCountInfo->triggerType);
    m_drawLine->setCurrentLine(m_currentLine);
    ui->comboBoxLine->setCurrentIndex(m_currentLine);
    m_eventLoop.exit();
}

void TabPeopleCountingSettings::ON_RESPONSE_FLAG_SET_VCA_PEOPLE_COUNT(MessageReceive *message)
{
    if (message->isNull()) {
        qMsWarning() << "data is null.";
        m_eventLoop.exit(-1);
        return;
    }
    int result = *((int *)message->data);
    if (result < 0) {
        qMsWarning() << QString("result: %1").arg(result);
    }
    m_eventLoop.exit(result);
}

void TabPeopleCountingSettings::ON_RESPONSE_FLAG_GET_VAC_SETTINGS2(MessageReceive *message)
{
    if (message->isNull()) {
        m_eventLoop.exit(-1);
        return;
    }
    ms_vca_settings_info2 *info = static_cast<ms_vca_settings_info2 *>(message->data);
    memcpy(m_settings_info2, info, sizeof(ms_vca_settings_info2));
    int minX = m_settings_info2->minobject_window_rectangle_x[ObjectType];
    int minY = m_settings_info2->minobject_window_rectangle_y[ObjectType];
    int minWidth = m_settings_info2->minobject_window_rectangle_width[ObjectType];
    int minHeight = m_settings_info2->minobject_window_rectangle_height[ObjectType];
    int maxX = m_settings_info2->maxobject_window_rectangle_x[ObjectType];
    int maxY = m_settings_info2->maxobject_window_rectangle_y[ObjectType];
    int maxWidth = m_settings_info2->maxobject_window_rectangle_width[ObjectType];
    int maxHeight = m_settings_info2->maxobject_window_rectangle_height[ObjectType];
    ui->widgetMinLineBox->setWidthValue(minWidth);
    ui->widgetMinLineBox->setHeightValue(minHeight);
    ui->widgetMaxLineBox->setWidthValue(maxWidth);
    ui->widgetMaxLineBox->setHeightValue(maxHeight);
    qMsDebug() << QString("min: %1, %2, %3 x %4").arg(minX).arg(minY).arg(minWidth).arg(minHeight);
    qMsDebug() << QString("max: %1, %2, %3 x %4").arg(maxX).arg(maxY).arg(maxWidth).arg(maxHeight);

    m_eventLoop.exit();
}

void TabPeopleCountingSettings::ON_RESPONSE_FLAG_SET_VAC_SETTINGS2(MessageReceive *message)
{
    if (message->isNull()) {
        qMsWarning() << "data is null.";
        m_eventLoop.exit(-1);
    }
    int result = *((int *)message->data);
    if (result < 0) {
        qMsWarning() << QString("result: %1").arg(result);
    }
    m_eventLoop.exit(result);
}

void TabPeopleCountingSettings::clearSettings()
{
    ui->checkBoxEnable->setChecked(false);
    m_drawLine->clearAllLine();
}

void TabPeopleCountingSettings::updateEnableState()
{
    ui->checkBoxEnable->setEnabled(m_isConnected && m_isSupported);
    ui->pushButtonObjectSizeEdit->setEnabled(ui->checkBoxEnable->isEnabled() && ui->checkBoxEnable->isChecked());
    ui->pushButtonObjectSizeReset->setEnabled(ui->checkBoxEnable->isEnabled() && ui->checkBoxEnable->isChecked());
    if (ui->pushButtonObjectSizeReset->isEnabled() && ui->pushButtonObjectSizeEdit->buttonState() == PushButtonEditState::StateEditing) {
        ui->pushButtonObjectSizeReset->show();
    } else {
        ui->pushButtonObjectSizeReset->hide();
    }

    ui->widgetSize->setEnabled(ui->checkBoxEnable->isEnabled() && ui->checkBoxEnable->isChecked()
                               && ui->pushButtonObjectSizeEdit->buttonState() == PushButtonEditState::StateEditing);
    ui->pushButtonApply->setEnabled(ui->checkBoxEnable->isEnabled());
    ui->pushButtonCopy->setEnabled(ui->checkBoxEnable->isEnabled());
    ui->commonVideo->setDrawViewVisible(ui->pushButtonObjectSizeEdit->buttonState() == PushButtonEditState::StateEditing);
    ui->checkBox_show->setEnabled(ui->checkBoxEnable->isEnabled() && ui->checkBoxEnable->isChecked());
    ui->comboBox_textPos->setEnabled(ui->checkBoxEnable->isEnabled() && ui->checkBoxEnable->isChecked());
    ui->pushButton_clearCount->setEnabled(ui->checkBoxEnable->isEnabled() && ui->checkBoxEnable->isChecked());

    ui->pushButtonLineEdit->setEnabled(ui->checkBoxEnable->isEnabled() && ui->checkBoxEnable->isChecked());
    ui->pushButton_clear->setEnabled(ui->checkBoxEnable->isEnabled() && ui->checkBoxEnable->isChecked());

    ui->comboBoxLine->setEnabled(m_isConnected && m_isSupported);
    ui->comboBoxDirection->setEnabled(ui->checkBoxEnable->isEnabled() && ui->checkBoxEnable->isChecked());
    ui->horizontalSliderSensitivity->setEnabled(ui->checkBoxEnable->isEnabled() && ui->checkBoxEnable->isChecked());
    ui->pushButtonCountingInfoEdit->setEnabled(ui->checkBoxEnable->isEnabled() && ui->checkBoxEnable->isChecked());
    if (ui->pushButton_clear->isEnabled() && ui->pushButtonLineEdit->buttonState() == PushButtonEditState::StateEditing) {
        ui->pushButton_clear->show();
    } else {
        ui->pushButton_clear->hide();
    }
    ui->checkBoxAlarmTrigger->setEnabled(ui->checkBoxEnable->isEnabled() && ui->checkBoxEnable->isChecked());
    ui->comboBoxAlarmTrigger->setEnabled(ui->checkBoxEnable->isEnabled() && ui->checkBoxEnable->isChecked());
    if (ui->checkBoxAlarmTrigger->isVisible()) {
        ui->pushButtonThresholds->setEnabled(ui->checkBoxEnable->isEnabled() && ui->checkBoxEnable->isChecked() && ui->checkBoxAlarmTrigger->isChecked());
    } else {
        ui->pushButtonThresholds->setEnabled(ui->checkBoxEnable->isEnabled() && ui->checkBoxEnable->isChecked());
    }
    ui->pushButtonPeopleCountingSchedule->setEnabled(ui->checkBoxEnable->isEnabled() && ui->checkBoxEnable->isChecked());
    ui->pushButtonAction->setEnabled(ui->checkBoxEnable->isEnabled() && ui->checkBoxEnable->isChecked());
}

void TabPeopleCountingSettings::onLanguageChanged()
{
    ui->labelObjectSizeLimits->setText(GET_TEXT("SMARTEVENT/55063", "Object Size Limits"));
    ui->pushButtonObjectSizeReset->setText(GET_TEXT("COMMON/1057", "Reset"));

    ui->label_peopelCounting->setText(GET_TEXT("SMARTEVENT/55008", "People Counting"));
    ui->label_showOSD->setText(GET_TEXT("SMARTEVENT/55026", "Show Counting OSD"));
    ui->label_position->setText(GET_TEXT("SMARTEVENT/55027", "Text Position"));
    ui->label_drawLine->setText(GET_TEXT("SMARTEVENT/55067", "Line Edit"));
    ui->label_clearCount->setText(GET_TEXT("SMARTEVENT/55028", "Clear Count"));
    ui->comboBox_textPos->setItemText(0, GET_TEXT("IMAGE/37335", "Top-Left"));
    ui->comboBox_textPos->setItemText(1, GET_TEXT("IMAGE/37336", "Top-Right"));
    ui->comboBox_textPos->setItemText(2, GET_TEXT("IMAGE/37337", "Bottom-Left"));
    ui->comboBox_textPos->setItemText(3, GET_TEXT("IMAGE/37338", "Bottom-Right"));
    ui->pushButton_clear->setText(GET_TEXT("CHANNELMANAGE/30023", "Delete"));
    ui->pushButton_clearCount->setText(GET_TEXT("SMARTEVENT/55022", "Clear"));

    ui->pushButtonApply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->checkBoxEnable->setText(GET_TEXT("COMMON/1009", "Enable"));

    ui->labelLine->setText(GET_TEXT("PEOPLECOUNTING_SEARCH/145003", "Line"));
    ui->labelDirection->setText(GET_TEXT("PEOPLECOUNTING_SEARCH/145004", "Direction"));
    ui->labelSensitivity->setText(GET_TEXT("MOTION/51003", "Sensitivity"));
    ui->labelCountingInfomation->setText(GET_TEXT("PEOPLECOUNTING_SEARCH/145005", "Counting Information"));
    ui->pushButtonCountingInfoEdit->setText(GET_TEXT("COMMON/1019", "Edit"));

    ui->pushButtonCopy->setText(GET_TEXT("COMMON/1005", "Copy"));

    ui->pushButtonThresholds->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->pushButtonPeopleCountingSchedule->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->pushButtonAction->setText(GET_TEXT("COMMON/1019", "Edit"));

    ui->labelAlarmTrigger->setText(GET_TEXT("REGIONAL_PEOPLECOUNTING/158000", "Alarm Trigger"));
    ui->labelThresholds->setText(GET_TEXT("REGIONAL_PEOPLECOUNTING/158001", "Thresholds"));
    ui->labelPeopleCountingSchedule->setText(GET_TEXT("REGIONAL_PEOPLECOUNTING/158002", "People Counting Schedule"));
    ui->labelAction->setText(GET_TEXT("VIDEOLOSS/50002", "Action"));

    ui->pushButtonThresholds->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->pushButtonPeopleCountingSchedule->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->pushButtonAction->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->checkBoxAlarmTrigger->setText(GET_TEXT("COMMON/1009", "Enable"));
}

void TabPeopleCountingSettings::hideMessage()
{
    ui->widgetMessage->hide();
}

void TabPeopleCountingSettings::showNotConnectedMessage()
{
    //closeWait();
    ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
}

void TabPeopleCountingSettings::showNotSupportMessage()
{
    //closeWait();
    ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
}

void TabPeopleCountingSettings::showDataError()
{
    //closeWait();
    ShowMessageBox(this, "Get data error, data is nullptr!");
}

int TabPeopleCountingSettings::waitForCheckVcaSupport()
{
    return 0;
    sendMessage(REQUEST_FLAG_GET_VAC_SUPPORT, &m_currentChannel, sizeof(int));
    m_eventLoop.exec();
    if (!m_isVcaSupport) {
        return -1;
    }
    sendMessage(REQUEST_FLAG_GET_VCA_LICENSE, &m_currentChannel, sizeof(int));
    m_eventLoop.exec();
    if (!m_isLicenseVaild) {
        return -1;
    }
    char version[50];
    //get_channel_version_name(m_currentChannel, version, sizeof(version));
    MsCameraVersion cameraVersion(version);
    if (qMsNvr->isSigma(m_currentChannel) && cameraVersion < MsCameraVersion(7, 77, 7)) {
        return -1;
    }
    return 0;
}

int TabPeopleCountingSettings::waitForCheckFisheeyeSupport()
{
    sendMessage(REQUEST_FLAG_GET_FISHEYE_MODE, &m_currentChannel, sizeof(int));
    int result = m_eventLoop.exec();
    return result;
}

bool TabPeopleCountingSettings::checkObjectSize()
{
    if (ui->checkBoxEnable->isChecked()) {
        bool valid = ui->widgetMinLineBox->checkValid();
        valid = ui->widgetMaxLineBox->checkValid() && valid;
        if (!valid) {
            return false;
        }

        int minWidth = ui->widgetMinLineBox->widthValue();
        int minHeight = ui->widgetMinLineBox->heightValue();
        int maxWidth = ui->widgetMaxLineBox->widthValue();
        int maxHeight = ui->widgetMaxLineBox->heightValue();

        qMsWarning() << minWidth << minHeight << maxWidth << maxHeight;

        if (minWidth > maxWidth || minHeight > maxHeight) {
            ShowMessageBox(GET_TEXT("SMARTEVENT/55010", "Minimum size must be smaller than Maximum size."));
            return false;
        }
    }

    return true;
}

void TabPeopleCountingSettings::clearCopyInfo()
{
    m_copyFlag = 0;
    for (int i = 0; i < MAX_LEN_65; i++) {
        m_peopleCountInfo->copyChn[i] = '0';
        m_settings_info2->copyChn[i] = '0';
    }
}

void TabPeopleCountingSettings::onChannelButtonClicked(int index)
{
    if (!isVisible()) {
        return;
    }
    if (m_action) {
        m_action->clearCache();
    }
    if (m_effectiveTime) {
        m_effectiveTime->clearCache();
    }
    //
    setMultiLineVisisble(true);
    ui->widgetMinLineBox->setValid(true);
    ui->widgetMaxLineBox->setValid(true);

    setCurrentChannel(index);
    m_currentChannel = index;
    ui->commonVideo->playVideo(m_currentChannel);

    ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateFinished);
    ui->pushButtonLineEdit->setButtonState(PushButtonEditState::StateFinished);
    ui->comboBoxAlarmTrigger->setCurrentIndex(0);
    m_isConnected = false;
    m_isSupported = false;
    m_currentLine = 0;
    m_copyFlag = 0;
    m_drawLine->clearAllLine();
    hideMessage();

    //检查通道是否连接
    m_isConnected = LiveView::instance()->isChannelConnected(m_currentChannel);
    if (!m_isConnected) {
        showNotConnectedMessage();
        clearSettings();
        updateEnableState();
        return;
    }
    //showWait();

    //检查鱼眼是否支持
    if (qMsNvr->isFisheye(m_currentChannel)) {
        int fisheyeSupport = waitForCheckFisheeyeSupport();
        if (fisheyeSupport < 0) {
            showNotSupportMessage();
            clearSettings();
            updateEnableState();
            return;
        }
    }

    //检查VCA是否支持
    m_isSupported = (waitForCheckVcaSupport() >= 0);
    if (!m_isSupported) {
        showNotSupportMessage();
        clearSettings();
        updateEnableState();
        return;
    }

    //获取数据
    MsCameraVersion cameraVersion = MsCameraVersion::fromChannel(currentChannel());
    if ((cameraVersion >= MsCameraVersion(8, 3, 9) && qMsNvr->isNT(currentChannel()))
    || cameraVersion >= MsCameraVersion(8, 4)) {
        m_objectSizeType = DrawSceneObjectSize::After_8_0_3_r9;
        ui->widgetMinLineBox->setWidthRange(8, 608);
        ui->widgetMinLineBox->setHeightRange(8, 352);
        ui->widgetMaxLineBox->setWidthRange(8, 608);
        ui->widgetMaxLineBox->setHeightRange(8, 352);
        ui->radioButton_minLayout->setText(GET_TEXT("STATUS/177014", "Minimum Size(%1x%2~%3x%4)").arg(8).arg(8).arg(608).arg(352));
        ui->radioButton_maxLayout->setText(GET_TEXT("STATUS/177015", "Maximum Size(%1x%2~%3x%4)").arg(8).arg(8).arg(608).arg(352));
    } else {
        m_objectSizeType = DrawSceneObjectSize::Before_8_0_3_r9;
        ui->widgetMinLineBox->setWidthRange(1, 320);
        ui->widgetMinLineBox->setHeightRange(1, 240);
        ui->widgetMaxLineBox->setWidthRange(1, 320);
        ui->widgetMaxLineBox->setHeightRange(1, 240);
        ui->radioButton_minLayout->setText(GET_TEXT("SMARTEVENT/55052", "Minimum Size(1x1~%1x%2)").arg(320).arg(240));
        ui->radioButton_maxLayout->setText(GET_TEXT("SMARTEVENT/55053", "Maximum Size(1x1~%1x%2)").arg(320).arg(240));
    }
    // int result = 0;
    // sendMessage(REQUEST_FLAG_GET_VCA_PEOPLE_COUNT, &m_currentChannel, sizeof(int));
    // result = m_eventLoop.exec();
    // if (result < 0) {
    //     m_isConnected = false;
    //     updateEnableState();
    //     showDataError();
    //     return;
    // }
    // sendMessage(REQUEST_FLAG_GET_VAC_SETTINGS2, &m_currentChannel, sizeof(int));
    // result = m_eventLoop.exec();
    // if (result < 0) {
    //     m_isConnected = false;
    //     updateEnableState();
    //     showDataError();
    //     return;
    // }
    // //closeWait();
    // updateEnableState();
}

void TabPeopleCountingSettings::on_pushButton_clear_clicked()
{
    m_drawLine->clearLine(ui->comboBoxLine->currentIntData());
}

void TabPeopleCountingSettings::on_pushButton_clearCount_clicked()
{
    int result = MessageBox::question(this, GET_TEXT("SMARTEVENT/55012", "Are you sure to clear the current count?"));
    if (result == MessageBox::Yes) {
        ms_vca_cleancount_info cleancount_info;
        memset(&cleancount_info, 0, sizeof(cleancount_info));
        cleancount_info.chanid = m_currentChannel;
        cleancount_info.incount = 0;
        cleancount_info.outcount = 0;

        //qDebug() << QString("PeopleCounting::on_pushButton_clearCount_clicked, REQUEST_FLAG_SET_VAC_CLEANCOUNT, channel = %1").arg(channel);
        sendMessage(REQUEST_FLAG_SET_VAC_CLEANCOUNT, (void *)&cleancount_info, sizeof(cleancount_info));
    }
}

void TabPeopleCountingSettings::on_checkBoxEnable_clicked(bool checked)
{
    Q_UNUSED(checked)
    m_peopleCountInfo->lines[m_currentLine].lineEnable = checked;
    m_drawLine->setIsShowLineInfo(m_peopleCountInfo->lines[m_currentLine].showCntEnable & m_peopleCountInfo->lines[m_currentLine].lineEnable, m_currentLine);

    if (!checked) {
        ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateFinished);
        ui->pushButtonLineEdit->setButtonState(PushButtonEditState::StateFinished);
    }
    updateEnableState();
}

void TabPeopleCountingSettings::onSceneObjectSizeChanged(const QRect &rc, MsQtVca::ObjectSizeType sizeType)
{
    switch (sizeType) {
    case MsQtVca::MinSize:
        m_settings_info2->minobject_window_rectangle_x[ObjectType] = rc.x();
        m_settings_info2->minobject_window_rectangle_y[ObjectType] = rc.y();
        m_settings_info2->minobject_window_rectangle_width[ObjectType] = rc.width();
        m_settings_info2->minobject_window_rectangle_height[ObjectType] = rc.height();

        ui->widgetMinLineBox->setWidthValue(rc.width());
        ui->widgetMinLineBox->setHeightValue(rc.height());
        break;
    case MsQtVca::MaxSize:
        m_settings_info2->maxobject_window_rectangle_x[ObjectType] = rc.x();
        m_settings_info2->maxobject_window_rectangle_y[ObjectType] = rc.y();
        m_settings_info2->maxobject_window_rectangle_width[ObjectType] = rc.width();
        m_settings_info2->maxobject_window_rectangle_height[ObjectType] = rc.height();

        ui->widgetMaxLineBox->setWidthValue(rc.width());
        ui->widgetMaxLineBox->setHeightValue(rc.height());
        break;
    default:
        break;
    }
}

void TabPeopleCountingSettings::on_radioButton_minLayout_clicked(bool checked)
{
    if (!checked) {
        ui->radioButton_minLayout->setChecked(true);
        return;
    } else {
        ui->radioButton_maxLayout->setChecked(false);
    }
    m_drawScene->showMinimumObjectSize(m_settings_info2, ObjectType, m_objectSizeType);
}

void TabPeopleCountingSettings::on_radioButton_maxLayout_clicked(bool checked)
{
    if (!checked) {
        ui->radioButton_maxLayout->setChecked(true);
    } else {
        ui->radioButton_minLayout->setChecked(false);
    }
    m_drawScene->showMaximumObjectSize(m_settings_info2, ObjectType, m_objectSizeType);
}

void TabPeopleCountingSettings::onLineEditMinWidthEditingFinished()
{
    int value = ui->widgetMinLineBox->widthValue();
    if (value < 1) {
        return;
    }
    m_settings_info2->minobject_window_rectangle_width[ObjectType] = value;
    if (ui->radioButton_minLayout->isChecked()) {
        m_drawScene->showMinimumObjectSize(m_settings_info2, ObjectType, m_objectSizeType);
    }
}

void TabPeopleCountingSettings::onLineEditMinHeightEditingFinished()
{
    int value = ui->widgetMinLineBox->heightValue();
    if (value < 1) {
        return;
    }
    m_settings_info2->minobject_window_rectangle_height[ObjectType] = value;
    if (ui->radioButton_minLayout->isChecked()) {
        m_drawScene->showMinimumObjectSize(m_settings_info2, ObjectType, m_objectSizeType);
    }
}

void TabPeopleCountingSettings::onLineEditMaxWidthEditingFinished()
{
    int value = ui->widgetMaxLineBox->widthValue();
    if (value < 1) {
        return;
    }
    m_settings_info2->maxobject_window_rectangle_width[ObjectType] = value;
    if (ui->radioButton_maxLayout->isChecked()) {
        m_drawScene->showMaximumObjectSize(m_settings_info2, ObjectType, m_objectSizeType);
    }
}

void TabPeopleCountingSettings::onLineEditMaxHeightEditingFinished()
{
    int value = ui->widgetMaxLineBox->heightValue();
    if (value < 1) {
        return;
    }
    m_settings_info2->maxobject_window_rectangle_height[ObjectType] = value;
    if (ui->radioButton_maxLayout->isChecked()) {
        m_drawScene->showMaximumObjectSize(m_settings_info2, ObjectType, m_objectSizeType);
    }
}

void TabPeopleCountingSettings::on_pushButtonLineEdit_clicked()
{
    switch (ui->pushButtonLineEdit->buttonState()) {
    case PushButtonEditState::StateFinished:
        ui->pushButtonLineEdit->setButtonState(PushButtonEditState::StateEditing);
        ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateFinished);
        break;
    case PushButtonEditState::StateEditing:
        ui->pushButtonLineEdit->setButtonState(PushButtonEditState::StateFinished);
        break;
    }
}

void TabPeopleCountingSettings::on_pushButtonLineEdit_stateSet(PushButtonEditState::State state)
{
    switch (state) {
    case PushButtonEditState::StateFinished:
        m_drawLine->setEnabled(false);
        break;
    case PushButtonEditState::StateEditing:
        m_drawLine->setEnabled(true);
        break;
    }
    updateEnableState();
}

void TabPeopleCountingSettings::on_pushButtonObjectSizeEdit_clicked()
{
    switch (ui->pushButtonObjectSizeEdit->buttonState()) {
    case PushButtonEditState::StateFinished:
        ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateEditing);
        ui->pushButtonLineEdit->setButtonState(PushButtonEditState::StateFinished);
        break;
    case PushButtonEditState::StateEditing:
        ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateFinished);
        break;
    }
}

void TabPeopleCountingSettings::on_pushButtonObjectSizeEdit_stateSet(PushButtonEditState::State state)
{
    switch (state) {
    case PushButtonEditState::StateFinished:
        m_drawLine->show();

        ui->radioButton_minLayout->setChecked(false);
        ui->radioButton_maxLayout->setChecked(false);
        ui->commonVideo->removeDrawScene();
        break;
    case PushButtonEditState::StateEditing:
        m_drawLine->hide();

        ui->radioButton_minLayout->setChecked(true);
        ui->radioButton_maxLayout->setChecked(false);
        ui->commonVideo->showDrawScene(m_drawScene);
        m_drawScene->showMinimumObjectSize(m_settings_info2, ObjectType, m_objectSizeType);
        break;
    }
    updateEnableState();
}

void TabPeopleCountingSettings::on_pushButtonObjectSizeReset_clicked()
{
    int minw,minh,maxw,maxh;
    if (m_objectSizeType == DrawSceneObjectSize::Before_8_0_3_r9) {
        minw = 3;
        minh = 3;
        maxw = 320;
        maxh = 240;
    } else {
        minw = 8;
        minh = 8;
        maxw = 608;
        maxh = 352;
    }
    m_settings_info2->minobject_window_rectangle_x[ObjectType] = 0;
    m_settings_info2->minobject_window_rectangle_y[ObjectType] = 0;
    m_settings_info2->minobject_window_rectangle_width[ObjectType] = minw;
    m_settings_info2->minobject_window_rectangle_height[ObjectType] = minh;
    m_settings_info2->maxobject_window_rectangle_x[ObjectType] = 0;
    m_settings_info2->maxobject_window_rectangle_y[ObjectType] = 0;
    m_settings_info2->maxobject_window_rectangle_width[ObjectType] = maxw;
    m_settings_info2->maxobject_window_rectangle_height[ObjectType] = maxh;

    ui->widgetMinLineBox->setWidthValue(minw);
    ui->widgetMinLineBox->setHeightValue(minh);
    ui->widgetMaxLineBox->setWidthValue(maxw);
    ui->widgetMaxLineBox->setHeightValue(maxh);


    if (ui->radioButton_minLayout->isChecked()) {
        m_drawScene->showMinimumObjectSize(m_settings_info2, ObjectType, m_objectSizeType);
    }
    if (ui->radioButton_maxLayout->isChecked()) {
        m_drawScene->showMaximumObjectSize(m_settings_info2, ObjectType, m_objectSizeType);
    }
}

void TabPeopleCountingSettings::on_pushButtonApply_clicked()
{
    saveData();
}

void TabPeopleCountingSettings::on_pushButtonBack_clicked()
{
    back();
}

void TabPeopleCountingSettings::on_pushButtonCountingInfoEdit_clicked()
{
    if (!m_countingInfo) {
        m_countingInfo = new PeopleCountingInformationEdit(this);
    }
    m_countingInfo->initializeData(m_peopleCountInfo, m_currentLine);
    int result = m_countingInfo->exec();
    if (result == PeopleCountingInformationEdit::Accepted) {
        m_drawLine->setLineCntOsdType(m_peopleCountInfo->osdType);
        m_drawLine->setIsShowLineInfo(m_peopleCountInfo->lines[m_currentLine].showCntEnable & m_peopleCountInfo->lines[m_currentLine].lineEnable, m_currentLine);
    }
}

void TabPeopleCountingSettings::setMultiLineVisisble(bool visible)
{
    if (visible) {
        ui->label_showOSD->hide();
        ui->label_position->hide();
        ui->label_clearCount->hide();
        ui->checkBox_show->hide();
        ui->comboBox_textPos->hide();
        ui->widgetClear->hide();
        ui->checkBoxAlarmTrigger->hide();

        ui->labelLine->show();
        ui->labelDirection->show();
        ui->labelSensitivity->show();
        ui->labelCountingInfomation->show();
        ui->comboBoxLine->show();
        ui->comboBoxDirection->show();
        ui->horizontalSliderSensitivity->show();
        ui->widgetCountingInformation->show();
        ui->comboBoxAlarmTrigger->show();
    } else {
        ui->labelLine->hide();
        ui->labelDirection->hide();
        ui->labelSensitivity->hide();
        ui->labelCountingInfomation->hide();
        ui->comboBoxLine->hide();
        ui->comboBoxDirection->hide();
        ui->horizontalSliderSensitivity->hide();
        ui->widgetCountingInformation->hide();
        ui->comboBoxAlarmTrigger->hide();

        ui->label_showOSD->show();
        ui->label_position->show();
        ui->label_clearCount->show();
        ui->checkBox_show->show();
        ui->comboBox_textPos->show();
        ui->widgetClear->show();
        ui->checkBoxAlarmTrigger->show();
    }
}

void TabPeopleCountingSettings::on_comboBoxLine_indexSet(int index)
{
    ui->pushButtonLineEdit->setButtonState(PushButtonEditState::StateFinished);
    ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateFinished);

    m_currentLine = index;
    m_drawLine->setCurrentLine(m_currentLine);
    ui->checkBoxEnable->setChecked(m_peopleCountInfo->lines[m_currentLine].lineEnable);
    ui->comboBoxDirection->setCurrentIndexFromData(m_peopleCountInfo->lines[m_currentLine].direction);
    updateEnableState();
}

void TabPeopleCountingSettings::on_pushButtonCopy_clicked()
{
    ChannelCopyDialog copy(this);
    copy.setCurrentChannel(m_currentChannel);
    int result = copy.exec();
    if (result == ChannelCopyDialog::Accepted) {
        QList<int> copyList = copy.checkedList(false);
        m_copyFlag = copy.checkedFlags(false);
        for (int i = 0; i < MAX_LEN_65; i++) {
            if (copyList.contains(i)) {
                m_peopleCountInfo->copyChn[i] = '1';
                m_settings_info2->copyChn[i] = '1';
            } else {
                m_peopleCountInfo->copyChn[i] = '0';
                m_settings_info2->copyChn[i] = '0';
            }
        }
        saveData();
    }
}

void TabPeopleCountingSettings::on_comboBoxDirection_indexSet(int index)
{
    m_peopleCountInfo->lines[m_currentLine].direction = static_cast<UInt8>(index);
    m_drawLine->setLineDirection(index);
}

void TabPeopleCountingSettings::updateDynamicDisplay(int type, int channel)
{
    Q_UNUSED(type)
    Q_UNUSED(channel)
}

void TabPeopleCountingSettings::on_comboBoxAlarmTrigger_activated(int index)
{
    Q_UNUSED(index)
    m_peopleCountInfo->triggerType = ui->comboBoxAlarmTrigger->currentIntData();
}

void TabPeopleCountingSettings::on_pushButtonThresholds_clicked()
{
    if (!m_thresholds) {
        m_thresholds = new ThresholdsEdit(this);
    }
    if (m_peopleCountInfo->lineNum == 1) {
        m_thresholds->initializeData(m_peopleCountInfo->lines[0].alarmEnable, m_peopleCountInfo->lines[0].alarmThresholds);
    } else if (m_peopleCountInfo->triggerType == 0) {
        m_thresholds->initializeData(m_peopleCountInfo->totalAlarmEnable, m_peopleCountInfo->totalAlarmThresholds);
    } else {
        m_thresholds->initializeData(m_peopleCountInfo->lines[m_currentLine].alarmEnable, m_peopleCountInfo->lines[m_currentLine].alarmThresholds);
    }
    int result = m_thresholds->exec();
    if (result == ThresholdsEdit::Accepted) {
        if (m_peopleCountInfo->lineNum == 1) {
            m_thresholds->getThresholdsData(m_peopleCountInfo->lines[0].alarmEnable, m_peopleCountInfo->lines[0].alarmThresholds);
        } else if (m_peopleCountInfo->triggerType == 0) {
            m_thresholds->getThresholdsData(m_peopleCountInfo->totalAlarmEnable, m_peopleCountInfo->totalAlarmThresholds);
        } else {
            m_thresholds->getThresholdsData(m_peopleCountInfo->lines[m_currentLine].alarmEnable, m_peopleCountInfo->lines[m_currentLine].alarmThresholds);
        }
    }
}

void TabPeopleCountingSettings::on_pushButtonPeopleCountingSchedule_clicked()
{
    if (!m_effectiveTime) {
        m_effectiveTime = new EffectiveTimePeopleCounting(PeopleCountingSettingsMode, this);
    }
    m_effectiveTime->setSchedule(m_peopleCountInfo->sche.schedule_day);
    m_effectiveTime->showEffectiveTime(m_currentChannel);
    int result = m_effectiveTime->exec();
    if (result == QDialog::Accepted) {
        m_effectiveTime->getSchedule(m_peopleCountInfo->sche.schedule_day);
    }
    ui->pushButtonPeopleCountingSchedule->clearUnderMouse();
}

void TabPeopleCountingSettings::on_pushButtonAction_clicked()
{
    if (!m_action) {
        m_action = new ActionSmartEvent(this);
    }
    m_action->showAction(m_currentChannel, PEOPLE_CNT);
}

void TabPeopleCountingSettings::on_checkBoxAlarmTrigger_clicked()
{
    ui->pushButtonThresholds->setEnabled(ui->checkBoxAlarmTrigger->isChecked());
}
