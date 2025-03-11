#include "LineCrossing.h"
#include "ui_LineCrossing.h"
#include "ActionLineCrossing.h"
#include "DrawSceneObjectSize.h"
#include "EffectiveTimeLineCrossing.h"
#include "FaceData.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "ptzdatamanager.h"
extern "C" {
#include "ptz_public.h"
}
#define DRAW_LINE 0
#define FINISH_LINE 1

const MsQtVca::ObjectVcaType ObjectType = MsQtVca::LineCrossing;

LineCrossing::LineCrossing(QWidget *parent)
    : BaseSmartWidget(parent)
    , ui(new Ui::LineCrossing)
{
    ui->setupUi(this);

    ui->widget_buttonGroup->setCount(qMsNvr->maxChannel());
    ui->widget_buttonGroup->setCurrentIndex(0);
    connect(ui->widget_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onChannelButtonClicked(int)));
    ui->slider_sensitivity->setRange(1, 10);

    m_drawLine = new GraphicsDrawLineCrossing();
    ui->commonVideo->addGraphicsItem(m_drawLine);

    ui->radioButton_minLayout->setAutoExclusive(false);
    ui->radioButton_maxLayout->setAutoExclusive(false);
    m_drawScene = new DrawSceneObjectSize(this);
    connect(m_drawScene, SIGNAL(objectSizeChanged(QRect, MsQtVca::ObjectSizeType)), this, SLOT(onSceneObjectSizeChanged(QRect, MsQtVca::ObjectSizeType)));

    m_settings_info2 = new ms_vca_settings_info2;

    ui->widgetMinLineBox->setWidthRange(1, 320);
    ui->widgetMinLineBox->setHeightRange(1, 240);
    connect(ui->widgetMinLineBox, SIGNAL(widthChange()), this, SLOT(onLineEditMinWidthEditingFinished()));
    connect(ui->widgetMinLineBox, SIGNAL(heightChange()), this, SLOT(onLineEditMinHeightEditingFinished()));

    ui->widgetMaxLineBox->setWidthRange(1, 320);
    ui->widgetMaxLineBox->setHeightRange(1, 240);
    connect(ui->widgetMaxLineBox, SIGNAL(widthChange()), this, SLOT(onLineEditMaxWidthEditingFinished()));
    connect(ui->widgetMaxLineBox, SIGNAL(heightChange()), this, SLOT(onLineEditMaxHeightEditingFinished()));

    ui->comboBoxEffectiveRegion->beginEdit();
    ui->comboBoxEffectiveRegion->clear();
    ui->comboBoxEffectiveRegion->addItem(GET_TEXT("SMARTEVENT/146002", "Normal"), 0);
    ui->comboBoxEffectiveRegion->addItem(GET_TEXT("SMARTEVENT/146003", "Advanced"), 1);
    ui->comboBoxEffectiveRegion->endEdit();

    ui->comboBoxEffectiveWithPreset->beginEdit();
    ui->comboBoxEffectiveWithPreset->clear();
    ui->comboBoxEffectiveWithPreset->addItem(GET_TEXT("PTZDIALOG/21015", "preset") + QString(" %1").arg(1));
    ui->comboBoxEffectiveWithPreset->addItem(GET_TEXT("PTZDIALOG/21015", "preset") + QString(" %1").arg(2));
    ui->comboBoxEffectiveWithPreset->addItem(GET_TEXT("PTZDIALOG/21015", "preset") + QString(" %1").arg(3));
    ui->comboBoxEffectiveWithPreset->addItem(GET_TEXT("PTZDIALOG/21015", "preset") + QString(" %1").arg(4));
    ui->comboBoxEffectiveWithPreset->endEdit();

    clearSettings();
    onLanguageChanged();
}

LineCrossing::~LineCrossing()
{
    if (m_settings_info2) {
        delete m_settings_info2;
        m_settings_info2 = nullptr;
    }
    delete ui;
}

void LineCrossing::initializeData(int channel)
{
    //这里会调用onChannelButtonClicked(channel)槽函数
    ui->widget_buttonGroup->setCurrentIndex(channel);
}

void LineCrossing::saveData()
{
    if (!checkObjectSize()) {
        clearCopyInfo();
        return;
    }
    //showWait();

    gFaceData.getFaceConfig(currentChannel());
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

    int channel = currentChannel();

    if (m_effectiveTime) {
        m_effectiveTime->saveEffectiveTime();
    }
    if (m_action) {
        m_action->saveAction();
    }
    REQ_UPDATE_CHN req = { 0 };
    ms_set_bit(&req.chnMask, channel, 1);
    sendMessageOnly(REQUEST_FLAG_SET_SMART_EVENT, &req, sizeof(req));

    saveLineCross(ui->comboBoxLine->currentIndex());
    if (m_cameraVersion >= MsCameraVersion(7, 75)) {
        m_linecrossing_info2.chanid = channel;
        if (m_linecrossing_info2.lineType != REGION_PRESET) {
            m_drawLine->getLineCrossInfo(&m_linecrossing_info2);
        } else {
            m_drawLine->getLineCrossInfo(&m_linecrossing_info2, m_linecrossing_info2.lineScene);
        }

        m_linecrossing_info2.sensitivity = ui->slider_sensitivity->value();
        QString text = "\nREQUEST_FLAG_SET_IPC_VCA_LINECROSSING2\n";
        text += QString("channel: %1, sensitivity: %2\n").arg(currentChannel()).arg(m_linecrossing_info2.sensitivity);
        for (int i = 0; i < 4; ++i) {
            const linecrossing_info2 &line = m_linecrossing_info2.line[i];
            text += QString("line: %1, enable: %2, direction: %3, object: %4\n")
                        .arg(i + 1)
                        .arg(line.enable)
                        .arg(line.direction)
                        .arg(line.objtype);
        }
        qMsDebug() << qPrintable(text);

        sendMessage(REQUEST_FLAG_SET_IPC_VCA_LINECROSSING2, (void *)&m_linecrossing_info2, sizeof(ms_linecrossing_info2));
    } else {
        m_linecrossing_info.chanid = channel;
        m_linecrossing_info.enable = ui->checkBoxEnable->isChecked();
        m_drawLine->getLineCrossInfo(&m_linecrossing_info);

        int objtype = 0;
        if (ui->checkBoxHuman->isChecked()) {
            objtype |= DETEC_HUMAN;
        }
        if (ui->checkBoxVehicle->isChecked()) {
            objtype |= DETEC_VEHICLE;
        }
        m_linecrossing_info.objtype = static_cast<DETEC_OBJ_EN>(objtype);

        sendMessage(REQUEST_FLAG_SET_VCA_LINECROSSING, (void *)&m_linecrossing_info, sizeof(ms_linecrossing_info));
    }
    sendMessage(REQUEST_FLAG_SET_VAC_SETTINGS2, m_settings_info2, sizeof(ms_vca_settings_info2));
    clearCopyInfo();
    //显示等待，避免频繁点击
    QEventLoop eventLoop;
    QTimer::singleShot(500, &eventLoop, SLOT(quit()));
    eventLoop.exec();
    //closeWait();
}

void LineCrossing::copyData()
{
    ChannelCopyDialog copy(this);
    copy.setCurrentChannel(currentChannel());
    int result = copy.exec();
    if (result == ChannelCopyDialog::Accepted) {
        QList<int> copyList = copy.checkedList(false);
        quint64 copyFlags = copy.checkedFlags(false);
        for (int i = 0; i < MAX_LEN_65; i++) {
            if (copyList.contains(i)) {
                m_linecrossing_info.copyChn[i] = '1';
                m_linecrossing_info2.copyChn[i] = '1';
                m_settings_info2->copyChn[i] = '1';
            } else {
                m_linecrossing_info.copyChn[i] = '0';
                m_linecrossing_info2.copyChn[i] = '0';
                m_settings_info2->copyChn[i] = '0';
            }
        }
        saveData();
        copy_smart_event_action(LINECROSS, currentChannel(), copyFlags, 1);
    }
}

void LineCrossing::clearCopyInfo()
{
    for (int i = 0; i < MAX_LEN_65; i++) {
        m_linecrossing_info.copyChn[i] = '0';
        m_linecrossing_info2.copyChn[i] = '0';
        m_settings_info2->copyChn[i] = '0';
    }
}

void LineCrossing::processMessage(MessageReceive *message)
{
    dealGlobalMessage(message);
    //
    switch (message->type()) {
    case RESPONSE_FLAG_GET_VCA_LINECROSSING:
        ON_RESPONSE_FLAG_GET_VCA_LINECROSSING(message);
        break;
    case RESPONSE_FLAG_SET_VCA_LINECROSSING:
        ON_RESPONSE_FLAG_SET_VCA_LINECROSSING(message);
        break;
    case RESPONSE_FLAG_GET_IPC_VCA_LINECROSSING2:
        ON_RESPONSE_FLAG_GET_IPC_VCA_LINECROSSING2(message);
        break;
    case RESPONSE_FLAG_SET_IPC_VCA_LINECROSSING2:
        ON_RESPONSE_FLAG_SET_IPC_VCA_LINECROSSING2(message);
        break;
    case RESPONSE_FLAG_GET_VAC_SETTINGS2:
        ON_RESPONSE_FLAG_GET_VAC_SETTINGS2(message);
        break;
    case RESPONSE_FLAG_SET_VAC_SETTINGS2:
        ON_RESPONSE_FLAG_SET_VAC_SETTINGS2(message);
    default:
        break;
    }
}

void LineCrossing::ON_RESPONSE_FLAG_GET_VCA_LINECROSSING(MessageReceive *message)
{
    if (message->isNull()) {
        qMsWarning() << QString("data is null.");
        m_eventLoop.exit(-1);
        return;
    }
    ms_linecrossing_info *linecrossing_info = static_cast<ms_linecrossing_info *>(message->data);
    if (linecrossing_info->chanid != currentChannel()) {
        return;
    }
    if (!isSupportDetectionObject(currentChannel())) {
        ui->labelDetectionObject->hide();
        ui->widgetDetectionObject->hide();
    }
    memcpy(&m_linecrossing_info, linecrossing_info, sizeof(ms_linecrossing_info));
    ui->checkBoxEnable->setChecked(m_linecrossing_info.enable);
    m_drawLine->setLineCrossInfo(&m_linecrossing_info);
    m_drawLine->setCurrentLine(0);
    ui->comboBoxLine->setCurrentIndex(0);
    ui->comboBox_direction->setCurrentIndex(m_drawLine->lineDirection(0));

    ui->checkBoxHuman->setChecked(m_linecrossing_info.objtype & DETEC_HUMAN);
    ui->checkBoxVehicle->setChecked(m_linecrossing_info.objtype & DETEC_VEHICLE);
    on_checkBoxEnable_clicked(m_linecrossing_info.enable);
    m_eventLoop.exit();
}

void LineCrossing::ON_RESPONSE_FLAG_GET_IPC_VCA_LINECROSSING2(MessageReceive *message)
{
    if (message->isNull()) {
        qMsWarning() << QString("data is null.");
        m_eventLoop.exit(-1);
        return;
    }
    ms_linecrossing_info2 *info2 = static_cast<ms_linecrossing_info2 *>(message->data);
    if (info2->chanid != currentChannel()) {
        return;
    }
    if (isSupportDetectionObject(currentChannel())) {
        ui->labelDetectionObject->show();
        ui->widgetDetectionObject->show();
    } else {
        ui->labelDetectionObject->hide();
        ui->widgetDetectionObject->hide();
    }
    memcpy(&m_linecrossing_info2, info2, sizeof(ms_linecrossing_info2));

    QString text = "\nRESPONSE_FLAG_GET_IPC_VCA_LINECROSSING2\n";
    text += QString("channel: %1, sensitivity: %2\n").arg(currentChannel()).arg(m_linecrossing_info2.sensitivity);
    for (int i = 0; i < 4; ++i) {
        const linecrossing_info2 &line = m_linecrossing_info2.line[i];
        text += QString("line: %1, enable: %2, direction: %3, object: %4\n")
                    .arg(i + 1)
                    .arg(line.enable)
                    .arg(line.direction)
                    .arg(line.objtype);
    }
    qMsDebug() << qPrintable(text);
    if (m_linecrossing_info2.lineType != REGION_PRESET) {
        m_drawLine->setLineCrossInfo(&m_linecrossing_info2);
    } else {
        m_drawLine->setLineCrossInfo(&m_linecrossing_info2, m_linecrossing_info2.lineScene);
        setEffectiveRegionVisible(true);
        //ptz preset
        gPtzDataManager->beginGetData(currentChannel());
        gPtzDataManager->waitForGetPtzOvfInfo();
        resp_ptz_ovf_info *ptz_ovf_info = gPtzDataManager->ptzOvfInfo();
        for (int i = 0; i < 4; i++) {
            const resp_ptz_preset &preset = ptz_ovf_info->preset[i];
            if (m_linecrossing_info2.presetExist[i + 1]) {
                ui->comboBoxEffectiveWithPreset->setItemData(i, preset.name);
            }
        }
        m_isFirstChoiceAdvanced = false;
        if (m_linecrossing_info2.lineScene == SCENE_NORMAL) {
            ui->comboBoxEffectiveRegion->setCurrentIndexFromData(0);
            m_isFirstChoiceAdvanced = true;

        } else {
            m_ptzNeedToCall = false;
            ui->comboBoxEffectiveWithPreset->setCurrentIndex(m_linecrossing_info2.lineScene - 1);
            ui->comboBoxEffectiveRegion->setCurrentIndexFromData(1);
        }
    }
    m_drawLine->setCurrentLine(0);
    ui->comboBox_direction->setCurrentIndex(m_drawLine->lineDirection(0));

    ui->slider_sensitivity->setValue(m_linecrossing_info2.sensitivity);
    ui->comboBoxLine->setCurrentIndex(0);
    m_eventLoop.exit();
}

void LineCrossing::ON_RESPONSE_FLAG_SET_VCA_LINECROSSING(MessageReceive *message)
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

void LineCrossing::ON_RESPONSE_FLAG_SET_IPC_VCA_LINECROSSING2(MessageReceive *message)
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

void LineCrossing::onLanguageChanged()
{
    ui->label_lineCrossing->setText(GET_TEXT("SMARTEVENT/55005", "Line Crossing"));
    ui->label_line->setText(GET_TEXT("SMARTEVENT/55017", "Line"));
    ui->label_direction->setText(GET_TEXT("SMARTEVENT/55018", "Direction"));
    ui->label_drawLine->setText(GET_TEXT("SMARTEVENT/119000", "Line Edit"));
    ui->labelObjectSizeLimits->setText(GET_TEXT("SMARTEVENT/55063", "Object Size Limits"));
    ui->pushButtonObjectSizeReset->setText(GET_TEXT("COMMON/1057", "Reset"));
    ui->label_effectiveTime->setText(GET_TEXT("SMARTEVENT/55015", "Effective Time"));
    ui->label_action->setText(GET_TEXT("VIDEOLOSS/50002", "Action"));
    ui->labelDetectionObject->setText(GET_TEXT("TARGETMODE/103200", "Detection Object"));
    ui->checkBoxHuman->setText(GET_TEXT("TARGETMODE/103201", "Human"));
    ui->checkBoxVehicle->setText(GET_TEXT("TARGETMODE/103202", "Vehicle"));
    ui->pushButton_clear->setText(GET_TEXT("CHANNELMANAGE/30023", "Delete"));
    ui->pushButton_editAction->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->pushButton_editTime->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->label_sensitivity->setText(GET_TEXT("MOTION/51003", "Sensitivity"));
    ui->checkBoxEnable->setText(GET_TEXT("COMMON/1009", "Enable"));

    ui->labelEffectiveRegion->setText(GET_TEXT("SMARTEVENT/146001", "Effective Region"));
    ui->labelEffectiveWithPresets->setText(GET_TEXT("SMARTEVENT/146007", "Effective with presets"));
    ui->pushButtonEffectiveWithPreset->setText(GET_TEXT("SMARTEVENT/146008", "call"));
}

void LineCrossing::hideEvent(QHideEvent *)
{
    m_drawLine->clearAllLine();
    ui->comboBox_direction->setCurrentIndex(0);
}
void LineCrossing::saveLineCross(int index)
{
    m_linecrossing_info2.line[index].enable = ui->checkBoxEnable->isChecked();
    //
    int objtype = 0;
    if (ui->checkBoxHuman->isChecked()) {
        objtype |= DETEC_HUMAN;
    }
    if (ui->checkBoxVehicle->isChecked()) {
        objtype |= DETEC_VEHICLE;
    }
    m_linecrossing_info2.line[index].objtype = static_cast<DETEC_OBJ_EN>(objtype);
}

bool LineCrossing::checkObjectSize()
{
    if (ui->checkBoxEnable->isEnabled() && ui->checkBoxEnable->isChecked()) {
        bool valid = ui->widgetMinLineBox->checkValid();
        valid = ui->widgetMaxLineBox->checkValid() && valid;
        if (!valid) {
            return false;
        }

        int minWidth = ui->widgetMinLineBox->widthValue();
        int minHeight = ui->widgetMinLineBox->heightValue();
        int maxWidth = ui->widgetMaxLineBox->widthValue();
        int maxHeight = ui->widgetMaxLineBox->heightValue();
        if (minWidth > maxWidth || minHeight > maxHeight) {
            ShowMessageBox(GET_TEXT("SMARTEVENT/55010", "Minimum size must be smaller than Maximum size."));
            return false;
        }
    }

    return true;
}

void LineCrossing::onChannelButtonClicked(int index)
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

    ui->labelDetectionObject->show();
    ui->widgetDetectionObject->show();
    ui->widgetMinLineBox->setValid(true);
    ui->widgetMaxLineBox->setValid(true);
    m_drawLine->clearAllLine();

    ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateFinished);
    ui->pushButtonLineEdit->setButtonState(PushButtonEditState::StateFinished);
    m_isConnected = false;
    m_isSupported = false;
    //ptz
    setEffectiveRegionVisible(false);

    emit hideMessage();
    //    resetUi();
    //
    int channel = index;
    setCurrentChannel(channel);
    m_currentLineIndex = -1;
    ui->commonVideo->playVideo(channel);
    ui->commonVideo->setBanOnBack(true);
    //检查通道是否连接
    m_isConnected = isChannelEnable(channel);
    if (!m_isConnected) {
        showNotConnectedMessage();
        clearSettings();
        updateEnableState();
        return;
    }
    //showWait();
    //检查鱼眼是否支持
    if (qMsNvr->isFisheye(channel)) {
        int fisheyeSupport = waitForCheckFisheeyeSupport(channel);
        if (fisheyeSupport < 0) {
            showNotSupportMessage();
            clearSettings();
            updateEnableState();
            return;
        }
    }
    //检查VCA是否支持
    m_isSupported = (waitForCheckVcaSupport(channel) >= 0);
    if (!m_isSupported) {
        showNotSupportMessage();
        clearSettings();
        updateEnableState();
        return;
    }
    //获取数据
    //int result = 0;
    m_cameraVersion = MsCameraVersion::fromChannel(channel);
    if ((m_cameraVersion >= MsCameraVersion(8, 3, 9) && qMsNvr->isNT(currentChannel()))
    || m_cameraVersion >= MsCameraVersion(8, 4)) {
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
    if (m_cameraVersion >= MsCameraVersion(7, 75)) {
        ui->label_sensitivity->show();
        ui->slider_sensitivity->show();
        sendMessage(REQUEST_FLAG_GET_IPC_VCA_LINECROSSING2, (void *)&channel, sizeof(int));
    } else {
        ui->label_sensitivity->hide();
        ui->slider_sensitivity->hide();
        sendMessage(REQUEST_FLAG_GET_VCA_LINECROSSING, (void *)&channel, sizeof(int));
    }
    // result = m_eventLoop.exec();
    // if (result < 0) {
    //     m_isConnected = false;
    //     updateEnableState();
    //     showDataError();
    //     return;
    // }
    // sendMessage(REQUEST_FLAG_GET_VAC_SETTINGS2, (void *)&channel, sizeof(int));
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

void LineCrossing::on_comboBoxLine_indexSet(int index)
{
    qMsDebug() << QString("index: %1, current index: %2").arg(index).arg(m_currentLineIndex);
    if (m_currentLineIndex != -1) {
        saveLineCross(m_currentLineIndex);
    }
    m_drawLine->setCurrentLine(index);
    ui->pushButtonLineEdit->setButtonState(PushButtonEditState::StateFinished);
    ui->comboBox_direction->setCurrentIndex(m_drawLine->lineDirection(index));
    if (m_cameraVersion >= MsCameraVersion(7, 75)) {
        const linecrossing_info2 &line2 = m_linecrossing_info2.line[index];
        ui->checkBoxEnable->setChecked(line2.enable);
        on_checkBoxEnable_clicked(line2.enable);
        ui->checkBoxHuman->setChecked(line2.objtype & DETEC_HUMAN);
        ui->checkBoxVehicle->setChecked(line2.objtype & DETEC_VEHICLE);
    }
    m_currentLineIndex = index;
}

void LineCrossing::on_comboBox_direction_currentIndexChanged(int index)
{
    m_drawLine->setLineDirection(index);
}

void LineCrossing::on_pushButton_clear_clicked()
{
    m_drawLine->clearLine(ui->comboBoxLine->currentIndex());
}

void LineCrossing::on_pushButton_editTime_clicked()
{
    if (!m_effectiveTime) {
        m_effectiveTime = new EffectiveTimeLineCrossing(this);
    }
    m_effectiveTime->showEffectiveTime(currentChannel(), LINECROSS);
}

void LineCrossing::on_pushButton_editAction_clicked()
{
    if (!m_action) {
        m_action = new ActionLineCrossing(this);
    }
    m_action->showAction(currentChannel(), LINECROSS);
}

void LineCrossing::ON_RESPONSE_FLAG_GET_VAC_SETTINGS2(MessageReceive *message)
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

void LineCrossing::ON_RESPONSE_FLAG_SET_VAC_SETTINGS2(MessageReceive *message)
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

void LineCrossing::clearSettings()
{
    m_drawLine->clearAllLine();
    ui->checkBoxEnable->setChecked(false);
    ui->checkBoxHuman->setChecked(false);
    ui->checkBoxVehicle->setChecked(false);
}

void LineCrossing::updateEnableState()
{
    ui->comboBoxLine->setEnabled(m_isConnected && m_isSupported);
    ui->checkBoxEnable->setEnabled(m_isConnected && m_isSupported);
    bool isEnable = ui->checkBoxEnable->isEnabled() && ui->checkBoxEnable->isChecked();
    ui->pushButtonObjectSizeEdit->setEnabled(isEnable);
    ui->pushButtonObjectSizeReset->setEnabled(isEnable);
    ui->pushButtonObjectSizeReset->setVisible(ui->pushButtonObjectSizeReset->isEnabled() && ui->pushButtonObjectSizeEdit->buttonState() == PushButtonEditState::StateEditing);

    ui->slider_sensitivity->setEnabled(isEnable);
    ui->checkBoxHuman->setEnabled(isEnable);
    ui->checkBoxVehicle->setEnabled(isEnable);
    ui->pushButton_editTime->setEnabled(isEnable);
    ui->pushButton_editAction->setEnabled(isEnable);
    ui->widgetSize->setEnabled(isEnable
                               && ui->pushButtonObjectSizeEdit->buttonState() == PushButtonEditState::StateEditing);
    setApplyButtonEnable(ui->checkBoxEnable->isEnabled());

    ui->pushButtonLineEdit->setEnabled(isEnable);
    ui->pushButton_clear->setEnabled(isEnable);
    ui->pushButton_clear->setVisible(ui->pushButton_clear->isEnabled() && ui->pushButtonLineEdit->buttonState() == PushButtonEditState::StateEditing);

    ui->comboBox_direction->setEnabled(isEnable);

    ui->commonVideo->setDrawViewVisible(ui->pushButtonObjectSizeEdit->buttonState() == PushButtonEditState::StateEditing);
}

void LineCrossing::setEffectiveRegionVisible(bool visible)
{
    if (visible) {
        ui->labelEffectiveRegion->show();
        ui->comboBoxEffectiveRegion->show();
    } else {
        ui->labelEffectiveRegion->hide();
        ui->labelEffectiveWithPresets->hide();
        ui->comboBoxEffectiveRegion->hide();
        ui->widgetPresets->hide();
    }
}

void LineCrossing::on_checkBoxEnable_clicked(bool checked)
{
    Q_UNUSED(checked)
    if (!checked) {
        ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateFinished);
        ui->pushButtonLineEdit->setButtonState(PushButtonEditState::StateFinished);
    }
    updateEnableState();
}

void LineCrossing::onLineEditMinWidthEditingFinished()
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

void LineCrossing::onLineEditMinHeightEditingFinished()
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

void LineCrossing::onLineEditMaxWidthEditingFinished()
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

void LineCrossing::onLineEditMaxHeightEditingFinished()
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

void LineCrossing::onSceneObjectSizeChanged(const QRect &rc, MsQtVca::ObjectSizeType sizeType)
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

void LineCrossing::on_radioButton_minLayout_clicked(bool checked)
{
    if (!checked) {
        ui->radioButton_minLayout->setChecked(true);
        return;
    } else {
        ui->radioButton_maxLayout->setChecked(false);
    }
    m_drawScene->showMinimumObjectSize(m_settings_info2, ObjectType, m_objectSizeType);
}

void LineCrossing::on_radioButton_maxLayout_clicked(bool checked)
{
    if (!checked) {
        ui->radioButton_maxLayout->setChecked(true);
    } else {
        ui->radioButton_minLayout->setChecked(false);
    }
    m_drawScene->showMaximumObjectSize(m_settings_info2, ObjectType, m_objectSizeType);
}

void LineCrossing::on_pushButtonLineEdit_clicked()
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

void LineCrossing::on_pushButtonLineEdit_stateSet(PushButtonEditState::State state)
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

void LineCrossing::on_pushButtonObjectSizeEdit_clicked()
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

void LineCrossing::on_pushButtonObjectSizeEdit_stateSet(PushButtonEditState::State state)
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

void LineCrossing::on_pushButtonObjectSizeReset_clicked()
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

void LineCrossing::on_comboBoxEffectiveRegion_indexSet(int index)
{
    if (index == 1 && !m_linecrossing_info2.presetExist[ui->comboBoxEffectiveWithPreset->currentIndex() + 1]) {
        ui->labelEffectiveWithPresets->show();
        ui->widgetPresets->show();
        MessageBox::information(this, GET_TEXT("SMARTEVENT/146006", "Please set up the preset first."));
        if (m_linecrossing_info2.lineScene == SCENE_NORMAL) {
            ui->comboBoxEffectiveRegion->setCurrentIndexFromData(0);
        }
        return;
    }
    if (m_isFirstChoiceAdvanced && index == 1) {
        m_isFirstChoiceAdvanced = false;
        const int &result = MessageBox::question(this, GET_TEXT("SMARTEVENT/146004", "The camera will jump to preset 1, continue?"));
        if (result == MessageBox::Yes) {
            ui->comboBoxEffectiveWithPreset->setCurrentIndex(0);
        } else {
            ui->comboBoxEffectiveRegion->setCurrentIndexFromData(0);
            return;
        }
    }

    m_drawLine->getLineCrossInfo(&m_linecrossing_info2, m_linecrossing_info2.lineScene);
    if (index == 0) {
        m_linecrossing_info2.lineScene = SCENE_NORMAL;
        ui->labelEffectiveWithPresets->hide();
        ui->widgetPresets->hide();
    } else {
        m_linecrossing_info2.lineScene = static_cast<SMART_REGION_SCENE>(ui->comboBoxEffectiveWithPreset->currentIndex() + 1);
        ui->labelEffectiveWithPresets->show();
        ui->widgetPresets->show();
        if (!m_linecrossing_info2.presetExist[m_linecrossing_info2.lineScene]) {
            MessageBox::information(this, GET_TEXT("SMARTEVENT/146006", "Please set up the preset first."));
            ui->comboBoxEffectiveWithPreset->setCurrentIndex(m_linecrossing_info2.lineScene - 1);
            return;
        }
        if (m_ptzNeedToCall) {
            gPtzDataManager->sendPresetControl(PTZ_PRESET_GOTO, m_linecrossing_info2.lineScene, 5, ui->comboBoxEffectiveWithPreset->currentData().toString());
        }
        m_ptzNeedToCall = true;
    }
    m_drawLine->clearAllLine();
    m_drawLine->setLineCrossInfo(&m_linecrossing_info2, m_linecrossing_info2.lineScene);
    m_drawLine->setCurrentLine(m_currentLineIndex);
    ui->comboBox_direction->setCurrentIndex(m_drawLine->lineDirection(m_currentLineIndex));
}

void LineCrossing::on_comboBoxEffectiveWithPreset_activated(int index)
{
    if (!m_linecrossing_info2.presetExist[index + 1]) {
        MessageBox::information(this, GET_TEXT("SMARTEVENT/146006", "Please set up the preset first."));
        ui->comboBoxEffectiveWithPreset->setCurrentIndex(m_linecrossing_info2.lineScene - 1);
        return;
    }
    m_drawLine->getLineCrossInfo(&m_linecrossing_info2, m_linecrossing_info2.lineScene);
    m_linecrossing_info2.lineScene = static_cast<SMART_REGION_SCENE>(ui->comboBoxEffectiveWithPreset->currentIndex() + 1);
    m_drawLine->clearAllLine();
    m_drawLine->setLineCrossInfo(&m_linecrossing_info2, m_linecrossing_info2.lineScene);
    m_drawLine->setCurrentLine(m_currentLineIndex);
    ui->comboBox_direction->setCurrentIndex(m_drawLine->lineDirection(m_currentLineIndex));

    gPtzDataManager->sendPresetControl(PTZ_PRESET_GOTO, index + 1, 5, ui->comboBoxEffectiveWithPreset->currentData().toString());
}

void LineCrossing::on_pushButtonEffectiveWithPreset_clicked()
{
    int index = ui->comboBoxEffectiveWithPreset->currentIndex();
    gPtzDataManager->sendPresetControl(PTZ_PRESET_GOTO, index + 1, 5, ui->comboBoxEffectiveWithPreset->currentData().toString());
}
