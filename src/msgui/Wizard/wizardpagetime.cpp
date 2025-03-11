#include "wizardpagetime.h"
#include "ui_wizardpagetime.h"
#include "GeneralSetting.h"
#include "centralmessage.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "myqt.h"

extern "C" {
#include "msg.h"
}

WizardPageTime::WizardPageTime(QWidget *parent)
    : AbstractWizardPage(parent)
    , ui(new Ui::WizardPageTime)
{
    ui->setupUi(this);

    //time zone
    ui->comboBox_zone->clear();
    const auto &zoneList = GeneralSetting::timezoneMap();
    for (int i = 0; i < zoneList.size(); ++i) {
        const QPair<QString, QString> &pair = zoneList.at(i);
        ui->comboBox_zone->addItem(pair.first, pair.second);
    }
    memset(&m_dbTime, 0, sizeof(m_dbTime));
    read_time(SQLITE_FILE_NAME, &m_dbTime);
    const QString &strTimezone = QString("%1 %2").arg(m_dbTime.time_zone).arg(m_dbTime.time_zone_name);
    ui->comboBox_zone->setCurrentIndexFromData(strTimezone);
    //daylight
    ui->comboBox_daylight->clear();
    ui->comboBox_daylight->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBox_daylight->addItem(GET_TEXT("COMMON/1014", "Auto"), 1);
    ui->comboBox_daylight->setCurrentIndexFromData(m_dbTime.dst_enable);
    //time setting
    ui->checkBox_synchronize->setChecked(m_dbTime.ntp_enable);
    on_checkBox_synchronize_clicked(m_dbTime.ntp_enable);
    //ntp server
    ui->lineEdit_ntpServer->setText(QString(m_dbTime.ntp_server));
    //manual date and time
    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

    ui->lineEdit_ntpServer->setCheckMode(MyLineEdit::ServerCheck);

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

WizardPageTime::~WizardPageTime()
{
    delete ui;
}

void WizardPageTime::saveSetting()
{
    if (ui->checkBox_synchronize->isChecked()) {
        if (!ui->lineEdit_ntpServer->checkValid()) {
            return;
        }
    }
    //time
    bool needWait = false;
    struct time dbTime;
    memcpy(&dbTime, &m_dbTime, sizeof(struct time));
    QString strTimeZone = ui->comboBox_zone->currentData().toString();
    QStringList strTimeZoneList = strTimeZone.split(" ");
    if (strTimeZoneList.size() == 2) {
        snprintf(dbTime.time_zone, sizeof(dbTime.time_zone), "%s", strTimeZoneList.at(0).toStdString().c_str());
        snprintf(dbTime.time_zone_name, sizeof(dbTime.time_zone_name), "%s", strTimeZoneList.at(1).toStdString().c_str());
    }
    dbTime.dst_enable = ui->comboBox_daylight->currentData().toInt();
    dbTime.ntp_enable = ui->checkBox_synchronize->isChecked();
    snprintf(dbTime.ntp_server, sizeof(dbTime.ntp_server), "%s", ui->lineEdit_ntpServer->text().toStdString().c_str());

    struct req_set_sysconf set_sysconf;
    memset(&set_sysconf, 0, sizeof(set_sysconf));
    if (memcmp(&dbTime, &m_dbTime, sizeof(struct time)) != 0) {
        write_time(SQLITE_FILE_NAME, &dbTime);

        //manual time
        if (ui->checkBox_manually->isChecked()) {
            const QString &strTime = ui->dateTimeEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss");
            snprintf(set_sysconf.arg, sizeof(set_sysconf.arg), "\"%s\"", strTime.toStdString().c_str());
        }
        sendMessage(REQUEST_FLAG_SET_SYSTIME, &set_sysconf, sizeof(set_sysconf));
        needWait = true;
    } else {
        if (ui->checkBox_manually->isChecked()) {
            const QString &strTime = ui->dateTimeEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss");
            snprintf(set_sysconf.arg, sizeof(set_sysconf.arg), "\"%s\"", strTime.toStdString().c_str());
            sendMessage(REQUEST_FLAG_SET_SYSTIME, &set_sysconf, sizeof(set_sysconf));
            needWait = true;
        }
    }
    if (needWait) {
        m_waitting->execWait();
    }
    //
    showWizardPage(Wizard_NetWork);
}

void WizardPageTime::previousPage()
{
    showWizardPage(Wizard_User);
}

void WizardPageTime::nextPage()
{
    saveSetting();
}

void WizardPageTime::skipWizard()
{
    AbstractWizardPage::skipWizard();
}

void WizardPageTime::initializeData()
{
    read_time(SQLITE_FILE_NAME, &m_dbTime);
    const QString &strTimezone = QString("%1 %2").arg(m_dbTime.time_zone).arg(m_dbTime.time_zone_name);
    ui->comboBox_zone->setCurrentIndexFromData(strTimezone);
    //daylight
    ui->comboBox_daylight->setCurrentIndexFromData(m_dbTime.dst_enable);
    //time setting
    ui->checkBox_synchronize->setChecked(m_dbTime.ntp_enable);
    on_checkBox_synchronize_clicked(m_dbTime.ntp_enable);
    //ntp server
    ui->lineEdit_ntpServer->setText(QString(m_dbTime.ntp_server));
    //manual date and time
    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
}

void WizardPageTime::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SET_SYSTIME:
        ON_RESPONSE_FLAG_SET_SYSTIME(message);
        break;
    }
}

void WizardPageTime::ON_RESPONSE_FLAG_SET_SYSTIME(MessageReceive *message)
{
    Q_UNUSED(message)

    //m_waitting->//closeWait();
}

void WizardPageTime::onLanguageChanged()
{
    ui->label_zone->setText(GET_TEXT("WIZARD/11014", "Time Zone"));
    ui->label_daylight->setText(GET_TEXT("WIZARD/11016", "Daylight Saving Time"));
    ui->checkBox_synchronize->setText(GET_TEXT("WIZARD/11017", "Synchronize with NTP server"));
    ui->label_ntpServer->setText(GET_TEXT("WIZARD/11018", "NTP Server"));
    ui->checkBox_manually->setText(GET_TEXT("WIZARD/11019", "Set Date and Time Manually"));
    ui->label_dateTime->setText(GET_TEXT("WIZARD/11020", "Set Date and Time"));
}

void WizardPageTime::on_checkBox_synchronize_clicked(bool checked)
{
    ui->label_ntpServer->setEnabled(checked);
    ui->lineEdit_ntpServer->setEnabled(checked);

    ui->checkBox_manually->setChecked(!checked);
    ui->label_dateTime->setEnabled(!checked);
    ui->dateTimeEdit->setEnabled(!checked);
}

void WizardPageTime::on_checkBox_manually_clicked(bool checked)
{
    ui->label_dateTime->setEnabled(checked);
    ui->dateTimeEdit->setEnabled(checked);

    ui->checkBox_synchronize->setChecked(!checked);
    ui->label_ntpServer->setEnabled(!checked);
    ui->lineEdit_ntpServer->setEnabled(!checked);
}
