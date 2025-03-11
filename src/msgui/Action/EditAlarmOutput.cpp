#include "EditAlarmOutput.h"
#include "ui_AddAlarmOutput.h"
#include "centralmessage.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include <QtDebug>

EditAlarmOutput *EditAlarmOutput::s_editAlarmOutput = nullptr;

EditAlarmOutput::EditAlarmOutput(QMap<AlarmKey, bool> *alarmMap, QWidget *parent)
    : AddAlarmOutput(alarmMap, parent)
{
    s_editAlarmOutput = this;

    ui->label_title->setText(GET_TEXT("ALARMOUT/53011", "Alarm Output Edit"));
}

EditAlarmOutput::~EditAlarmOutput()
{
    s_editAlarmOutput = nullptr;
}

EditAlarmOutput *EditAlarmOutput::instance()
{
    return s_editAlarmOutput;
}

int EditAlarmOutput::execEdit(const AlarmKey &currentKey)
{
    m_currentKey = currentKey;
    //BUG Qt4似乎不能比较自定义类型的QVariant
#if 0
    ui->comboBox_channel->setCurrentIndexFromData(QVariant::fromValue(m_currentKey));
#else
    int index = -1;
    for (int i = 0; i < ui->comboBox_channel->count(); ++i) {
        const AlarmKey &key = ui->comboBox_channel->itemData(i).value<AlarmKey>();
        if (key == m_currentKey) {
            index = i;
            break;
        }
    }
    ui->comboBox_channel->setCurrentIndex(index);
#endif
    return exec();
}

void EditAlarmOutput::on_pushButton_ok_clicked()
{
    const AlarmKey &key = ui->comboBox_channel->currentData().value<AlarmKey>();
    if (m_alarmMap->value(key) && m_currentKey != key) {
        ShowMessageBox(this, GET_TEXT("ALARMOUT/53014", "This alarm output No. has already existed."));
        return;
    }

    m_alarmMap->insert(m_currentKey, false);
    m_alarmMap->insert(key, true);

    accept();
}

void EditAlarmOutput::on_pushButton_cancel_clicked()
{
    reject();
}
