#include "PageDeviceInformation.h"
#include "ui_PageDeviceInformation.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "centralmessage.h"
#include <QFile>
#include <QProcess>
#include <QRegExp>
#include <QtDebug>

extern "C" {
#include "msstd.h"
}

PageDeviceInformation::PageDeviceInformation(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::DeviceInformation)
{
    ui->setupUi(this);

    ui->label_model->setText(GET_TEXT("CHANNELMANAGE/30029", "Model"));
    ui->label_mac->setText(GET_TEXT("DEVICEINFO/60006", "MAC Address"));
    ui->label_hardware->setText(GET_TEXT("DEVICEINFO/60003", "Hardware Version"));
    ui->label_software->setText(GET_TEXT("DEVICEINFO/60004", "Software Version"));
    ui->label_uptime->setText(GET_TEXT("DEVICEINFO/60005", "Uptime"));

    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->labelSN->setText(GET_TEXT("CAMERA/143001", "SN Address"));
}

PageDeviceInformation::~PageDeviceInformation()
{
    delete ui;
}

int PageDeviceInformation::get_uptime(char *uptime, int size)
{
    FILE *fp = ms_vpopen("uptime", "r");
    if (!fp) {
        return -1;
    }
    fread(uptime, sizeof(char), size, fp);
    ms_vpclose(fp);

    char *com = strstr(uptime, "users");
    if (!com) {
        com = strchr(uptime, 'l');
        if (!com)
            return -1;
        com = com - 2;
        *com = '\0';
    } else {
        com = com - 5;
        *com = '\0';
    }

    return 0;
}

void PageDeviceInformation::initializeData()
{
    //model
    ui->lineEdit_model->setText(qMsNvr->model());

    //mac
    char mac[32] = { 0 };
    read_mac_conf(mac);
    QString strMac;
    QString tempMac(mac);
    if (tempMac.isEmpty()) {
        strMac = qMsNvr->macList().first();
    } else {
        for (int i = 0; i < tempMac.size(); i += 2) {
            strMac.append(tempMac.mid(i, 2));
            if (i < tempMac.size() - 2) {
                strMac.append(":");
            }
        }
    }
    ui->lineEdit_mac->setText(strMac);

    //hadrware
    ui->lineEdit_hardware->setText(qMsNvr->hardwareVersion());

    //software
    ui->lineEdit_software->setText(qMsNvr->softwareVersion());

    //uptime
    char uptime[64] = { 0 };
    qDebug() << QString("DeviceInformation::initializeData, begin get uptime.");
    get_uptime(uptime, sizeof(uptime));
    qDebug() << QString("DeviceInformation::initializeData, end get uptime.");
    ui->lineEdit_uptime->setText(QString(uptime).simplified());

    sendMessage(REQUEST_FLAG_GET_SYSINFO, nullptr, 0);
}

void PageDeviceInformation::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_SYSINFO:
        ON_RESPONSE_FLAG_GET_SYSINFO(message);
        break;
    }
}

void PageDeviceInformation::ON_RESPONSE_FLAG_GET_SYSINFO(MessageReceive *message)
{
    if (!message->data) {
        return;
    }
    device_info *dbInfo = static_cast<device_info *>(message->data);
    ui->lineEditSN->setText(dbInfo->sncode);
}

void PageDeviceInformation::on_pushButton_back_clicked()
{
    emit sig_back();
}
