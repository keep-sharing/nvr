#include "AddAlarmOutput.h"
#include "ui_AddAlarmOutput.h"
#include "centralmessage.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"

AddAlarmOutput *AddAlarmOutput::s_addAlarmOutput = nullptr;

AddAlarmOutput::AddAlarmOutput(QMap<AlarmKey, bool> *alarmMap, QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::AddAlarmOutput)
{
    s_addAlarmOutput = this;
    ui->setupUi(this);

    m_alarmMap = alarmMap;
    ui->comboBox_channel->clear();
    for (auto iter = alarmMap->constBegin(); iter != alarmMap->constEnd(); ++iter) {
        const AlarmKey &key = iter.key();
        ui->comboBox_channel->addItem(key.numberName(), QVariant::fromValue(key));
    }

    m_waitting = new MsWaitting(this);

    onLanguageChanged();
}

AddAlarmOutput::~AddAlarmOutput()
{
    s_addAlarmOutput = nullptr;
    delete ui;
}

AddAlarmOutput *AddAlarmOutput::instance()
{
    return s_addAlarmOutput;
}

void AddAlarmOutput::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPC_ALARM_OUTPUT:
        ON_RESPONSE_FLAG_GET_IPC_ALARM_OUTPUT(message);
        break;
    }
}

void AddAlarmOutput::ON_RESPONSE_FLAG_GET_IPC_ALARM_OUTPUT(MessageReceive *message)
{
    ms_ipc_alarm_out *ipc_alarm_out = (ms_ipc_alarm_out *)message->data;
    if (!ipc_alarm_out) {
        m_waitting->exitWait(false);
        return;
    }

    const AlarmKey &key = ui->comboBox_channel->currentData().value<AlarmKey>();
    if (ipc_alarm_out->alarmCnt > key.alarmId()) {
        m_waitting->exitWait(true);
    } else {
        m_waitting->exitWait(false);
    }
}

void AddAlarmOutput::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("ACTION/56005", "Alarm Output Add"));
    ui->label_channel->setText(GET_TEXT("ALARMOUT/53001", "Alarm Output No."));
    ui->label_name->setText(GET_TEXT("ALARMIN/52003", "Alarm Name"));

    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void AddAlarmOutput::on_pushButton_ok_clicked()
{
    const AlarmKey &key = ui->comboBox_channel->currentData().value<AlarmKey>();
    if (m_alarmMap->value(key)) {
        ShowMessageBox(this, GET_TEXT("ALARMOUT/53014", "This alarm output No. has already existed."));
        return;
    }

    m_alarmMap->insert(key, true);

    accept();
}

void AddAlarmOutput::on_pushButton_cancel_clicked()
{
    reject();
}

void AddAlarmOutput::on_comboBox_channel_currentIndexChanged(int index)
{
    const AlarmKey &key = ui->comboBox_channel->itemData(index).value<AlarmKey>();
    ui->lineEdit_name->setText(key.name());
}
