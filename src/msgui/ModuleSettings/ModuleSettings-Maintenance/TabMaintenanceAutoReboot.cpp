#include "TabMaintenanceAutoReboot.h"
#include "ui_TabMaintenanceAutoReboot.h"
#include "centralmessage.h"
#include "MessageBox.h"
#include "MsLanguage.h"
#include <QtDebug>

TabMaintenanceAutoReboot::TabMaintenanceAutoReboot(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::TabMaintenanceAutoReboot)
{
    ui->setupUi(this);

    ui->comboBox_enable->clear();
    ui->comboBox_enable->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBox_enable->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);

    ui->comboBox_day->clear();
    ui->comboBox_day->addItem(GET_TEXT("AUTOREBOOT/78001", "Everyday"), EVERYDAY);
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1024", "Sunday"), SUNDAY);
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1025", "Monday"), MONDAY);
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1026", "Tuesday"), TUESDAY);
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1027", "Wednesday"), WEDNESDAY);
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1028", "Thursday"), THURSDAY);
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1029", "Friday"), FRIDAY);
    ui->comboBox_day->addItem(GET_TEXT("COMMON/1030", "Saturday"), SATURDAY);
    onLanguageChanged();
}

TabMaintenanceAutoReboot::~TabMaintenanceAutoReboot()
{
    delete ui;
}

void TabMaintenanceAutoReboot::initializeData()
{
    qDebug() << "TabMaintenanceAutoReboot:initializeData:sizeof(reboot_conf) 0000:" << sizeof(reboot_conf);
    memset(&m_reboot_conf, 0, sizeof(reboot_conf));
    read_autoreboot_conf(SQLITE_FILE_NAME, &m_reboot_conf);

    ui->comboBox_enable->setCurrentIndexFromData(m_reboot_conf.enable);
    ui->comboBox_day->setCurrentIndexFromData(m_reboot_conf.wday);
    ui->timeEdit_time->setTime(QTime(m_reboot_conf.hour, m_reboot_conf.minutes, m_reboot_conf.seconds));

    qDebug() << "TabMaintenanceAutoReboot:initializeData:enable:" << m_reboot_conf.enable << "; day:" << m_reboot_conf.wday;
    qDebug() << "TabMaintenanceAutoReboot:initializeData:time:hour" << m_reboot_conf.hour << "; minutes:" << m_reboot_conf.minutes << "; seconds:" << m_reboot_conf.seconds;
    qDebug() << "TabMaintenanceAutoReboot:initializeData:settime:" << QTime(m_reboot_conf.hour, m_reboot_conf.minutes, m_reboot_conf.seconds);
}

void TabMaintenanceAutoReboot::onLanguageChanged()
{
    ui->label_autoReboot->setText(GET_TEXT("AUTOREBOOT/78000", "Auto Reboot"));
    ui->label_day->setText(GET_TEXT("AUTOREBOOT/78009", "Day"));
    ui->label_time->setText(GET_TEXT("AUTOREBOOT/78010", "Time"));

    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
}

void TabMaintenanceAutoReboot::on_pushButton_apply_clicked()
{

    m_reboot_conf.enable = ui->comboBox_enable->currentData().toInt();
    m_reboot_conf.wday = static_cast<DAY_INTERVAL>(ui->comboBox_day->currentData().toInt());
    const QTime &time = ui->timeEdit_time->time();
    m_reboot_conf.hour = time.hour();
    m_reboot_conf.minutes = time.minute();
    m_reboot_conf.seconds = time.second();
    write_autoreboot_conf(SQLITE_FILE_NAME, &m_reboot_conf);

    sendMessage(REQUEST_FLAG_SET_AUTO_REBOOT, NULL, 0);
}

void TabMaintenanceAutoReboot::on_pushButton_back_clicked()
{
    emit sig_back();
}
