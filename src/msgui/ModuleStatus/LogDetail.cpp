#include "LogDetail.h"
#include "msfs_log.h"
#include "ui_LogDetail.h"
#include "LogWrite.h"
#include "centralmessage.h"
#include "PageLogStatus.h"
#include "MsLanguage.h"

LogDetail::LogDetail(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::LogDetail)
{
    ui->setupUi(this);

    onLanguageChanged();
}

LogDetail::~LogDetail()
{
    delete ui;
}

void LogDetail::setDetail(const QString &str)
{
    if (str.isEmpty()) {
        ui->textEdit_information->setText(GET_TEXT("LOG/64076", "N/A"));
    } else {
        ui->textEdit_information->setText(str);
    }
}

void LogDetail::showLog(const log_data &log)
{
    QDateTime dateTime = QDateTime::fromTime_t(log.log_data_info.pts);
    ui->lineEdit_time->setText(dateTime.toString("yyyy-MM-dd HH:mm:ss"));
    QString strType = QString("%1-%2").arg(PageLogStatus::mainTypeString(log.log_data_info.mainType)).arg(PageLogStatus::subTypeString(log.log_data_info.subType));
    ui->lineEdit_type->setText(strType);
    QString strUser = QString(log.log_data_info.user);
    if (strUser.isEmpty()) {
        strUser = GET_TEXT("LOG/64076", "N/A");
    }
    ui->lineEdit_user->setText(strUser);
    //ip
    QString strIP;
    char ip[24] = { 0 };
    snprintf(ip, sizeof(ip), "%s", "0.0.0.0");
    if (strlen(log.log_data_info.ip) == 0) {
        strIP = QString("%1").arg(GET_TEXT("LOG/64076", "N/A"));
    } else if (strncmp(log.log_data_info.ip, "::ffff:", 7) == 0 && strchr(log.log_data_info.ip, '.')) //hrz.milesight
    {
        strIP = QString("%1").arg(log.log_data_info.ip + 7);
    } else if (QString(log.log_data_info.ip).contains("%")) {
        QStringList strList = QString(log.log_data_info.ip).split("%");
        strIP = QString("%1").arg(strList.first());
    } else if (!memcmp(log.log_data_info.ip, ip, sizeof(ip))) {
        strIP = QString("%1").arg(GET_TEXT("LOG/64011", "Localhost"));
    } else {
        strIP = QString("%1").arg(log.log_data_info.ip);
    }
    ui->lineEdit_ip->setText(strIP);
    if (isDiskType(log.log_data_info.mainType, log.log_data_info.subType)) {
        ui->label_number->setText(GET_TEXT("LOG/64146", "Disk No."));
    } else {
        ui->label_number->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));
    }
    if (log.log_data_info.chan_no > 0) {
        ui->lineEdit_number->setText(QString::number(log.log_data_info.chan_no));
    } else {
        ui->lineEdit_number->setText(GET_TEXT("LOG/64076", "N/A"));
    }
    //
    QString strParameters = PageLogStatus::subTypeString(log.log_data_info.parameter_type);
    ui->lineEdit_parameters->setText(strParameters);

    show();
}

bool LogDetail::isDiskType(int mainType, int subType)
{
    bool isDisk = false;

    switch (mainType) {
    case MAIN_OP:
        switch (subType) {
        case SUB_OP_DISK_INIT_LOCAL:
        case SUP_OP_ADD_NETWORK_DISK_LOCK:
        case SUP_OP_DELETE_NETWORK_DISK_LOCK:
        case SUP_OP_CONFIG_NETWORK_DISK_LOCK:
        case SUP_OP_PHYSICAL_DISK_SELF_CHECK_LOCK:
        case SUP_OP_MOUNT_DISK_LOCK:
        case SUP_OP_UNMOUNT_DISK_LOCK:
        case SUP_OP_DELETE_ABNORMAL_LOCK:
        case SUB_OP_DISK_INIT_REMOTE:
        case SUP_OP_ADD_NETWORK_DISK_REMOTE:
        case SUP_OP_DELETE_NETWORK_DISK_REMOTE:
        case SUP_OP_CONFIG_NETWORK_DISK_REMOTE:
        case SUP_OP_MOUNT_DISK_REMOTE:
        case SUP_OP_UNMOUNT_DISK_REMOTE:
        case SUP_OP_DELETE_ABNORMAL_DISK_REMOTE:
            isDisk = true;
            break;
        default:
            break;
        }
        break;
    case MAIN_INFO:
        switch (subType) {
        case SUB_INFO_DISK_INFO:
        case SUP_INFO_NETWORK_DISK_INFORMATION:
        case SUB_INFO_DISK_SMART:
            isDisk = true;
            break;
        default:
            break;
        }
        break;
    case MAIN_EXCEPT:
        switch (subType) {
        case SUB_EXCEPT_DISK_FULL:
        case SUB_EXCEPT_DISK_FAILURE:
        case SUB_EXCEPT_DISK_UNINITIALIZED:
        case SUB_EXCEPT_DISK_OFFLINE:
        case SUB_EXCEPT_DISK_HEAT:
        case SUB_EXCEPT_DISK_MICROTHERM:
        case SUB_EXCEPT_DISK_CONNECTION_EXCEPTION:
        case SUB_EXCEPT_DISK_DISK_STRIKE:
            isDisk = true;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return isDisk;
}

void LogDetail::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("LOG/64139", "Details"));
    ui->label_time->setText(GET_TEXT("LOG/64005", "Time"));
    ui->label_type->setText(GET_TEXT("COMMON/1052", "Type"));
    ui->label_user->setText(GET_TEXT("LOG/64140", "Local User"));
    ui->label_ip->setText(GET_TEXT("LOG/64141", "Remote Host IP"));
    ui->label_parameters->setText(GET_TEXT("LOG/64006", "Parameter"));
    ui->label_information->setText(GET_TEXT("LOG/64143", "Log Information"));

    ui->pushButton_previous->setText(GET_TEXT("LOG/64144", "Previous"));
    ui->pushButton_next->setText(GET_TEXT("COMMONBACKUP/100048", "Next "));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
}

void LogDetail::on_pushButton_previous_clicked()
{
    emit previousLog();
}

void LogDetail::on_pushButton_next_clicked()
{
    emit nextLog();
}

void LogDetail::on_pushButton_back_clicked()
{
    close();
}
