#include "TabMaintenanceDiagnosis.h"
#include "ui_TabMaintenanceDiagnosis.h"
#include "MessageFilter.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "MyFileSystemDialog.h"
#include "centralmessage.h"
#include <QDir>

TabMaintenanceDiagnosis::TabMaintenanceDiagnosis(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::TabMaintenanceDiagnosis)
{
    ui->setupUi(this);

    gMessageFilter.installMessageFilter(RESPONSE_FLAG_UDISK_OFFLINE, this);
    onLanguageChanged();
}

TabMaintenanceDiagnosis::~TabMaintenanceDiagnosis()
{
    delete ui;
}

void TabMaintenanceDiagnosis::initializeData()
{
}

void TabMaintenanceDiagnosis::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_EXPORT_DIAGNOSTIC_LOG:
        ON_RESPONSE_FLAG_EXPORT_DIAGNOSTIC_LOG(message);
        break;
    }
}

void TabMaintenanceDiagnosis::filterMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_UDISK_OFFLINE:
        ON_RESPONSE_FLAG_UDISK_OFFLINE(message);
        break;
    }
}

void TabMaintenanceDiagnosis::ON_RESPONSE_FLAG_EXPORT_DIAGNOSTIC_LOG(MessageReceive *message)
{
    //closeWait();

    if (!message->data) {
        qMsWarning() << "data is nullptr";
        ShowMessageBox(GET_TEXT("PROFILE/76011", "Backup failed."));
    } else {
        resp_diagnostic_log_export *log = (resp_diagnostic_log_export *)message->data;
        qMsDebug() << "result:" << log->res;
        if (log->res != -1) {
            ShowMessageBox(GET_TEXT("PROFILE/76010", "Backup successfully."));
        } else {
            ShowMessageBox(GET_TEXT("PROFILE/76011", "Backup failed."));
        }
    }
}

void TabMaintenanceDiagnosis::ON_RESPONSE_FLAG_UDISK_OFFLINE(MessageReceive *message)
{
    int port = *((int *)message->data);
    if (port == m_currentPort) {
        ui->myLineEditPath->clear();
    }
}

void TabMaintenanceDiagnosis::onLanguageChanged()
{
    ui->groupBoxExportDiagnosis->setTitle(GET_TEXT("MAINTENANCE/77006", "Export Diagnosis File"));
    ui->labelExportDirectory->setText(GET_TEXT("MAINTENANCE/77006", "Export Diagnosis File"));
    ui->myPushButtonBrowse->setText(GET_TEXT("CAMERAMAINTENANCE/38004", "Browse"));
    ui->myPushButtonBackup->setText(GET_TEXT("PROFILE/76001", "Backup"));
    ui->myPushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
}

void TabMaintenanceDiagnosis::on_myPushButtonBrowse_clicked()
{
    const QString &directory = MyFileSystemDialog::instance()->getOpenDirectory();
    if (directory.isEmpty()) {
        ui->myLineEditPath->clear();
        m_currentPort = -1;
    } else {
        ui->myLineEditPath->setText(directory);
        m_currentPort = MyFileSystemDialog::instance()->currentDevicePort();
    }
}

void TabMaintenanceDiagnosis::on_myPushButtonBackup_clicked()
{
    ui->myPushButtonBackup->clearUnderMouse();
    QString strPath = ui->myLineEditPath->text();
    if (strPath.isEmpty()) {
        ui->myLineEditPath->setCustomValid(false, GET_TEXT("MYLINETIP/112000", "Cannot be empty."));
        return;
    }

    //导出dump文件
    QDir dir("/mnt/nand");
    QList<QFileInfo> infoList = dir.entryInfoList(QStringList("*.dmp"), QDir::Files, QDir::Time);
    for (int i = 0; i < infoList.size(); ++i) {
        QFileInfo info = infoList.at(i);
        ms_system(QString("cp %1 %2").arg(info.absoluteFilePath()).arg(strPath).toStdString().c_str());
        qMsDebug() << "cp dmp file to usb:" << info.absoluteFilePath();
    }

    //
    char path[256] = { 0 };
    snprintf(path, sizeof(path), "%s", strPath.toStdString().c_str());
    sendMessage(REQUEST_FLAG_EXPORT_DIAGNOSTIC_LOG, path, sizeof(path));

    //showWait();
}

void TabMaintenanceDiagnosis::on_myPushButtonBack_clicked()
{
    emit sig_back();
}
