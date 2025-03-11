#include "TabMaintenanceOnlineUpgrade.h"
#include "ui_TabMaintenanceOnlineUpgrade.h"
#include "MyDebug.h"
#include "UpgradeThread.h"
#include "centralmessage.h"
#include "MessageBox.h"
#include "MsApplication.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MsWaitting.h"

TabMaintenanceOnlineUpgrade::TabMaintenanceOnlineUpgrade(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::TabMaintenanceOnlineUpgrade)
{
    ui->setupUi(this);

    m_upgradeThread = new UpgradeThread();
    connect(m_upgradeThread, SIGNAL(downloadFinished(int)), this, SLOT(onDownloadFinished(int)));
    connect(m_upgradeThread, SIGNAL(upgradeFinished(int)), this, SLOT(onUpgradeFinished(int)));
}

TabMaintenanceOnlineUpgrade::~TabMaintenanceOnlineUpgrade()
{
    m_upgradeThread->stopThread();
    m_upgradeThread->deleteLater();
    delete ui;
}

bool TabMaintenanceOnlineUpgrade::canAutoLogout()
{
    if (MsWaitting::instance()->isVisible()) {
        return false;
    } else if (m_isAboutToReboot) {
        return false;
    } else {
        return true;
    }
}

void TabMaintenanceOnlineUpgrade::onLanguageChanged()
{
    ui->label_note->setText(GET_TEXT("CAMERAMAINTENANCE/38011", "Note: The upgrading process will take 5-10 minutes, please don't disconnect power of the device during the process. The device will reboot automatically after upgrading."));

    ui->pushButton_check->setText(GET_TEXT("UPGRADE/75013", "Check"));
    ui->pushButton_upgrade->setText(GET_TEXT("SYSTEMGENERAL/70006", "Upgrade"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->label_model->setText(GET_TEXT("CHANNELMANAGE/30029", "Model"));
    ui->label_current_ver->setText(GET_TEXT("UPGRADE/75017", "Software Version"));
    ui->label_latest_ver->setText(GET_TEXT("UPGRADE/75018", "Latest Version"));
}

void TabMaintenanceOnlineUpgrade::onUpgradeFinished(int result)
{
    qMsDebug() << "nvr online upgrade result:" << result;
    if (result < 0) {
        //MsWaitting::closeGlobalWait();
        ShowMessageBox(GET_TEXT("UPGRADE/75010", "Upgrade Failed."));
    } else {
        qMsDebug() << "onUpgradeFinished success";
        m_isAboutToReboot = true;
        qMsNvr->reboot();
        qMsApp->setAboutToReboot(m_isAboutToReboot);
    }
}

void TabMaintenanceOnlineUpgrade::onDownloadFinished(int result)
{
    if (result != 0) {
        //MsWaitting::closeGlobalWait();
        ShowMessageBox(GET_TEXT("UPGRADE/75028", "Download Failed."));
    } else {
        //调用升级镜像前，先REQUEST_FLAG_UPGRADE_SYSTEM_INIT这个请求
        sendMessageOnly(REQUEST_FLAG_UPGRADE_SYSTEM_INIT, nullptr, 0);
        m_upgradeThread->startOnlineUpgrade();
    }
}

void TabMaintenanceOnlineUpgrade::initializeData()
{
    curren_ver = qMsNvr->softwareVersion();
    ui->lineEdit_model->setText(qMsNvr->model());
    ui->lineEdit_software->setText(curren_ver);
    ui->pushButton_upgrade->setEnabled(false);
    ui->lineEdit_latest_version->setText("");
    onLanguageChanged();
    upgrade_state = 0;
}
void TabMaintenanceOnlineUpgrade::ON_RESPONSE_FLAG_CHECK_ONLINE_NVR(MessageReceive *message)
{
    //MsWaitting::closeGlobalWait();
    if (!message->data) {
        qMsWarning() << "check nvr data is null.";
        return;
    }
    struct resp_check_online_upgrade *online_upgrade = (struct resp_check_online_upgrade *)message->data;
    QString Description = online_upgrade->pDescription;
    upgrade_state = online_upgrade->state;
    url = online_upgrade->pUrl;
    QString version = online_upgrade->pSoftversion;
    m_fileSize = online_upgrade->fileSize;
    qMsDebug() << QString("222 check nvr:latest_ver:[%1], state:[%2], url:[%3], pDescription:[%4], size:[%5]")
                      .arg(version)
                      .arg(upgrade_state)
                      .arg(url)
                      .arg(Description)
                      .arg(online_upgrade->fileSize);
    ui->lineEdit_latest_version->setText(version);
    if (upgrade_state && !version.isEmpty() && !url.isEmpty() && version != curren_ver) {
        ui->pushButton_upgrade->setEnabled(true);
    } else if (version == curren_ver) {
        ShowMessageBox(GET_TEXT("UPGRADE/75016", "Already Latest."));
    } else if (QString(Description).isEmpty()) {
        ShowMessageBox(GET_TEXT("UPGRADE/75019", "Network error."));
    } else {
        ShowMessageBox(Description);
    }
    //    MsDebug()<<QString("Description isEmpty() :%1;  Description:%2").arg(QString(Description).isEmpty()).arg(Description);
}

quint64 TabMaintenanceOnlineUpgrade::maxFirmwareSize() const
{
    quint64 size = (84 << 20);
    return size;
}

void TabMaintenanceOnlineUpgrade::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_CHECK_ONLINE_NVR:
        ON_RESPONSE_FLAG_CHECK_ONLINE_NVR(message);
        break;
    default:
        break;
    }
}

void TabMaintenanceOnlineUpgrade::on_pushButton_check_clicked()
{
    sendMessage(REQUEST_FLAG_CHECK_ONLINE_NVR, nullptr, 0);
    //MsDebug() <<"111 on_pushButton_check_clicked";
    ui->pushButton_upgrade->setEnabled(false);
    ui->lineEdit_latest_version->setText("");
    //MsWaitting::showGlobalWait(this);
}

void TabMaintenanceOnlineUpgrade::on_pushButton_upgrade_clicked()
{
    if (m_fileSize > maxFirmwareSize()) {
        ShowMessageBox(GET_TEXT("CAMERAMAINTENANCE/38014", "Firmware file is too large."));
        return;
    }
    int result = MessageBox::question(this, GET_TEXT("UPGRADE/75015", "The device will reboot automatically after upgrading, continue?"));
    if (result == MessageBox::Yes) {
        m_upgradeThread->getOnlineUpgradeImage(url);
        //MsWaitting::showGlobalWait(this);
    }
}

void TabMaintenanceOnlineUpgrade::on_pushButton_back_clicked()
{
    emit sig_back();
}
