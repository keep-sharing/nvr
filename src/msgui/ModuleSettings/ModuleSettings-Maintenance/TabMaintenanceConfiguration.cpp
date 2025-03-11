#include "TabMaintenanceConfiguration.h"
#include "ui_TabMaintenanceConfiguration.h"
#include "MessageBox.h"
#include "MessageFilter.h"
#include "MsApplication.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MyFileSystemDialog.h"
#include "centralmessage.h"
#include "msuser.h"
#include <QDir>

extern "C" {
#include "log.h"
}

TabMaintenanceConfiguration::TabMaintenanceConfiguration(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::TabMaintenanceConfiguration)
{
    ui->setupUi(this);

    gMessageFilter.installMessageFilter(RESPONSE_FLAG_UDISK_OFFLINE, this);
    onLanguageChanged();
}

TabMaintenanceConfiguration::~TabMaintenanceConfiguration()
{
    delete ui;
}

void TabMaintenanceConfiguration::initializeData()
{
    ui->lineEdit_filePath->clear();
    ui->lineEdit_exportDirectory->clear();
}

void TabMaintenanceConfiguration::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void TabMaintenanceConfiguration::filterMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_UDISK_OFFLINE:
        ON_RESPONSE_FLAG_UDISK_OFFLINE(message);
        break;
    }
}

void TabMaintenanceConfiguration::ON_RESPONSE_FLAG_UDISK_OFFLINE(MessageReceive *message)
{
    int port = *((int *)message->data);
    if (port == m_importPort) {
        ui->lineEdit_filePath->clear();
    }
    if (port == m_exportPort) {
        ui->lineEdit_exportDirectory->clear();
    }
}

void TabMaintenanceConfiguration::onLanguageChanged()
{
    ui->pushButton_restore->setText(GET_TEXT("PROFILE/76000", "Restore"));
    ui->pushButton_backup->setText(GET_TEXT("PROFILE/76001", "Backup"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButton_browse_import->setTranslatableText("CAMERAMAINTENANCE/38004", "Browse");
    ui->pushButton_browse_export->setTranslatableText("CAMERAMAINTENANCE/38004", "Browse");
    ui->label_configurationFile->setText(GET_TEXT("SYSTEMGENERAL/70062", "Configuration File"));
    ui->label_exportDirectory->setText(GET_TEXT("SYSTEMGENERAL/70063", "Export Directory"));
    ui->groupBox_import->setTitle(GET_TEXT("SYSTEMGENERAL/70064", "Import Configuration File"));
    ui->groupBox_export->setTitle(GET_TEXT("SYSTEMGENERAL/70065", "Export Configuration File"));
}

void TabMaintenanceConfiguration::on_pushButton_browse_import_clicked()
{
    const QString &filePath = MyFileSystemDialog::instance()->getOpenFileInfo();
    ui->lineEdit_filePath->setText(filePath);
    if (filePath.isEmpty()) {
        ui->lineEdit_filePath->clear();
        m_importPort = -1;
    } else {
        ui->lineEdit_filePath->setText(filePath);
        m_importPort = MyFileSystemDialog::instance()->currentDevicePort();
    }
}

void TabMaintenanceConfiguration::on_pushButton_restore_clicked()
{
    QString filePath = ui->lineEdit_filePath->text();
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        ui->lineEdit_filePath->setCustomValid(false, GET_TEXT("MYLINETIP/112000", "Cannot be empty."));
        return;
    }
    if (fileInfo.suffix() != QString("cfg")) {
        ui->lineEdit_filePath->setCustomValid(false, GET_TEXT("MYLINETIP/112001", "Invalid."));
        return;
    }

    int result = MessageBox::question(this, GET_TEXT("PROFILE/76005", "If restore successfully, the system will automatically restart, Do you want to continue?"));
    if (result == MessageBox::Cancel) {
        return;
    }

    char cmd[256] = { 0 };
    snprintf(cmd, sizeof(cmd), "msprofile -i \"%s\"", filePath.toStdString().c_str());
    int rv = ms_system(cmd);
    if (rv == 0) {
        struct log_data log_data;
        memset(&log_data, 0, sizeof(struct log_data));
        log_data.log_data_info.subType = SUB_OP_PROFILE_IMPORT_LOCAL;
        log_data.log_data_info.parameter_type = SUB_PARAM_NONE;
        snprintf(log_data.log_data_info.user, sizeof(log_data.log_data_info.user), "%s", gMsUser.userName().toStdString().c_str());
        log_data.log_data_info.chan_no = 0;
        MsWriteLog(log_data);
        //qMsNvr->reboot();
        //MessageBox::message(this, GET_TEXT("PROFILE/76006", "Restore successfully. Restarting..."));
        //qMsApp->setAboutToReboot(true);
    } else {
        ShowMessageBox(GET_TEXT("PROFILE/76007", "Restore Failed."));
    }
}

void TabMaintenanceConfiguration::on_pushButton_browse_export_clicked()
{
    const QString &directory = MyFileSystemDialog::instance()->getOpenDirectory();
    if (directory.isEmpty()) {
        ui->lineEdit_exportDirectory->setText(directory);
        m_exportPort = -1;
    } else {
        ui->lineEdit_exportDirectory->setText(directory);
        m_exportPort = MyFileSystemDialog::instance()->currentDevicePort();
    }
}

void TabMaintenanceConfiguration::on_pushButton_backup_clicked()
{
    QString directory = ui->lineEdit_exportDirectory->text();
    QDir dir(directory);
    if (directory.isEmpty() || !dir.exists()) {
        ui->lineEdit_exportDirectory->setCustomValid(false, GET_TEXT("MYLINETIP/112000", "Cannot be empty."));
        return;
    }

    char cmd[256] = { 0 };
    snprintf(cmd, sizeof(cmd), "msprofile -e \"%s\"", directory.toStdString().c_str());
    int rv = ms_system(cmd);
    if (rv == 0) {
        struct log_data log_data;
        memset(&log_data, 0, sizeof(struct log_data));
        log_data.log_data_info.subType = SUB_OP_PROFILE_EXPORT_LOCAL;
        log_data.log_data_info.parameter_type = SUB_PARAM_NONE;
        snprintf(log_data.log_data_info.user, sizeof(log_data.log_data_info.user), "%s", gMsUser.userName().toStdString().c_str());
        log_data.log_data_info.chan_no = 0;
        MsWriteLog(log_data);
        sync();
        ShowMessageBox(GET_TEXT("PROFILE/76010", "Backup successfully."));
    } else {
        ShowMessageBox(GET_TEXT("PROFILE/76011", "Backup failed."));
    }
}

void TabMaintenanceConfiguration::on_pushButton_back_clicked()
{
    emit sig_back();
}
