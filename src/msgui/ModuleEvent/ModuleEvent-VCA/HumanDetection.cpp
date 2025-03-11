#include "HumanDetection.h"
#include "ui_HumanDetection.h"
#include "ActionVCA.h"
#include "DrawSceneObjectSize.h"
#include "EffectiveTimeVCA.h"
#include "FaceData.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "VideoTrack.h"
#include "centralmessage.h"
const MsQtVca::ObjectVcaType ObjectType = MsQtVca::HumanDetection;

HumanDetection::HumanDetection(QWidget *parent)
    : BaseSmartWidget(parent)
    , ui(new Ui::HumanDetection)
{
    ui->setupUi(this);

    ui->widget_buttonGroup->setCount(qMsNvr->maxChannel());
    ui->widget_buttonGroup->setCurrentIndex(0);
    connect(ui->widget_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onChannelButtonClicked(int)));

    m_videoDraw = new VideoTrack(this);
    ui->commonVideo->showDrawWidget(m_videoDraw);
    m_auto_tracking = new ms_auto_tracking;
    ui->radioButton_minLayout->setAutoExclusive(false);
    ui->radioButton_maxLayout->setAutoExclusive(false);
    m_drawScene = new DrawSceneObjectSize(this);
    connect(m_drawScene, SIGNAL(objectSizeChanged(QRect, MsQtVca::ObjectSizeType)), this, SLOT(onSceneObjectSizeChanged(QRect, MsQtVca::ObjectSizeType)));

    m_event_info = new ms_smart_event_info;
    m_settings_info2 = new ms_vca_settings_info2;

    ui->widgetMinLineBox->setWidthRange(1, 320);
    ui->widgetMinLineBox->setHeightRange(1, 240);
    connect(ui->widgetMinLineBox, SIGNAL(widthChange()), this, SLOT(onLineEditMinWidthEditingFinished()));
    connect(ui->widgetMinLineBox, SIGNAL(heightChange()), this, SLOT(onLineEditMinHeightEditingFinished()));

    ui->widgetMaxLineBox->setWidthRange(1, 320);
    ui->widgetMaxLineBox->setHeightRange(1, 240);
    connect(ui->widgetMaxLineBox, SIGNAL(widthChange()), this, SLOT(onLineEditMaxWidthEditingFinished()));
    connect(ui->widgetMaxLineBox, SIGNAL(heightChange()), this, SLOT(onLineEditMaxHeightEditingFinished()));

    clearSettings();
    onLanguageChanged();
}

HumanDetection::~HumanDetection()
{
    if (m_auto_tracking) {
        delete m_auto_tracking;
        m_auto_tracking = nullptr;
    }
    if (m_event_info) {
        delete m_event_info;
        m_event_info = nullptr;
    }
    if (m_settings_info2) {
        delete m_settings_info2;
        m_settings_info2 = nullptr;
    }
    delete ui;
}

void HumanDetection::initializeData(int channel)
{
    ui->widget_buttonGroup->setCurrentIndex(channel);
}

void HumanDetection::saveData()
{
    if (!checkObjectSize()) {
        clearCopyInfo();
        return;
    }
    return;
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

    int channel = currentChannel();

    if (m_effectiveTime) {
        m_effectiveTime->saveEffectiveTime();
    }
    if (m_action) {
        m_action->saveAction();
    }
    REQ_UPDATE_CHN req = {0};
    ms_set_bit(&req.chnMask, channel, 1);
    sendMessageOnly(REQUEST_FLAG_SET_SMART_EVENT, &req, sizeof(req));

    //和ptz auto tracking互斥
    if (ui->checkBoxEnable->isChecked()) {
        sendMessage(REQUEST_FLAG_GET_AUTO_TRACKING, &channel, sizeof(int));
        m_eventLoop.exec();
        if (m_auto_tracking->enable) {
            //closeWait();
            MessageBox::Result result = MessageBox::question(this, GET_TEXT("SMARTEVENT/55065", "Auto Tracking will be disabled, confirm to continue?"));
            if (result == MessageBox::Yes) {
                //showWait();
                m_auto_tracking->enable = false;
                sendMessage(REQUEST_FLAG_SET_AUTO_TRACKING, m_auto_tracking, sizeof(ms_auto_tracking));
                m_eventLoop.exec();
            } else {
                clearCopyInfo();
                return;
            }
        }
    }
    m_event_info->enable = ui->checkBoxEnable->isChecked();
    m_event_info->show_tracks_enable = ui->checkBox_tracks->isChecked();
    sendMessage(REQUEST_FLAG_SET_VCA_HUMANDETECTION, m_event_info, sizeof(ms_smart_event_info));
    m_eventLoop.exec();
    //
    sendMessage(REQUEST_FLAG_SET_VAC_SETTINGS2, m_settings_info2, sizeof(ms_vca_settings_info2));
    m_eventLoop.exec();
    clearCopyInfo();
    //closeWait();
}

void HumanDetection::copyData()
{
    ChannelCopyDialog copy(this);
    copy.setCurrentChannel(currentChannel());
    int result = copy.exec();
    if (result == ChannelCopyDialog::Accepted) {
        QList<int> copyList = copy.checkedList(false);
        quint64 copyFlags = copy.checkedFlags(false);
        for (int i = 0; i < MAX_LEN_65; i++) {
            if (copyList.contains(i)) {
                m_event_info->copyChn[i] = '1';
                m_settings_info2->copyChn[i] = '1';
            } else {
                m_event_info->copyChn[i] = '0';
                m_settings_info2->copyChn[i] = '0';
            }
        }
        saveData();
        copy_smart_event_action(HUMAN, currentChannel(), copyFlags, 1);
    }
}

void HumanDetection::clearCopyInfo()
{
    for (int i = 0; i < MAX_LEN_65; i++) {
        m_event_info->copyChn[i] = '0';
        m_settings_info2->copyChn[i] = '0';
    }
}

void HumanDetection::processMessage(MessageReceive *message)
{
    dealGlobalMessage(message);
    //
    switch (message->type()) {
    case RESPONSE_FLAG_GET_VCA_HUMANDETECTION:
        ON_RESPONE_FLAG_GET_VCA_HUMANDETECTION(message);
        break;
    case RESPONSE_FLAG_SET_VCA_HUMANDETECTION:
        ON_RESPONE_FLAG_SET_VCA_HUMANDETECTION(message);
        break;
    case RESPONSE_FLAG_GET_AUTO_TRACKING:
        ON_RESPONSE_FLAG_GET_AUTO_TRACKING(message);
        break;
    case RESPONSE_FLAG_SET_AUTO_TRACKING:
        ON_RESPONSE_FLAG_SET_AUTO_TRACKING(message);
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

void HumanDetection::ON_RESPONE_FLAG_GET_VCA_HUMANDETECTION(MessageReceive *message)
{
    if (message->isNull()) {
        qMsWarning() << QString("data is null.");
        m_eventLoop.exit(-1);
        return;
    }
    ms_smart_event_info *info = (ms_smart_event_info *)message->data;
    memcpy(m_event_info, info, sizeof(ms_smart_event_info));
    ui->checkBoxEnable->setChecked(m_event_info->enable);

    ui->checkBox_tracks->setChecked(m_event_info->show_tracks_enable);
    on_checkBox_tracks_clicked(m_event_info->show_tracks_enable);

    m_eventLoop.exit();
}

void HumanDetection::ON_RESPONE_FLAG_SET_VCA_HUMANDETECTION(MessageReceive *message)
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

void HumanDetection::ON_RESPONSE_FLAG_GET_AUTO_TRACKING(MessageReceive *message)
{
    ms_auto_tracking *auto_tracking = (ms_auto_tracking *)message->data;
    memset(m_auto_tracking, 0, sizeof(ms_auto_tracking));
    if (auto_tracking) {
        memcpy(m_auto_tracking, auto_tracking, sizeof(ms_auto_tracking));
    }
    m_eventLoop.exit();
}

void HumanDetection::ON_RESPONSE_FLAG_SET_AUTO_TRACKING(MessageReceive *message)
{
    Q_UNUSED(message)
    m_eventLoop.exit();
}

void HumanDetection::ON_RESPONSE_FLAG_GET_VAC_SETTINGS2(MessageReceive *message)
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

void HumanDetection::ON_RESPONSE_FLAG_SET_VAC_SETTINGS2(MessageReceive *message)
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

void HumanDetection::clearSettings()
{
    ui->checkBoxEnable->setChecked(false);
    ui->checkBox_tracks->setChecked(false);
}

void HumanDetection::updateEnableState()
{
    ui->checkBoxEnable->setEnabled(m_isConnected && m_isSupported);
    bool isEnable = ui->checkBoxEnable->isEnabled() && ui->checkBoxEnable->isChecked();
    ui->checkBox_tracks->setEnabled(isEnable);
    ui->pushButtonObjectSizeEdit->setEnabled(isEnable);
    ui->pushButtonObjectSizeReset->setEnabled(isEnable);
    ui->pushButtonObjectSizeReset->setVisible(ui->pushButtonObjectSizeReset->isEnabled()
                                              && ui->pushButtonObjectSizeEdit->buttonState() == PushButtonEditState::StateEditing);
    ui->pushButton_editTime->setEnabled(isEnable);
    ui->pushButton_editAction->setEnabled(isEnable);
    ui->widgetSize->setEnabled(isEnable
                               && ui->pushButtonObjectSizeEdit->buttonState() == PushButtonEditState::StateEditing);
    setApplyButtonEnable(ui->checkBoxEnable->isEnabled());
    ui->commonVideo->setDrawViewVisible(ui->pushButtonObjectSizeEdit->buttonState() == PushButtonEditState::StateEditing);
}

bool HumanDetection::checkObjectSize()
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

void HumanDetection::onLanguageChanged()
{
    ui->label_human->setText(GET_TEXT("SMARTEVENT/55007", "Human Detection"));
    ui->label_showTracks->setText(GET_TEXT("SMARTEVENT/55025", "Show Tracks"));
    ui->labelObjectSizeLimits->setText(GET_TEXT("SMARTEVENT/55063", "Object Size Limits"));
    ui->pushButtonObjectSizeReset->setText(GET_TEXT("COMMON/1057", "Reset"));
    ui->label_effectiveTime->setText(GET_TEXT("SMARTEVENT/55015", "Effective Time"));
    ui->label_action->setText(GET_TEXT("VIDEOLOSS/50002", "Action"));
    ui->pushButton_editAction->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->pushButton_editTime->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->checkBoxEnable->setText(GET_TEXT("COMMON/1009", "Enable"));
}

void HumanDetection::onChannelButtonClicked(int index)
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
    m_videoDraw->setTrackEnable(false);
    //
    ui->widgetMinLineBox->setValid(true);
    ui->widgetMaxLineBox->setValid(true);

    int channel = index;
    setCurrentChannel(channel);
    ui->commonVideo->playVideo(channel);
    ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateFinished);
    m_isConnected = false;
    m_isSupported = false;
    emit hideMessage();
    //检查通道是否连接
    m_isConnected = isChannelEnable(channel);
    if (!m_isConnected) {
        showNotConnectedMessage();
        clearSettings();
        updateEnableState();
        return;
    }

    //【ID1045474】QT-VCA-Human Detection：45平台机型，77版本或全平台80及以上或Sigma机型不支持人体检测功能
    MsCameraVersion cameraVersion = MsCameraVersion::fromChannel(channel);
    if ((cameraVersion.chipVersion() == 5 && cameraVersion >= MsCameraVersion(7, 77)) || cameraVersion >= MsCameraVersion(8, 1) || qMsNvr->isSigma(channel)) {
        showNotSupportMessage();
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
    //获取数据
    int result = 0;
    sendMessage(REQUEST_FLAG_GET_VCA_HUMANDETECTION, (void *)&channel, sizeof(int));
    result = m_eventLoop.exec();
    if (result < 0) {
        m_isConnected = false;
        updateEnableState();
        showDataError();
        return;
    }
    sendMessage(REQUEST_FLAG_GET_VAC_SETTINGS2, (void *)&channel, sizeof(int));
    result = m_eventLoop.exec();
    if (result < 0) {
        m_isConnected = false;
        updateEnableState();
        showDataError();
        return;
    }
    //closeWait();
    updateEnableState();
}

void HumanDetection::on_pushButton_editTime_clicked()
{
    if (!m_effectiveTime) {

        m_effectiveTime = new EffectiveTimeVCA(this);
    }
    m_effectiveTime->showEffectiveTime(currentChannel(), HUMAN);
}

void HumanDetection::on_pushButton_editAction_clicked()
{
    if (!m_action) {
        m_action = new ActionSmartEvent(this);
    }
    m_action->showAction(currentChannel(), HUMAN);
}

void HumanDetection::on_checkBoxEnable_clicked(bool checked)
{
    if (!checked) {
        ui->checkBox_tracks->setChecked(false);
        on_checkBox_tracks_clicked(false);
        ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateFinished);
    }
    updateEnableState();
}

void HumanDetection::on_checkBox_tracks_clicked(bool checked)
{
    m_videoDraw->setCurrentChannel(currentChannel());
    m_videoDraw->setTrackEnable(checked);
}

void HumanDetection::onSceneObjectSizeChanged(const QRect &rc, MsQtVca::ObjectSizeType sizeType)
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

void HumanDetection::on_radioButton_minLayout_clicked(bool checked)
{
    if (!checked) {
        ui->radioButton_minLayout->setChecked(true);
        return;
    } else {
        ui->radioButton_maxLayout->setChecked(false);
    }
    m_drawScene->showMinimumObjectSize(m_settings_info2, ObjectType, m_objectSizeType);
}
void HumanDetection::on_radioButton_maxLayout_clicked(bool checked)
{
    if (!checked) {
        ui->radioButton_maxLayout->setChecked(true);
    } else {
        ui->radioButton_minLayout->setChecked(false);
    }
    m_drawScene->showMaximumObjectSize(m_settings_info2, ObjectType, m_objectSizeType);
}

void HumanDetection::onLineEditMinWidthEditingFinished()
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

void HumanDetection::onLineEditMinHeightEditingFinished()
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

void HumanDetection::onLineEditMaxWidthEditingFinished()
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

void HumanDetection::onLineEditMaxHeightEditingFinished()
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

void HumanDetection::on_pushButtonObjectSizeEdit_clicked()
{
    switch (ui->pushButtonObjectSizeEdit->buttonState()) {
    case PushButtonEditState::StateFinished:
        ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateEditing);
        break;
    case PushButtonEditState::StateEditing:
        ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateFinished);
        break;
    }
}

void HumanDetection::on_pushButtonObjectSizeEdit_stateSet(PushButtonEditState::State state)
{
    switch (state) {
    case PushButtonEditState::StateFinished:
        ui->commonVideo->showDrawWidget();
        ui->radioButton_minLayout->setChecked(false);
        ui->radioButton_maxLayout->setChecked(false);
        ui->commonVideo->removeDrawScene();
        break;
    case PushButtonEditState::StateEditing:
        ui->commonVideo->hideDrawWidget();
        ui->radioButton_minLayout->setChecked(true);
        ui->radioButton_maxLayout->setChecked(false);
        ui->commonVideo->showDrawScene(m_drawScene);
        m_drawScene->showMinimumObjectSize(m_settings_info2, ObjectType, m_objectSizeType);
        break;
    }
    updateEnableState();
}

void HumanDetection::on_pushButtonObjectSizeReset_clicked()
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
