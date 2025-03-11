#include "TabMaintenanceLocalUpgrade.h"
#include "ui_TabMaintenanceLocalUpgrade.h"
#include "LogWrite.h"
#include "MessageBox.h"
#include "MessageFilter.h"
#include "MsApplication.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyFileSystemDialog.h"
#include "UpgradeThread.h"
#include "centralmessage.h"
#include <QtDebug>

TabMaintenanceLocalUpgrade::TabMaintenanceLocalUpgrade(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::TabMaintenanceLocalUpgrade)
{
    ui->setupUi(this);

    m_upgradeThread = new UpgradeThread();
    connect(m_upgradeThread, SIGNAL(upgradeFinished(int)), this, SLOT(onUpgradeFinished(int)));

    gMessageFilter.installMessageFilter(RESPONSE_FLAG_UDISK_OFFLINE, this);
    onLanguageChanged();
}

TabMaintenanceLocalUpgrade::~TabMaintenanceLocalUpgrade()
{
    m_upgradeThread->stopThread();
    m_upgradeThread->deleteLater();
    delete ui;
}

void TabMaintenanceLocalUpgrade::initializeData()
{
    ui->lineEdit_filePath->clear();
    ui->checkBox_reset->setChecked(false);
}

bool TabMaintenanceLocalUpgrade::canAutoLogout()
{
    if (MsWaitting::instance()->isVisible()) {
        return false;
    } else if (m_isAboutToReboot) {
        return false;
    } else {
        return true;
    }
}

void TabMaintenanceLocalUpgrade::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void TabMaintenanceLocalUpgrade::filterMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_UDISK_OFFLINE:
        ON_RESPONSE_FLAG_UDISK_OFFLINE(message);
        break;
    }
}

void TabMaintenanceLocalUpgrade::ON_RESPONSE_FLAG_UDISK_OFFLINE(MessageReceive *message)
{
    int port = *((int *)message->data);
    if (port == m_currentPort) {
        ui->lineEdit_filePath->clear();
    }
}

void TabMaintenanceLocalUpgrade::onLanguageChanged()
{
    ui->label_note->setText(GET_TEXT("CAMERAMAINTENANCE/38011", "Note: The upgrading process will take 5-10 minutes, please don't disconnect power of the device during the process. The device will reboot automatically after upgrading."));

    ui->checkBox_reset->setText(GET_TEXT("UPGRADE/75002", "Reset settings to factory default (except IP Address and User Information)"));

    ui->pushButton_upgrade->setText(GET_TEXT("SYSTEMGENERAL/70006", "Upgrade"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->label_reset->setText(GET_TEXT("COMMON/1057", "Reset"));
    ui->label_firmware->setText(GET_TEXT("CAMERAMAINTENANCE/38001", "Firmware"));
    ui->pushButton_browse->setTranslatableText("CAMERAMAINTENANCE/38004", "Browse");
}

void TabMaintenanceLocalUpgrade::onUpgradeFinished(int result)
{
    LogWrite::instance()->writeLog(SUB_OP_UPGRADE_LOCAL);

    if (result < 0) {
        //MsWaitting::closeGlobalWait();
        ShowMessageBox(GET_TEXT("UPGRADE/75010", "Upgrade Failed."));
    } else {
        m_isAboutToReboot = true;
        qMsNvr->reboot();
        qMsApp->setAboutToReboot(m_isAboutToReboot);
        //m_waitting->//showWait();
    }
}

void TabMaintenanceLocalUpgrade::on_pushButton_browse_clicked()
{
    const QString &filePath = MyFileSystemDialog::instance()->getOpenFileInfo();
    if (filePath.isEmpty()) {
        ui->lineEdit_filePath->clear();
        m_currentPort = -1;
    } else {
        ui->lineEdit_filePath->setText(filePath);
        m_currentPort = MyFileSystemDialog::instance()->currentDevicePort();
    }
}

void TabMaintenanceLocalUpgrade::on_pushButton_upgrade_clicked()
{
    QString filePath = ui->lineEdit_filePath->text();
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || fileInfo.size() < 150000) {
        ui->lineEdit_filePath->setCustomValid(false, GET_TEXT("MYLINETIP/112001", "Invalid."));
        return;
    }
    if (fileInfo.size() > qint64(120 << 20)) {
        ShowMessageBox(GET_TEXT("CAMERAMAINTENANCE/38014", "Firmware file is too large."));
        return;
    }

    //
    int newPrefix = filePath.mid(filePath.lastIndexOf("/") + 1).split(".").at(1).toInt();
    QString strSoftVersion = qMsNvr->deviceInfo().softver;
    int oldPrefix = strSoftVersion.split(".").at(1).toInt();
    int oldPrefixEnd = strSoftVersion.split(".").at(3).toInt();
    if (newPrefix >= 9 && oldPrefix < 9) {
        if (oldPrefix != 8 || oldPrefixEnd != 7) {
            int result = MessageBox::question(this, GET_TEXT("UPGRADE/75016", "Warning: All hard disks need to be formatted after upgrading to this new firmware. Still upgrade?"));
            if (result == MessageBox::Cancel) {
                return;
            }
        }
    }

    //
    int result = MessageBox::question(this, GET_TEXT("UPGRADE/75009", "Note: Please do not shut down the system during the upgrade. System will reboot, continue?"));
    if (result == MessageBox::Cancel) {
        return;
    }

    //调用升级镜像前，先REQUEST_FLAG_UPGRADE_SYSTEM_INIT这个请求
    sendMessage(REQUEST_FLAG_UPGRADE_SYSTEM_INIT, nullptr, 0);

    m_upgradeThread->startLocalUpgrade(filePath, ui->checkBox_reset->isChecked());

    //MsWaitting::showGlobalWait(this);
}

void TabMaintenanceLocalUpgrade::on_pushButton_back_clicked()
{
    emit sig_back();
}
