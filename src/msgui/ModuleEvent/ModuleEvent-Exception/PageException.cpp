#include "PageException.h"
#include "msdefs.h"
#include "ui_PageException.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include <QtDebug>

PageException::PageException(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::PageException)
{
    ui->setupUi(this);

    ui->comboBox_exceptionType->clear();
    ui->comboBox_exceptionType->addItem(GET_TEXT("LIVEVIEW/20042", "NETWORK DISCONNECTED"), EXCEPT_NETWORK_DISCONN);
    ui->comboBox_exceptionType->addItem(GET_TEXT("LIVEVIEW/168005", "IP Address Conflict"), EXCEPT_IP_CONFLICT);
    ui->comboBox_exceptionType->addItem(GET_TEXT("LIVEVIEW/20043", "Disk Full"), EXCEPT_DISK_FULL);
    ui->comboBox_exceptionType->addItem(GET_TEXT("LIVEVIEW/20041", "Record Failed"), EXCEPT_RECORD_FAIL);
    ui->comboBox_exceptionType->addItem(GET_TEXT("EXCEPTION/54008", "Disk Error"), EXCEPT_DISK_FAIL);
    ui->comboBox_exceptionType->addItem(GET_TEXT("EXCEPTION/54010", "Disk Uninitialized"), EXCEPT_DISK_NO_FORMAT);
    ui->comboBox_exceptionType->addItem(GET_TEXT("EXCEPTION/54011", "NO Disk"), EXCEPT_NO_DISK);
    ui->comboBox_exceptionType->addItem(GET_TEXT("EXCEPTION/176000", "Disk Offine"), EXCEPT_DISK_OFFLINE);
    if(qMsNvr->is3536a()) {
        ui->comboBox_exceptionType->addItem(GET_TEXT("EXCEPTION/176001", "Disk Heat"), EXCEPT_DISK_HEAT);
        ui->comboBox_exceptionType->addItem(GET_TEXT("EXCEPTION/176002", "Disk Microtherm"), EXCEPT_DISK_MICROTHERM);
        ui->comboBox_exceptionType->addItem(GET_TEXT("EXCEPTION/176003", "Disk Connection Exception"), EXCEPT_DISK_CONNECTION_EXCEPTION);
        ui->comboBox_exceptionType->addItem(GET_TEXT("EXCEPTION/176004", "Disk Strike"), EXCEPT_DISK_STRIKE);
    }

    ui->comboBox_email_interval->addItem("10s", 10);
    ui->comboBox_email_interval->addItem("20s", 20);
    ui->comboBox_email_interval->addItem("40s", 40);
    ui->comboBox_email_interval->addItem("60s", 60);
    ui->comboBox_email_interval->addItem("100s", 100);
    ui->comboBox_email_interval->addItem("5min", 300);
    ui->comboBox_email_interval->addItem("15min", 900);
    ui->comboBox_email_interval->addItem("30min", 1800);
    ui->comboBox_email_interval->addItem("1h", 3600);
    ui->comboBox_email_interval->addItem("8h", 28800);
    ui->comboBox_email_interval->addItem("12h", 43200);
    ui->comboBox_email_interval->addItem("24h", 86400);

    ui->comboBoxAudioFile->beginEdit();
    ui->comboBoxAudioFile->clear();
    ui->comboBoxAudioFile->addItem(GET_TEXT("COMMON/1050", "Default"), 0);
    ui->comboBoxAudioFile->addItem("1", 1);
    ui->comboBoxAudioFile->addItem("2", 2);
    ui->comboBoxAudioFile->addItem("3", 3);
    ui->comboBoxAudioFile->addItem("4", 4);
    ui->comboBoxAudioFile->addItem("5", 5);
    ui->comboBoxAudioFile->addItem("6", 6);
    ui->comboBoxAudioFile->addItem("7", 7);
    ui->comboBoxAudioFile->addItem("8", 8);
    ui->comboBoxAudioFile->addItem("9", 9);
    ui->comboBoxAudioFile->addItem("10", 10);
    ui->comboBoxAudioFile->endEdit();

    ui->comboBoxEnable->clear();
    ui->comboBoxEnable->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);
    ui->comboBoxEnable->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);

    if (!qMsNvr->isSupportAudio()) {
        ui->widgetAudioFile->setVisible(false);
    }
    connect(ui->checkBox_event, SIGNAL(clicked(bool)), this, SLOT(onSaveSetting()));
    connect(ui->checkBox_audible, SIGNAL(clicked(bool)), this, SLOT(onSaveSetting()));
    connect(ui->checkBox_email, SIGNAL(clicked(bool)), this, SLOT(onSaveSetting()));
    connect(ui->checkBoxHTTP, SIGNAL(clicked(bool)), this, SLOT(onSaveSetting()));
    connect(ui->textEditURL, SIGNAL(editingFinished()), this, SLOT(onSaveSetting()));
    connect(ui->lineEditUserName, SIGNAL(editingFinished()), this, SLOT(onSaveSetting()));
    connect(ui->lineEditPassword, SIGNAL(editingFinished()), this, SLOT(onSaveSetting()));

    //热备机模式隐藏Exception - Email Linkage选项
    if (qMsNvr->isSlaveMode()) {
        ui->checkBox_email->setVisible(false);
    }

    //
    int alarmoutCount = qMsNvr->maxAlarmOutput();
    if (alarmoutCount < 1) {
        ui->widget_alarmout->hide();
    } else {
        switch (alarmoutCount) {
        case 1:
            ui->checkBox_alarmout2->hide();
            ui->checkBox_alarmout3->hide();
            ui->checkBox_alarmout4->hide();
            break;
        case 2:
            ui->checkBox_alarmout3->hide();
            ui->checkBox_alarmout4->hide();
            break;
        case 3:
            ui->checkBox_alarmout4->hide();
            break;
        case 4:
            break;
        }
    }
    connect(ui->checkBox_alarmoutAll, SIGNAL(clicked(bool)), this, SLOT(onAlarmOutAllClicked(bool)));
    connect(ui->checkBox_alarmout1, SIGNAL(clicked(bool)), this, SLOT(onAlarmOutCheckChanged()));
    connect(ui->checkBox_alarmout2, SIGNAL(clicked(bool)), this, SLOT(onAlarmOutCheckChanged()));
    connect(ui->checkBox_alarmout3, SIGNAL(clicked(bool)), this, SLOT(onAlarmOutCheckChanged()));
    connect(ui->checkBox_alarmout4, SIGNAL(clicked(bool)), this, SLOT(onAlarmOutCheckChanged()));

    //
    onLanguageChanged();
}

PageException::~PageException()
{
    delete ui;
}

void PageException::initializeData()
{
    memset(&m_alarm, 0, sizeof(trigger_alarms));
    read_trigger_alarms(SQLITE_FILE_NAME, &m_alarm);
    memset(m_httpPrms, 0, sizeof(m_httpPrms));
    for (int i = 0; i < EXCEPT_COUNT; i++) {
        m_httpPrms[i].id = i;
        if (i == EXCEPT_NETWORK_DISCONN || i == EXCEPT_IP_CONFLICT) {
            m_httpPrms[i].id = -1;
        }
    }
    read_http_notification_params_init(SQLITE_FILE_NAME, EXCEPT_HTTP_PARAMS, m_httpPrms, EXCEPT_COUNT);
    ui->comboBox_exceptionType->setCurrentIndex(0);
    on_comboBox_exceptionType_activated(0);
}

void PageException::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void PageException::showAlarmOut(int value)
{
    if (ui->checkBox_alarmout1->isVisible()) {
        ui->checkBox_alarmout1->setChecked((value >> 6) & 0x0001);
    }
    if (ui->checkBox_alarmout1->isVisible()) {
        ui->checkBox_alarmout2->setChecked((value >> 7) & 0x0001);
    }
    if (ui->checkBox_alarmout1->isVisible()) {
        ui->checkBox_alarmout3->setChecked((value >> 8) & 0x0001);
    }
    if (ui->checkBox_alarmout1->isVisible()) {
        ui->checkBox_alarmout4->setChecked((value >> 9) & 0x0001);
    }
    if (ui->comboBoxAudioFile->isVisible()) {
        ui->comboBoxAudioFile->setCurrentIndexFromData((value >> 10) & 15);
    }
    onAlarmOutCheckChanged();
}

void PageException::getAlarmOut(int &value)
{
    if (ui->checkBox_alarmout1->isChecked()) {
        value |= (1 << 6);
    } else {
        value &= ~(1 << 6);
    }
    if (ui->checkBox_alarmout2->isChecked()) {
        value |= (1 << 7);
    } else {
        value &= ~(1 << 7);
    }
    if (ui->checkBox_alarmout3->isChecked()) {
        value |= (1 << 8);
    } else {
        value &= ~(1 << 8);
    }
    if (ui->checkBox_alarmout4->isChecked()) {
        value |= (1 << 9);
    } else {
        value &= ~(1 << 9);
    }
    value &= ~(15 << 10);
    int fileNo = ui->comboBoxAudioFile->currentData().toInt();
    value |= (fileNo << 10);
}

void PageException::onLanguageChanged()
{
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->groupBox->setTitle(QString("%1").arg(GET_TEXT("EXCEPTION/54002", "Trigger Action")));
    ui->label_exceptionType->setText(GET_TEXT("EXCEPTION/54001", "Exception Type"));
    ui->checkBox_event->setText(GET_TEXT("EXCEPTION/54009", "Event Notification"));
    ui->labelAudioFile->setText((GET_TEXT("AUDIOFILE/117012", "Audio File")));
    ui->checkBox_audible->setText(GET_TEXT("VIDEOLOSS/50009", "Audible Warning"));
    ui->checkBox_email->setText(GET_TEXT("VIDEOLOSS/50010", "Email Linkage"));
    ui->label_email_interval->setText(GET_TEXT("VIDEOLOSS/50006", "Triggered Interval"));
    ui->checkBox_alarmoutAll->setText(GET_TEXT("ALARMOUT/53005", "NVR Alarm Output"));

    ui->checkBoxHTTP->setText(GET_TEXT("ACTION/153000", "HTTP Notification"));
    ui->labelURL->setText(GET_TEXT("ACTION/153001", "URL"));
    ui->labelUserName->setText(GET_TEXT("COMMON/1007", "User Name"));
    ui->labelPassword->setText(GET_TEXT("COMMON/1008", "Password"));
}

void PageException::on_comboBox_email_interval_activated(int index)
{
    //ui->comboBox_email_interval->setCurrentIndexFromData(index);
    qDebug() << "Exception::on_comboBox_email_interval_activated(int index):" << index;
    Q_UNUSED(index)
    onSaveSetting();
}

void PageException::onSaveSetting()
{
    int interval = ui->comboBox_email_interval->currentData().toInt();
    const except_event &event = static_cast<except_event>(ui->comboBox_exceptionType->currentData().toInt());
    switch (event) {
    case EXCEPT_NETWORK_DISCONN:
        if (ui->comboBoxEnable->currentIntData()) {
            m_alarm.network_disconn &= ~(1 << 0);
        } else {
            m_alarm.network_disconn |= (1 << 0);
        }
        if (ui->checkBox_event->isChecked()) {
            m_alarm.network_disconn &= ~(1 << 3);
        } else {
            m_alarm.network_disconn |= (1 << 3);
        }
        if (ui->checkBox_audible->isChecked()) {
            m_alarm.network_disconn |= (1 << 4);
        } else {
            m_alarm.network_disconn &= ~(1 << 4);
        }
        if (ui->checkBox_email->isChecked()) {
            m_alarm.network_disconn |= (1 << 5);
        } else {
            m_alarm.network_disconn &= ~(1 << 5);
        }
        getAlarmOut(m_alarm.network_disconn);

        if (ui->checkBoxHTTP->isChecked()) {
            m_alarm.network_disconn |= (1 << 14);
        } else {
            m_alarm.network_disconn &= ~(1 << 14);
        }
        break;
    case EXCEPT_DISK_FULL:
        if (ui->comboBoxEnable->currentIntData()) {
            m_alarm.disk_full &= ~(1 << 0);
        } else {
            m_alarm.disk_full |= (1 << 0);
        }
        if (ui->checkBox_event->isChecked()) {
            m_alarm.disk_full &= ~(1 << 3);
        } else {
            m_alarm.disk_full |= (1 << 3);
        }
        if (ui->checkBox_audible->isChecked()) {
            m_alarm.disk_full |= (1 << 4);
        } else {
            m_alarm.disk_full &= ~(1 << 4);
        }
        if (ui->checkBox_email->isChecked()) {
            m_alarm.disk_full |= (1 << 5);
        } else {
            m_alarm.disk_full &= ~(1 << 5);
        }
        getAlarmOut(m_alarm.disk_full);
        if (ui->checkBoxHTTP->isChecked()) {
            m_alarm.disk_full |= (1 << 14);
        } else {
            m_alarm.disk_full &= ~(1 << 14);
        }
        strcpy(m_httpPrms[EXCEPT_DISK_FULL].url, ui->textEditURL->toPlainText().toStdString().c_str());
        strcpy(m_httpPrms[EXCEPT_DISK_FULL].username, ui->lineEditUserName->text().toStdString().c_str());
        strcpy(m_httpPrms[EXCEPT_DISK_FULL].password, ui->lineEditPassword->text().toStdString().c_str());
        break;
    case EXCEPT_RECORD_FAIL:
        if (ui->comboBoxEnable->currentIntData()) {
            m_alarm.record_fail &= ~(1 << 0);
        } else {
            m_alarm.record_fail |= (1 << 0);
        }
        if (ui->checkBox_event->isChecked()) {
            m_alarm.record_fail &= ~(1 << 3);
        } else {
            m_alarm.record_fail |= (1 << 3);
        }
        if (ui->checkBox_audible->isChecked()) {
            m_alarm.record_fail |= (1 << 4);
        } else {
            m_alarm.record_fail &= ~(1 << 4);
        }
        if (ui->checkBox_email->isChecked()) {
            m_alarm.record_fail |= (1 << 5);
        } else {
            m_alarm.record_fail &= ~(1 << 5);
        }
        getAlarmOut(m_alarm.record_fail);
        m_alarm.record_mail_interval = interval;
        if (ui->checkBoxHTTP->isChecked()) {
            m_alarm.record_fail |= (1 << 14);
        } else {
            m_alarm.record_fail &= ~(1 << 14);
        }
        strcpy(m_httpPrms[EXCEPT_RECORD_FAIL].url, ui->textEditURL->toPlainText().toStdString().c_str());
        strcpy(m_httpPrms[EXCEPT_RECORD_FAIL].username, ui->lineEditUserName->text().toStdString().c_str());
        strcpy(m_httpPrms[EXCEPT_RECORD_FAIL].password, ui->lineEditPassword->text().toStdString().c_str());
        break;
    case EXCEPT_DISK_FAIL:
        if (ui->comboBoxEnable->currentIntData()) {
            m_alarm.disk_fail &= ~(1 << 0);
        } else {
            m_alarm.disk_fail |= (1 << 0);
        }
        if (ui->checkBox_event->isChecked()) {
            m_alarm.disk_fail &= ~(1 << 3);
        } else {
            m_alarm.disk_fail |= (1 << 3);
        }
        if (ui->checkBox_audible->isChecked()) {
            m_alarm.disk_fail |= (1 << 4);
        } else {
            m_alarm.disk_fail &= ~(1 << 4);
        }
        if (ui->checkBox_email->isChecked()) {
            m_alarm.disk_fail |= (1 << 5);
        } else {
            m_alarm.disk_fail &= ~(1 << 5);
        }
        getAlarmOut(m_alarm.disk_fail);
        if (ui->checkBoxHTTP->isChecked()) {
            m_alarm.disk_fail |= (1 << 14);
        } else {
            m_alarm.disk_fail &= ~(1 << 14);
        }
        strcpy(m_httpPrms[EXCEPT_DISK_FAIL].url, ui->textEditURL->toPlainText().toStdString().c_str());
        strcpy(m_httpPrms[EXCEPT_DISK_FAIL].username, ui->lineEditUserName->text().toStdString().c_str());
        strcpy(m_httpPrms[EXCEPT_DISK_FAIL].password, ui->lineEditPassword->text().toStdString().c_str());
        break;
    case EXCEPT_DISK_NO_FORMAT:
        if (ui->comboBoxEnable->currentIntData()) {
            m_alarm.disk_unformat &= ~(1 << 0);
        } else {
            m_alarm.disk_unformat |= (1 << 0);
        }
        if (ui->checkBox_event->isChecked()) {
            m_alarm.disk_unformat &= ~(1 << 3);
        } else {
            m_alarm.disk_unformat |= (1 << 3);
        }
        if (ui->checkBox_audible->isChecked()) {
            m_alarm.disk_unformat |= (1 << 4);
        } else {
            m_alarm.disk_unformat &= ~(1 << 4);
        }
        if (ui->checkBox_email->isChecked()) {
            m_alarm.disk_unformat |= (1 << 5);
        } else {
            m_alarm.disk_unformat &= ~(1 << 5);
        }
        getAlarmOut(m_alarm.disk_unformat);
        if (ui->checkBoxHTTP->isChecked()) {
            m_alarm.disk_unformat |= (1 << 14);
        } else {
            m_alarm.disk_unformat &= ~(1 << 14);
        }
        strcpy(m_httpPrms[EXCEPT_DISK_NO_FORMAT].url, ui->textEditURL->toPlainText().toStdString().c_str());
        strcpy(m_httpPrms[EXCEPT_DISK_NO_FORMAT].username, ui->lineEditUserName->text().toStdString().c_str());
        strcpy(m_httpPrms[EXCEPT_DISK_NO_FORMAT].password, ui->lineEditPassword->text().toStdString().c_str());
        break;
    case EXCEPT_NO_DISK:
        if (ui->comboBoxEnable->currentIntData()) {
            m_alarm.no_disk &= ~(1 << 0);
        } else {
            m_alarm.no_disk |= (1 << 0);
        }
        if (ui->checkBox_event->isChecked()) {
            m_alarm.no_disk &= ~(1 << 3);
        } else {
            m_alarm.no_disk |= (1 << 3);
        }
        if (ui->checkBox_audible->isChecked()) {
            m_alarm.no_disk |= (1 << 4);
        } else {
            m_alarm.no_disk &= ~(1 << 4);
        }
        if (ui->checkBox_email->isChecked()) {
            m_alarm.no_disk |= (1 << 5);
        } else {
            m_alarm.no_disk &= ~(1 << 5);
        }
        getAlarmOut(m_alarm.no_disk);
        if (ui->checkBoxHTTP->isChecked()) {
            m_alarm.no_disk |= (1 << 14);
        } else {
            m_alarm.no_disk &= ~(1 << 14);
        }
        strcpy(m_httpPrms[EXCEPT_NO_DISK].url, ui->textEditURL->toPlainText().toStdString().c_str());
        strcpy(m_httpPrms[EXCEPT_NO_DISK].username, ui->lineEditUserName->text().toStdString().c_str());
        strcpy(m_httpPrms[EXCEPT_NO_DISK].password, ui->lineEditPassword->text().toStdString().c_str());
        break;
    case EXCEPT_IP_CONFLICT:
        if (ui->comboBoxEnable->currentIntData()) {
            m_alarm.ipConflict &= ~(1 << 0);
        } else {
            m_alarm.ipConflict |= (1 << 0);
        }
        if (ui->checkBox_event->isChecked()) {
            m_alarm.ipConflict &= ~(1 << 3);
        } else {
            m_alarm.ipConflict |= (1 << 3);
        }
        if (ui->checkBox_audible->isChecked()) {
            m_alarm.ipConflict |= (1 << 4);
        } else {
            m_alarm.ipConflict &= ~(1 << 4);
        }
        if (ui->checkBox_email->isChecked()) {
            m_alarm.ipConflict |= (1 << 5);
        } else {
            m_alarm.ipConflict &= ~(1 << 5);
        }
        getAlarmOut(m_alarm.ipConflict);
        if (ui->checkBoxHTTP->isChecked()) {
            m_alarm.ipConflict |= (1 << 14);
        } else {
            m_alarm.ipConflict &= ~(1 << 14);
        }
        strcpy(m_httpPrms[EXCEPT_IP_CONFLICT].url, ui->textEditURL->toPlainText().toStdString().c_str());
        strcpy(m_httpPrms[EXCEPT_IP_CONFLICT].username, ui->lineEditUserName->text().toStdString().c_str());
        strcpy(m_httpPrms[EXCEPT_IP_CONFLICT].password, ui->lineEditPassword->text().toStdString().c_str());
        break;
    case EXCEPT_DISK_OFFLINE:
        if (ui->comboBoxEnable->currentIntData()) {
            m_alarm.diskOffline &= ~(1 << 0);
        } else {
            m_alarm.diskOffline |= (1 << 0);
        }
        if (ui->checkBox_event->isChecked()) {
            m_alarm.diskOffline &= ~(1 << 3);
        } else {
            m_alarm.diskOffline |= (1 << 3);
        }
        if (ui->checkBox_audible->isChecked()) {
            m_alarm.diskOffline |= (1 << 4);
        } else {
            m_alarm.diskOffline &= ~(1 << 4);
        }
        if (ui->checkBox_email->isChecked()) {
            m_alarm.diskOffline |= (1 << 5);
        } else {
            m_alarm.diskOffline &= ~(1 << 5);
        }
        getAlarmOut(m_alarm.diskOffline);
        if (ui->checkBoxHTTP->isChecked()) {
            m_alarm.diskOffline |= (1 << 14);
        } else {
            m_alarm.diskOffline &= ~(1 << 14);
        }
        strcpy(m_httpPrms[EXCEPT_DISK_OFFLINE].url, ui->textEditURL->toPlainText().toStdString().c_str());
        strcpy(m_httpPrms[EXCEPT_DISK_OFFLINE].username, ui->lineEditUserName->text().toStdString().c_str());
        strcpy(m_httpPrms[EXCEPT_DISK_OFFLINE].password, ui->lineEditPassword->text().toStdString().c_str());
        break;
    case EXCEPT_DISK_HEAT:
        if (ui->comboBoxEnable->currentIntData()) {
            m_alarm.diskHeat &= ~(1 << 0);
        } else {
            m_alarm.diskHeat |= (1 << 0);
        }
        if (ui->checkBox_event->isChecked()) {
            m_alarm.diskHeat &= ~(1 << 3);
        } else {
            m_alarm.diskHeat |= (1 << 3);
        }
        if (ui->checkBox_audible->isChecked()) {
            m_alarm.diskHeat |= (1 << 4);
        } else {
            m_alarm.diskHeat &= ~(1 << 4);
        }
        if (ui->checkBox_email->isChecked()) {
            m_alarm.diskHeat |= (1 << 5);
        } else {
            m_alarm.diskHeat &= ~(1 << 5);
        }
        getAlarmOut(m_alarm.diskHeat);
        if (ui->checkBoxHTTP->isChecked()) {
            m_alarm.diskHeat |= (1 << 14);
        } else {
            m_alarm.diskHeat &= ~(1 << 14);
        }
        strcpy(m_httpPrms[EXCEPT_DISK_HEAT].url, ui->textEditURL->toPlainText().toStdString().c_str());
        strcpy(m_httpPrms[EXCEPT_DISK_HEAT].username, ui->lineEditUserName->text().toStdString().c_str());
        strcpy(m_httpPrms[EXCEPT_DISK_HEAT].password, ui->lineEditPassword->text().toStdString().c_str());
        break;
    case EXCEPT_DISK_MICROTHERM:
        if (ui->comboBoxEnable->currentIntData()) {
            m_alarm.diskMicrotherm &= ~(1 << 0);
        } else {
            m_alarm.diskMicrotherm |= (1 << 0);
        }
        if (ui->checkBox_event->isChecked()) {
            m_alarm.diskMicrotherm &= ~(1 << 3);
        } else {
            m_alarm.diskMicrotherm |= (1 << 3);
        }
        if (ui->checkBox_audible->isChecked()) {
            m_alarm.diskMicrotherm |= (1 << 4);
        } else {
            m_alarm.diskMicrotherm &= ~(1 << 4);
        }
        if (ui->checkBox_email->isChecked()) {
            m_alarm.diskMicrotherm |= (1 << 5);
        } else {
            m_alarm.diskMicrotherm &= ~(1 << 5);
        }
        getAlarmOut(m_alarm.diskMicrotherm);
        if (ui->checkBoxHTTP->isChecked()) {
            m_alarm.diskMicrotherm |= (1 << 14);
        } else {
            m_alarm.diskMicrotherm &= ~(1 << 14);
        }
        strcpy(m_httpPrms[EXCEPT_DISK_MICROTHERM].url, ui->textEditURL->toPlainText().toStdString().c_str());
        strcpy(m_httpPrms[EXCEPT_DISK_MICROTHERM].username, ui->lineEditUserName->text().toStdString().c_str());
        strcpy(m_httpPrms[EXCEPT_DISK_MICROTHERM].password, ui->lineEditPassword->text().toStdString().c_str());
        break;
    case EXCEPT_DISK_CONNECTION_EXCEPTION:
        if (ui->comboBoxEnable->currentIntData()) {
            m_alarm.diskConnectionException &= ~(1 << 0);
        } else {
            m_alarm.diskConnectionException |= (1 << 0);
        }
        if (ui->checkBox_event->isChecked()) {
            m_alarm.diskConnectionException &= ~(1 << 3);
        } else {
            m_alarm.diskConnectionException |= (1 << 3);
        }
        if (ui->checkBox_audible->isChecked()) {
            m_alarm.diskConnectionException |= (1 << 4);
        } else {
            m_alarm.diskConnectionException &= ~(1 << 4);
        }
        if (ui->checkBox_email->isChecked()) {
            m_alarm.diskConnectionException |= (1 << 5);
        } else {
            m_alarm.diskConnectionException &= ~(1 << 5);
        }
        getAlarmOut(m_alarm.diskConnectionException);
        if (ui->checkBoxHTTP->isChecked()) {
            m_alarm.diskConnectionException |= (1 << 14);
        } else {
            m_alarm.diskConnectionException &= ~(1 << 14);
        }
        strcpy(m_httpPrms[EXCEPT_DISK_CONNECTION_EXCEPTION].url, ui->textEditURL->toPlainText().toStdString().c_str());
        strcpy(m_httpPrms[EXCEPT_DISK_CONNECTION_EXCEPTION].username, ui->lineEditUserName->text().toStdString().c_str());
        strcpy(m_httpPrms[EXCEPT_DISK_CONNECTION_EXCEPTION].password, ui->lineEditPassword->text().toStdString().c_str());
        break;
    case EXCEPT_DISK_STRIKE:
        if (ui->comboBoxEnable->currentIntData()) {
            m_alarm.diskStrike &= ~(1 << 0);
        } else {
            m_alarm.diskStrike |= (1 << 0);
        }
        if (ui->checkBox_event->isChecked()) {
            m_alarm.diskStrike &= ~(1 << 3);
        } else {
            m_alarm.diskStrike |= (1 << 3);
        }
        if (ui->checkBox_audible->isChecked()) {
            m_alarm.diskStrike |= (1 << 4);
        } else {
            m_alarm.diskStrike &= ~(1 << 4);
        }
        if (ui->checkBox_email->isChecked()) {
            m_alarm.diskStrike |= (1 << 5);
        } else {
            m_alarm.diskStrike &= ~(1 << 5);
        }
        getAlarmOut(m_alarm.diskStrike);
        if (ui->checkBoxHTTP->isChecked()) {
            m_alarm.diskStrike |= (1 << 14);
        } else {
            m_alarm.diskStrike &= ~(1 << 14);
        }
        strcpy(m_httpPrms[EXCEPT_DISK_STRIKE].url, ui->textEditURL->toPlainText().toStdString().c_str());
        strcpy(m_httpPrms[EXCEPT_DISK_STRIKE].username, ui->lineEditUserName->text().toStdString().c_str());
        strcpy(m_httpPrms[EXCEPT_DISK_STRIKE].password, ui->lineEditPassword->text().toStdString().c_str());
        break;
    default:
        break;
    }
}

void PageException::onAlarmOutCheckChanged()
{
    int alarmoutCount = qMsNvr->maxAlarmOutput();
    int checkedCount = 0;

    if (ui->checkBox_alarmout1->isVisible() && ui->checkBox_alarmout1->isChecked()) {
        checkedCount++;
    }
    if (ui->checkBox_alarmout2->isVisible() && ui->checkBox_alarmout2->isChecked()) {
        checkedCount++;
    }
    if (ui->checkBox_alarmout3->isVisible() && ui->checkBox_alarmout3->isChecked()) {
        checkedCount++;
    }
    if (ui->checkBox_alarmout4->isVisible() && ui->checkBox_alarmout4->isChecked()) {
        checkedCount++;
    }
    if (checkedCount == 0) {
        ui->checkBox_alarmoutAll->setCheckState(Qt::Unchecked);
    } else if (checkedCount == alarmoutCount) {
        ui->checkBox_alarmoutAll->setCheckState(Qt::Checked);
    } else {
        ui->checkBox_alarmoutAll->setCheckState(Qt::PartiallyChecked);
    }

    onSaveSetting();
}

void PageException::onAlarmOutAllClicked(bool checked)
{
    if (ui->checkBox_alarmout1->isVisible())
        ui->checkBox_alarmout1->setChecked(checked);
    if (ui->checkBox_alarmout2->isVisible())
        ui->checkBox_alarmout2->setChecked(checked);
    if (ui->checkBox_alarmout3->isVisible())
        ui->checkBox_alarmout3->setChecked(checked);
    if (ui->checkBox_alarmout4->isVisible())
        ui->checkBox_alarmout4->setChecked(checked);

    if (!checked) {
        ui->checkBox_alarmoutAll->setCheckState(Qt::Unchecked);
    } else {
        ui->checkBox_alarmoutAll->setCheckState(Qt::Checked);
    }
    onSaveSetting();
}

void PageException::on_comboBox_exceptionType_activated(int index)
{
    ui->checkBox_email->setEnabled(true);
    ui->checkBox_audible->setEnabled(true);
    ui->widget_email_interval->setVisible(false);
    ui->checkBoxHTTP->setEnabled(true);

    const except_event &event = static_cast<except_event>(ui->comboBox_exceptionType->itemData(index).toInt());
    switch (event) {
    case EXCEPT_NETWORK_DISCONN:
        ui->checkBox_email->setEnabled(false);
        ui->checkBoxHTTP->setEnabled(false);
        ui->comboBoxEnable->setCurrentIndexFromData(!((m_alarm.network_disconn >> 0) & 0x01));
        ui->checkBox_event->setChecked(!((m_alarm.network_disconn >> 3) & 0x01));
        ui->checkBox_audible->setChecked((m_alarm.network_disconn >> 4) & 0x01);
        ui->checkBox_email->setChecked((m_alarm.network_disconn >> 5) & 0x01);
        ui->checkBoxHTTP->setChecked((m_alarm.network_disconn >> 14) & 0x01);
        ui->textEditURL->setText(m_httpPrms[EXCEPT_NETWORK_DISCONN].url);
        ui->lineEditUserName->setText(m_httpPrms[EXCEPT_NETWORK_DISCONN].username);
        ui->lineEditPassword->setText(m_httpPrms[EXCEPT_NETWORK_DISCONN].password);
        showAlarmOut(m_alarm.network_disconn);
        break;
    case EXCEPT_DISK_FULL:
        ui->comboBoxEnable->setCurrentIndexFromData(!((m_alarm.disk_full >> 0) & 0x01));
        ui->checkBox_event->setChecked(!((m_alarm.disk_full >> 3) & 0x01));
        ui->checkBox_audible->setChecked((m_alarm.disk_full >> 4) & 0x01);
        ui->checkBox_email->setChecked((m_alarm.disk_full >> 5) & 0x01);
        ui->checkBoxHTTP->setChecked((m_alarm.disk_full >> 14) & 0x01);
        ui->textEditURL->setText(m_httpPrms[EXCEPT_DISK_FULL].url);
        ui->lineEditUserName->setText(m_httpPrms[EXCEPT_DISK_FULL].username);
        ui->lineEditPassword->setText(m_httpPrms[EXCEPT_DISK_FULL].password);
        showAlarmOut(m_alarm.disk_full);
        break;
    case EXCEPT_RECORD_FAIL:
        ui->comboBoxEnable->setCurrentIndexFromData(!((m_alarm.record_fail >> 0) & 0x01));
        ui->checkBox_event->setChecked(!((m_alarm.record_fail >> 3) & 0x01));
        ui->checkBox_audible->setChecked((m_alarm.record_fail >> 4) & 0x01);
        ui->checkBox_email->setChecked((m_alarm.record_fail >> 5) & 0x01);
        ui->comboBox_email_interval->setCurrentIndexFromData(m_alarm.record_mail_interval);
        ui->widget_email_interval->setVisible(true);
        ui->checkBoxHTTP->setChecked((m_alarm.record_fail >> 14) & 0x01);
        ui->textEditURL->setText(m_httpPrms[EXCEPT_RECORD_FAIL].url);
        ui->lineEditUserName->setText(m_httpPrms[EXCEPT_RECORD_FAIL].username);
        ui->lineEditPassword->setText(m_httpPrms[EXCEPT_RECORD_FAIL].password);
        showAlarmOut(m_alarm.record_fail);
        break;
    case EXCEPT_DISK_FAIL:
        ui->comboBoxEnable->setCurrentIndexFromData(!((m_alarm.disk_fail >> 0) & 0x01));
        ui->checkBox_event->setChecked(!((m_alarm.disk_fail >> 3) & 0x01));
        ui->checkBox_audible->setChecked((m_alarm.disk_fail >> 4) & 0x01);
        ui->checkBox_email->setChecked((m_alarm.disk_fail >> 5) & 0x01);
        ui->checkBoxHTTP->setChecked((m_alarm.disk_fail >> 14) & 0x01);
        ui->textEditURL->setText(m_httpPrms[EXCEPT_DISK_FAIL].url);
        ui->lineEditUserName->setText(m_httpPrms[EXCEPT_DISK_FAIL].username);
        ui->lineEditPassword->setText(m_httpPrms[EXCEPT_DISK_FAIL].password);
        showAlarmOut(m_alarm.disk_fail);
        break;
    case EXCEPT_DISK_NO_FORMAT:
        ui->comboBoxEnable->setCurrentIndexFromData(!((m_alarm.disk_unformat >> 0) & 0x01));
        ui->checkBox_event->setChecked(!((m_alarm.disk_unformat >> 3) & 0x01));
        ui->checkBox_audible->setChecked((m_alarm.disk_unformat >> 4) & 0x01);
        ui->checkBox_email->setChecked((m_alarm.disk_unformat >> 5) & 0x01);
        ui->checkBoxHTTP->setChecked((m_alarm.disk_unformat >> 14) & 0x01);
        ui->textEditURL->setText(m_httpPrms[EXCEPT_DISK_NO_FORMAT].url);
        ui->lineEditUserName->setText(m_httpPrms[EXCEPT_DISK_NO_FORMAT].username);
        ui->lineEditPassword->setText(m_httpPrms[EXCEPT_DISK_NO_FORMAT].password);
        showAlarmOut(m_alarm.disk_unformat);
        break;
    case EXCEPT_NO_DISK:
        ui->comboBoxEnable->setCurrentIndexFromData(!((m_alarm.no_disk >> 0) & 0x01));
        ui->checkBox_event->setChecked(!((m_alarm.no_disk >> 3) & 0x01));
        ui->checkBox_audible->setChecked((m_alarm.no_disk >> 4) & 0x01);
        ui->checkBox_email->setChecked((m_alarm.no_disk >> 5) & 0x01);
        ui->checkBoxHTTP->setChecked((m_alarm.no_disk >> 14) & 0x01);
        ui->textEditURL->setText(m_httpPrms[EXCEPT_NO_DISK].url);
        ui->lineEditUserName->setText(m_httpPrms[EXCEPT_NO_DISK].username);
        ui->lineEditPassword->setText(m_httpPrms[EXCEPT_NO_DISK].password);
        showAlarmOut(m_alarm.no_disk);
        break;
    case EXCEPT_IP_CONFLICT:
        ui->checkBox_email->setEnabled(false);
        ui->checkBoxHTTP->setEnabled(false);
        ui->comboBoxEnable->setCurrentIndexFromData(!((m_alarm.ipConflict >> 0) & 0x01));
        ui->checkBox_event->setChecked(!((m_alarm.ipConflict >> 3) & 0x01));
        ui->checkBox_audible->setChecked((m_alarm.ipConflict >> 4) & 0x01);
        ui->checkBox_email->setChecked((m_alarm.ipConflict >> 5) & 0x01);
        ui->checkBoxHTTP->setChecked((m_alarm.ipConflict >> 14) & 0x01);
        ui->textEditURL->setText(m_httpPrms[EXCEPT_IP_CONFLICT].url);
        ui->lineEditUserName->setText(m_httpPrms[EXCEPT_IP_CONFLICT].username);
        ui->lineEditPassword->setText(m_httpPrms[EXCEPT_IP_CONFLICT].password);
        showAlarmOut(m_alarm.ipConflict);
        break;
    case EXCEPT_DISK_OFFLINE:
        ui->comboBoxEnable->setCurrentIndexFromData(!((m_alarm.diskOffline >> 0) & 0x01));
        ui->checkBox_event->setChecked(!((m_alarm.diskOffline >> 3) & 0x01));
        ui->checkBox_audible->setChecked((m_alarm.diskOffline >> 4) & 0x01);
        ui->checkBox_email->setChecked((m_alarm.diskOffline >> 5) & 0x01);
        ui->checkBoxHTTP->setChecked((m_alarm.diskOffline >> 14) & 0x01);
        ui->textEditURL->setText(m_httpPrms[EXCEPT_DISK_OFFLINE].url);
        ui->lineEditUserName->setText(m_httpPrms[EXCEPT_DISK_OFFLINE].username);
        ui->lineEditPassword->setText(m_httpPrms[EXCEPT_DISK_OFFLINE].password);
        showAlarmOut(m_alarm.diskOffline);
        break;
    case EXCEPT_DISK_HEAT:
        ui->comboBoxEnable->setCurrentIndexFromData(!((m_alarm.diskHeat >> 0) & 0x01));
        ui->checkBox_event->setChecked(!((m_alarm.diskHeat >> 3) & 0x01));
        ui->checkBox_audible->setChecked((m_alarm.diskHeat >> 4) & 0x01);
        ui->checkBox_email->setChecked((m_alarm.diskHeat >> 5) & 0x01);
        ui->checkBoxHTTP->setChecked((m_alarm.diskHeat >> 14) & 0x01);
        ui->textEditURL->setText(m_httpPrms[EXCEPT_DISK_HEAT].url);
        ui->lineEditUserName->setText(m_httpPrms[EXCEPT_DISK_HEAT].username);
        ui->lineEditPassword->setText(m_httpPrms[EXCEPT_DISK_HEAT].password);
        showAlarmOut(m_alarm.diskHeat);
        break;
    case EXCEPT_DISK_MICROTHERM:
        ui->comboBoxEnable->setCurrentIndexFromData(!((m_alarm.diskMicrotherm >> 0) & 0x01));
        ui->checkBox_event->setChecked(!((m_alarm.diskMicrotherm >> 3) & 0x01));
        ui->checkBox_audible->setChecked((m_alarm.diskMicrotherm >> 4) & 0x01);
        ui->checkBox_email->setChecked((m_alarm.diskMicrotherm >> 5) & 0x01);
        ui->checkBoxHTTP->setChecked((m_alarm.diskMicrotherm >> 14) & 0x01);
        ui->textEditURL->setText(m_httpPrms[EXCEPT_DISK_MICROTHERM].url);
        ui->lineEditUserName->setText(m_httpPrms[EXCEPT_DISK_MICROTHERM].username);
        ui->lineEditPassword->setText(m_httpPrms[EXCEPT_DISK_MICROTHERM].password);
        showAlarmOut(m_alarm.diskMicrotherm);
        break;
    case EXCEPT_DISK_CONNECTION_EXCEPTION:
        ui->comboBoxEnable->setCurrentIndexFromData(!((m_alarm.diskConnectionException >> 0) & 0x01));
        ui->checkBox_event->setChecked(!((m_alarm.diskConnectionException >> 3) & 0x01));
        ui->checkBox_audible->setChecked((m_alarm.diskConnectionException >> 4) & 0x01);
        ui->checkBox_email->setChecked((m_alarm.diskConnectionException >> 5) & 0x01);
        ui->checkBoxHTTP->setChecked((m_alarm.diskConnectionException >> 14) & 0x01);
        ui->textEditURL->setText(m_httpPrms[EXCEPT_DISK_CONNECTION_EXCEPTION].url);
        ui->lineEditUserName->setText(m_httpPrms[EXCEPT_DISK_CONNECTION_EXCEPTION].username);
        ui->lineEditPassword->setText(m_httpPrms[EXCEPT_DISK_CONNECTION_EXCEPTION].password);
        showAlarmOut(m_alarm.diskConnectionException);
        break;
    case EXCEPT_DISK_STRIKE:
        ui->comboBoxEnable->setCurrentIndexFromData(!((m_alarm.diskStrike >> 0) & 0x01));
        ui->checkBox_event->setChecked(!((m_alarm.diskStrike >> 3) & 0x01));
        ui->checkBox_audible->setChecked((m_alarm.diskStrike >> 4) & 0x01);
        ui->checkBox_email->setChecked((m_alarm.diskStrike >> 5) & 0x01);
        ui->checkBoxHTTP->setChecked((m_alarm.diskStrike >> 14) & 0x01);
        ui->textEditURL->setText(m_httpPrms[EXCEPT_DISK_STRIKE].url);
        ui->lineEditUserName->setText(m_httpPrms[EXCEPT_DISK_STRIKE].username);
        ui->lineEditPassword->setText(m_httpPrms[EXCEPT_DISK_STRIKE].password);
        showAlarmOut(m_alarm.diskStrike);
        break;
    default:
        break;
    }
    on_comboBoxEnable_activated(0);
}

void PageException::on_pushButton_apply_clicked()
{
    qDebug() << QString("Exception::on_pushButton_apply_clicked() 1111");
    write_trigger_alarms(SQLITE_FILE_NAME, &m_alarm);
    write_http_notification_params_batch(SQLITE_FILE_NAME, m_httpPrms, EXCEPT_HTTP_PARAMS, EXCEPT_COUNT);
    qDebug() << QString("Exception::on_pushButton_apply_clicked() 2222");
    sendMessageOnly(REQUEST_FLAG_SET_TRIGGER_ALARMS, nullptr, 0);
    qDebug() << QString("Exception::on_pushButton_apply_clicked() 3333");
    sendMessageOnly(REQUEST_FLAG_SET_EXCEPT_HTTP_PARAMS, nullptr, 0);
}

void PageException::on_pushButton_back_clicked()
{
    emit sig_back();
}

void PageException::on_comboBoxAudioFile_activated(int index)
{
    Q_UNUSED(index)
    onSaveSetting();
}

void PageException::on_checkBoxHTTP_clicked()
{
    bool checked = ui->checkBoxHTTP->isChecked() && ui->checkBoxHTTP->isEnabled();
    ui->textEditURL->setReadOnly(!checked);
    if (!checked) {
        ui->textEditURL->setStyleSheet("background-color: rgb(190, 190, 190);color: #4A4A4A;");
    } else {
        ui->textEditURL->setStyleSheet("background-color: #FFFFFF;color: #4A4A4A;");
    }
    ui->lineEditUserName->setEnabled(checked);
    ui->lineEditPassword->setEnabled(checked);
}

void PageException::on_textEditURL_textChanged()
{
    QString textContent = ui->textEditURL->toPlainText();
    int length = textContent.count();
    int maxLength = MAX_LEN_512;

    if (length > maxLength) {
        int position = ui->textEditURL->textCursor().position();
        QTextCursor textCursor = ui->textEditURL->textCursor();
        textContent.remove(position - (length - maxLength), length - maxLength);
        ui->textEditURL->setText(textContent);
        textCursor.setPosition(position - (length - maxLength));
        ui->textEditURL->setTextCursor(textCursor);
    }
}

void PageException::on_comboBoxEnable_activated(int index)
{
    Q_UNUSED(index)
    ui->groupBox->setEnabled(ui->comboBoxEnable->currentIntData());
    on_checkBoxHTTP_clicked();
    onSaveSetting();
}
