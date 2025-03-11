#include "AddWhiteLed.h"
#include "ui_AddWhiteLed.h"
#include "centralmessage.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsWaitting.h"
#include <QtDebug>

AddWhiteLed *AddWhiteLed::s_self = nullptr;

AddWhiteLed::AddWhiteLed(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::AddWhiteLed)
{
    s_self = this;

    ui->setupUi(this);

    ui->comboBox_channel->clear();
    for (int i = 0; i < qMsNvr->maxChannel(); ++i) {
        ui->comboBox_channel->addItem(QString("%1").arg(i + 1), i);
    }

    ui->label_channel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));
    ui->label_mode->setText(GET_TEXT("WHITELED/105001", "Flash Mode"));
    ui->label_time->setText(GET_TEXT("WHITELED/105002", "Flash Time"));

    ui->pushButton_reset->setText(GET_TEXT("COMMON/1057", "Reset"));
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));

    ui->comboBox_mode->clear();
    ui->comboBox_mode->addItem(GET_TEXT("ACTION/56001", "Twinkle"), 0);
    ui->comboBox_mode->addItem(GET_TEXT("ACTION/56002", "Always"), 1);
}

AddWhiteLed::~AddWhiteLed()
{
    s_self = nullptr;
    delete ui;
}

AddWhiteLed *AddWhiteLed::instance()
{
    return s_self;
}

void AddWhiteLed::setParamsMap(const QMap<int, WHITE_LED_PARAMS> &map)
{
    m_paramsMap = map;
}

int AddWhiteLed::execAdd()
{
    m_mode = ModeAdd;
    ui->label_title->setText(GET_TEXT("WHITELED/105003", "White LED Add"));
    ui->comboBox_channel->setCurrentIndexFromData(0);
    ui->comboBox_mode->setCurrentIndexFromData(0);
    on_comboBox_mode_indexSet(0);
    return exec();
}

int AddWhiteLed::execEdit(const WHITE_LED_PARAMS &params)
{
    qDebug() << "====AddWhiteLed::execEdit====";
    qDebug() << "----chnid:" << params.chnid;
    qDebug() << "----acto_chn_id:" << params.acto_chn_id;
    qDebug() << "----flash_mode:" << params.flash_mode;
    qDebug() << "----flash_time:" << params.flash_time;

    m_mode = ModeEdit;
    m_paramsMap.remove(params.acto_chn_id);
    ui->label_title->setText(GET_TEXT("WHITELED/105004", "White LED Edit"));

    ui->comboBox_channel->setCurrentIndexFromData(params.acto_chn_id);
    QMetaObject::invokeMethod(this, "on_comboBox_channel_activated", Qt::QueuedConnection, Q_ARG(int, params.acto_chn_id));

    ui->comboBox_mode->setCurrentIndexFromData(params.flash_mode);
    ui->horizontalSlider_time->setValue(params.flash_time);

    return exec();
}

WHITE_LED_PARAMS AddWhiteLed::params() const
{
    return m_params;
}

void AddWhiteLed::dealMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPC_LED_PARAMS:
        ON_RESPONSE_FLAG_GET_IPC_LED_PARAMS(message);
        break;
    }
}

void AddWhiteLed::ON_RESPONSE_FLAG_GET_IPC_LED_PARAMS(MessageReceive *message)
{
    //MsWaitting::closeGlobalWait();

    ms_ipc_led_params_resp *led_params = (ms_ipc_led_params_resp *)message->data;
    qDebug() << "====AddWhiteLed::ON_RESPONSE_FLAG_GET_IPC_LED_PARAMS====";
    if (led_params) {
        qDebug() << "----channel:" << led_params->chnid;
        qDebug() << "----led_alarm:" << led_params->led_alarm;
        qDebug() << "----led_number:" << led_params->led_number;
        qDebug() << "----led_manual:" << led_params->led_manual;

        if (led_params->led_number != 1 || led_params->led_alarm != 1) {
            ShowMessageBox(this, GET_TEXT("PTZCONFIG/36013", "This channel does not support this function."));
        }
    }
}

void AddWhiteLed::on_comboBox_channel_activated(int index)
{
    Q_UNUSED(index);
}

void AddWhiteLed::on_comboBox_mode_indexSet(int index)
{
    int mode = ui->comboBox_mode->itemData(index).toInt();
    switch (mode) {
    case 0:
        ui->horizontalSlider_time->setRange(1, 10);
        ui->horizontalSlider_time->setValue(3);
        break;
    case 1:
        ui->horizontalSlider_time->setRange(1, 60);
        ui->horizontalSlider_time->setValue(5);
        break;
    }
}

void AddWhiteLed::on_pushButton_reset_clicked()
{
    int mode = ui->comboBox_mode->currentData().toInt();
    switch (mode) {
    case 0:
        ui->horizontalSlider_time->setValue(3);
        break;
    case 1:
        ui->horizontalSlider_time->setValue(5);
        break;
    }
}

void AddWhiteLed::on_pushButton_ok_clicked()
{
    int channel = ui->comboBox_channel->currentData().toInt();
    if (channel < 0) {
        return;
    }
    m_params.acto_chn_id = channel;
    m_params.flash_mode = ui->comboBox_mode->currentData().toInt();
    m_params.flash_time = ui->horizontalSlider_time->value();

    if (m_paramsMap.contains(m_params.acto_chn_id)) {
        ShowMessageBox(this, GET_TEXT("ALARMIN/52016", "This channel has already existed."));
        return;
    }

    accept();
}

void AddWhiteLed::on_pushButton_cancel_clicked()
{
    reject();
}
