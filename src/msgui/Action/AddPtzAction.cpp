#include "AddPtzAction.h"
#include "ui_AddPtzAction.h"
#include "centralmessage.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "ptzdatamanager.h"
#include <QtDebug>

AddPtzAction::AddPtzAction(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::AddPtzAction)
{
    ui->setupUi(this);

    //
    ui->comboBox_ptzChannel->clear();
    const int maxChannel = qMsNvr->maxChannel();
    for (int i = 0; i < maxChannel && i < MAX_CAMERA; i++) {
        ui->comboBox_ptzChannel->addItem(QString("%1").arg(i + 1), i);
    }
    //
    if (m_info == NULL) {
        m_info = new resp_ptz_ovf_info;
        memset(m_info, 0x0, sizeof(resp_ptz_ovf_info));
    }
    //
    ui->comboBox_actionType->clear();
    ui->comboBox_actionType->addItem(GET_TEXT("PTZDIALOG/21015", "Preset"), PRESET);
    ui->comboBox_actionType->addItem(GET_TEXT("PTZDIALOG/21016", "Patrol"), PATROL);
    //
    m_waitting = new MsWaitting(this);
    onLanguageChanged();
}

AddPtzAction::~AddPtzAction()
{
    delete ui;
}

void AddPtzAction::showAdd(int channel, int eventType, QMap<int, ptz_action_params> *ptzActionMap)
{
    m_mode = ModeAdd;
    m_actionChannel = channel;
    m_eventType = eventType;
    m_ptzActionMap = ptzActionMap;

    ui->label_title->setText(GET_TEXT("ACTION/56004", "PTZ Action Add"));
    ui->comboBox_ptzChannel->setCurrentIndexFromData(0);
    ui->comboBox_actionType->setCurrentIndexFromData(PRESET);

    show();
}

void AddPtzAction::showEdit(int channel, int eventType, const ptz_action_params &action, QMap<int, ptz_action_params> *ptzActionMap)
{
    m_mode = ModeEdit;
    m_actionChannel = channel;
    m_eventType = eventType;
    m_ptzActionMap = ptzActionMap;

    ui->label_title->setText(GET_TEXT("ACTION/56009", "PTZ Action Edit"));
    ui->comboBox_ptzChannel->setCurrentIndexFromData(action.acto_ptz_channel - 1);
    ui->comboBox_actionType->setCurrentIndexFromData(action.acto_ptz_type);
    switch (action.acto_ptz_type) {
    case PRESET:
        m_itemNumber = action.acto_ptz_preset - 1;
        break;
    case PATROL:
        m_itemNumber = action.acto_ptz_patrol - 1;
        break;
    }
    m_isEdit = true;
    memcpy(&m_currentAction, &action, sizeof(ptz_action_params));

    QMetaObject::invokeMethod(this, "on_comboBox_ptzChannel_activated", Qt::QueuedConnection, Q_ARG(int, ui->comboBox_ptzChannel->currentIndex()));
    show();
}

void AddPtzAction::setAll()
{
    switch (m_eventType) {
    case ALARMIO:
        sendMessage(REQUEST_FLAG_SET_ALARMIN, (void *)&m_actionChannel, sizeof(int));
        break;
    case MOTION:
        sendMessageOnly(REQUEST_FLAG_SET_MOTION, (void *)&m_actionChannel, sizeof(int));
        break;
    case VIDEOLOSS:
        sendMessageOnly(REQUEST_FLAG_SET_VIDEOLOSS, (void *)&m_actionChannel, sizeof(int));
        break;
    default:
        break;
    }
}
void AddPtzAction::onLanguageChanged()
{
    ui->label_ptzChannel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));
    ui->label_actionType->setText(GET_TEXT("ALARMIN/52014", "Action Type"));
    ui->label_ptzNumber->setText(GET_TEXT("CAMERASEARCH/32002", "No."));
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void AddPtzAction::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_PTZ_OVF_INFO:
        ON_RESPONSE_FLAG_PTZ_OVF_INFO(message);
        break;
    case RESPONSE_FLAG_PTZ_SUPPORT:
        ON_RESPONSE_FLAG_PTZ_SUPPORT(message);
        break;
    case RESPONSE_FLAG_GET_SINGLE_IPCTYPE:
        ON_RESPONSE_FLAG_GET_SINGLE_IPCTYPE(message);
        break;
    default:
        break;
    }
}

void AddPtzAction::ON_RESPONSE_FLAG_PTZ_SUPPORT(MessageReceive *message)
{
    if (!message->data) {
        //m_waitting->//closeWait();
        qWarning() << "AddPtzAction::ON_RESPONSE_FLAG_PTZ_SUPPORT, data is null.";
        return;
    }
    int enable = *((int *)message->data);
    qDebug() << QString("AddPtzAction::ON_RESPONSE_FLAG_PTZ_SUPPORT, channel: %1, result: %2").arg(m_actionChannel).arg(enable);

    m_isPtzSupport = (enable != -1);

    if (!m_isPtzSupport) {
        //m_waitting->//closeWait();
        showPtzActionList();
        ShowMessageBox(GET_TEXT("PTZCONFIG/36048", "This channel does not support PTZ Action"));
        return;
    }

    sendMessage(REQUEST_FLAG_PTZ_OVF_INFO, (void *)&m_itemChannel, sizeof(int));
}

void AddPtzAction::ON_RESPONSE_FLAG_PTZ_OVF_INFO(MessageReceive *message)
{
    //m_waitting->//closeWait();

    //
    struct resp_ptz_ovf_info *data = (resp_ptz_ovf_info *)message->data;
    if (!data) {
        showPtzActionList();
        return;
    }
    memcpy(m_info, data, sizeof(resp_ptz_ovf_info));

    //
    showPtzActionList();
}

void AddPtzAction::ON_RESPONSE_FLAG_GET_SINGLE_IPCTYPE(MessageReceive *message)
{
    memset(&m_model_info, 0, sizeof(CAM_MODEL_INFO));

    CAM_MODEL_INFO *cam_model_info = (CAM_MODEL_INFO *)message->data;
    if (!cam_model_info) {
        //m_waitting->//closeWait();
        qWarning() << "AddPtzAction::ON_RESPONSE_FLAG_GET_SINGLE_IPCTYPE, data is null.";
        return;
    } else {
        memcpy(&m_model_info, cam_model_info, sizeof(CAM_MODEL_INFO));
        qDebug() << "[david debug] chnid:" << m_model_info.chnid << " ipaddr:" << m_model_info.ipaddr << " model:" << m_model_info.model;
    }


    //m_waitting->//closeWait();
    showPtzActionList();

    return;
}

bool AddPtzAction::modelDetect(const QRegExp &rx)
{
    int index = rx.indexIn(QString(m_model_info.model));
    return index != -1;
}

void AddPtzAction::showPtzActionList()
{
    ui->comboBox_ptzNumber->clear();
    int actionType = ui->comboBox_actionType->currentData().toInt();
    switch (actionType) {
    case PRESET: {
        for (int i = 0; i < MAX_PRESET_CNT; ++i) {
            ui->comboBox_ptzNumber->addItem(QString::number(i + 1), i);
        }
        break;
    }
    case PATROL:
        for (int i = 0; i < TOUR_MAX; ++i) {
            ui->comboBox_ptzNumber->addItem(QString::number(i + 1), i);
        }
        break;
    }
    //
    //
    if (m_isEdit) {
        ui->comboBox_ptzNumber->setCurrentIndexFromData(m_itemNumber);
        m_isEdit = false;
    }
}

void AddPtzAction::on_comboBox_ptzChannel_indexSet(int index)
{
    m_itemChannel = ui->comboBox_ptzChannel->itemData(index).toInt();
    sendMessage(REQUEST_FLAG_GET_SINGLE_IPCTYPE, &m_itemChannel, sizeof(int));
    //m_waitting->//showWait();
}

void AddPtzAction::on_comboBox_actionType_indexSet(int index)
{
    Q_UNUSED(index)

    showPtzActionList();
}

void AddPtzAction::on_pushButton_ok_clicked()
{
    m_itemChannel = ui->comboBox_ptzChannel->currentData().toInt();
    if (m_itemChannel < 0) {
        return;
    }

    ptz_action_params param;
    memset(&param, 0x0, sizeof(ptz_action_params));
    param.chn_id = m_actionChannel;
    //底层第一个通道是1，不是0
    param.acto_ptz_channel = m_itemChannel + 1;

    //
    switch (m_mode) {
    case ModeAdd:
        if (m_ptzActionMap->contains(param.acto_ptz_channel)) {
            ShowMessageBox(GET_TEXT("ALARMIN/52016", "This channel has already existed."));
            return;
        }
        break;
    case ModeEdit:
        if (param.acto_ptz_channel != m_currentAction.acto_ptz_channel && m_ptzActionMap->contains(param.acto_ptz_channel)) {
            ShowMessageBox(GET_TEXT("ALARMIN/52016", "This channel has already existed."));
            return;
        } else {
            m_ptzActionMap->remove(m_currentAction.acto_ptz_channel);
        }
        break;
    default:
        break;
    }

    //
    int actionType = ui->comboBox_actionType->currentData().toInt();
    switch (actionType) {
    case PRESET:
        param.acto_ptz_type = PRESET;
        param.acto_ptz_preset = ui->comboBox_ptzNumber->currentData().toInt() + 1;
        param.acto_ptz_patrol = 0;
        break;
    case PATROL:
        param.acto_ptz_type = PATROL;
        param.acto_ptz_preset = 0;
        param.acto_ptz_patrol = ui->comboBox_ptzNumber->currentData().toInt() + 1;
        break;
    }
    m_ptzActionMap->insert(param.acto_ptz_channel, param);

    accept();
}

void AddPtzAction::on_pushButton_cancel_clicked()
{
    reject();
}

void AddPtzAction::on_comboBox_ptzChannel_activated(int index)
{
    Q_UNUSED(index);
}
