#include "TamperDetection.h"
#include "ui_TamperDetection.h"
#include "ActionVCA.h"
#include "EffectiveTimeVCA.h"
#include "FaceData.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "drawmotion.h"

TamperDetection::TamperDetection(QWidget *parent)
    : BaseSmartWidget(parent)
    , ui(new Ui::TamperDetection)
{
    ui->setupUi(this);

    ui->widget_buttonGroup->setCount(qMsNvr->maxChannel());
    ui->widget_buttonGroup->setCurrentIndex(0);
    connect(ui->widget_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onChannelButtonClicked(int)));
    ui->horizontalSlider_sensitivity->setRange(1, 10);
    //    m_videoDraw = new DrawMotion(this);
    //    ui->commonVideo->showDrawWidget(m_videoDraw);
    m_event_info = new ms_smart_event_info;
    clearSettings();
    onLanguageChanged();
}

TamperDetection::~TamperDetection()
{
    if (m_event_info) {
        delete m_event_info;
        m_event_info = nullptr;
    }
    delete ui;
}

void TamperDetection::initializeData(int channel)
{
    ui->widget_buttonGroup->setCurrentIndex(channel);
}

void TamperDetection::saveData()
{
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
    m_event_info->enable = ui->checkBoxEnable->isChecked();
    //m_videoDraw->getRegion(m_event_info->area);
    m_event_info->sensitivity = ui->horizontalSlider_sensitivity->value();
    sendMessage(REQUEST_FLAG_SET_VCA_TAMPER, m_event_info, sizeof(ms_smart_event_info));
    m_eventLoop.exec();
    clearCopyInfo();
    //closeWait();
}

void TamperDetection::copyData()
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
            } else {
                m_event_info->copyChn[i] = '0';
            }
        }
        saveData();
        copy_smart_event_action(TAMPER, currentChannel(), copyFlags, 1);
    }
}

void TamperDetection::clearCopyInfo()
{
    for (int i = 0; i < MAX_LEN_65; i++) {
        m_event_info->copyChn[i] = '0';
    }
}

void TamperDetection::processMessage(MessageReceive *message)
{
    dealGlobalMessage(message);
    //
    switch (message->type()) {
    case RESPONSE_FLAG_GET_VCA_TAMPER:
        ON_RESPONSE_FLAG_GET_VCA_TAMPER(message);
        break;
    case RESPONSE_FLAG_SET_VCA_TAMPER:
        ON_RESPONSE_FLAG_SET_VCA_TAMPER(message);
        break;
    default:
        break;
    }
}

void TamperDetection::ON_RESPONSE_FLAG_GET_VCA_TAMPER(MessageReceive *message)
{
    if (message->isNull()) {
        qMsWarning() << QString("data is null.");
        m_eventLoop.exit(-1);
        return;
    }
    ms_smart_event_info *info = (ms_smart_event_info *)message->data;
    memcpy(m_event_info, info, sizeof(ms_smart_event_info));
    ui->checkBoxEnable->setChecked(m_event_info->enable);
    //m_videoDraw->setRegion(m_event_info->area);
    ui->horizontalSlider_sensitivity->setValue(m_event_info->sensitivity);
    m_eventLoop.exit();
}

void TamperDetection::ON_RESPONSE_FLAG_SET_VCA_TAMPER(MessageReceive *message)
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

void TamperDetection::onLanguageChanged()
{
    ui->label_tamper->setText(GET_TEXT("SMARTEVENT/55004", "Tamper Detection"));
    ui->label_sensitivity->setText(GET_TEXT("MOTION/51003", "Sensitivity"));
    ui->label_effectiveTime->setText(GET_TEXT("SMARTEVENT/55015", "Effective Time"));
    ui->label_action->setText(GET_TEXT("VIDEOLOSS/50002", "Action"));
    ui->pushButton_editAction->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->pushButton_editTime->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->checkBoxEnable->setText(GET_TEXT("COMMON/1009", "Enable"));
}

void TamperDetection::clearSettings()
{
    ui->checkBoxEnable->setChecked(false);
}

void TamperDetection::updateEnableState()
{
    ui->checkBoxEnable->setEnabled(m_isConnected && m_isSupported);
    bool isEnable = ui->checkBoxEnable->isEnabled() && ui->checkBoxEnable->isChecked();
    ui->horizontalSlider_sensitivity->setEnabled(isEnable);
    ui->pushButton_editTime->setEnabled(isEnable);
    ui->pushButton_editAction->setEnabled(isEnable);
    setApplyButtonEnable(ui->checkBoxEnable->isEnabled());
}

void TamperDetection::onChannelButtonClicked(int index)
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
    int channel = index;
    setCurrentChannel(channel);
    ui->commonVideo->playVideo(channel);

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
    // int result = 0;
    // sendMessage(REQUEST_FLAG_GET_VCA_TAMPER, (void *)&channel, sizeof(int));
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

void TamperDetection::on_pushButton_editTime_clicked()
{
    if (!m_effectiveTime) {

        m_effectiveTime = new EffectiveTimeVCA(this);
    }
    m_effectiveTime->showEffectiveTime(currentChannel(), TAMPER);
}

void TamperDetection::on_pushButton_editAction_clicked()
{
    if (!m_action) {
        m_action = new ActionSmartEvent(this);
    }
    m_action->showAction(currentChannel(), TAMPER);
}

void TamperDetection::on_checkBoxEnable_clicked(bool checked)
{
    Q_UNUSED(checked)
    updateEnableState();
}
