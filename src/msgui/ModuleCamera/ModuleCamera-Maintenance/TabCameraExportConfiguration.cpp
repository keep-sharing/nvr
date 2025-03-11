#include "TabCameraExportConfiguration.h"
#include "ui_TabCameraExportConfiguration.h"
#include "FilePassword.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "MyFileSystemDialog.h"
#include "centralmessage.h"

extern "C" {
#include "msg.h"
#include "recortsp.h"
}

const int CameraInfoRole = Qt::UserRole + 100;
TabCameraExportConfiguration::TabCameraExportConfiguration(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::TabCameraExportConfiguration)
{
    ui->setupUi(this);
    ui->tableView->setHorizontalHeaderItem(MaintenanceTableView::ColumnProgress, QString(GET_TEXT("SYSTEMGENERAL/70071", "Progress")));

    ui->lineEditExport->setCheckMode(MyLineEdit::EmptyCheck);
    ui->lineEditImport->setCheckMode(MyLineEdit::EmptyCheck);
    onLanguageChanged();
}

TabCameraExportConfiguration::~TabCameraExportConfiguration()
{
    delete ui;
}

void TabCameraExportConfiguration::initializeData()
{
    ui->lineEditExport->clear();
    ui->lineEditImport->clear();
    m_stateMap.clear();
    ui->tableView->scrollToTop();
    ui->tableView->clearSort();
    //showWait();
    sendMessage(REQUEST_FLAG_GET_IPCLIST, nullptr, 0);
}

void TabCameraExportConfiguration::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPCLIST:
        ON_RESPONSE_FLAG_GET_IPCLIST(message);
        break;
    case RESPONSE_FLAG_SET_IPC_CFG:
        ON_RESPONSE_FLAG_SET_IPC_CFG(message);
        break;
    case RESPONSE_FLAG_GET_IPC_CFG:
        ON_RESPONSE_FLAG_GET_IPC_CFG(message);
        break;
    }
}

bool TabCameraExportConfiguration::isCloseable()
{
    if (ui->pushButtonExport->isEnabled()) {
        return true;
    }
    int result = MessageBox::question(this, GET_TEXT("CAMERAMAINTENANCE/38010", "Exit will stop all unfinished progress, continue?"));
    if (result == MessageBox::Cancel) {
        return false;
    }

    if (ui->pushButtonExport->isEnabled()) {
        return true;
    }
    m_isCancel = true;
    m_isAboutToClose = true;
    //showWait();
    return false;
}

bool TabCameraExportConfiguration::isChangeable()
{
    if (ui->pushButtonExport->isEnabled()) {
        return true;
    }
    int result = MessageBox::question(this, GET_TEXT("CAMERAMAINTENANCE/38010", "Exit will stop all unfinished progress, continue?"));
    if (result == MessageBox::Cancel) {
        return false;
    } else {
        m_isCancel = true;
        m_isAboutToClose = false;
        //showWait();
        return true;
    }
}

bool TabCameraExportConfiguration::canAutoLogout()
{
    if (ui->pushButtonExport->isEnabled()) {
        return true;
    } else {
        return false;
    }
}

void TabCameraExportConfiguration::onLanguageChanged()
{
    ui->groupBoxImport->setTitle(GET_TEXT("SYSTEMGENERAL/70064", "Import Configuration File"));
    ui->groupBoxExport->setTitle(GET_TEXT("SYSTEMGENERAL/70065", "Export Configuration File"));
    ui->groupBoxTable->setTitle(GET_TEXT("CHANNELMANAGE/30008", "Channel"));

    ui->labelImport->setText(GET_TEXT("SYSTEMGENERAL/70062", "Configuration File"));
    ui->labelExport->setText(GET_TEXT("SYSTEMGENERAL/70063", "Export Directory"));

    ui->pushButtonImport->setText(GET_TEXT("ANPR/103049", "Import"));
    ui->pushButtonExport->setText(GET_TEXT("PROFILE/76001", "Backup"));
    ui->pushButtonBrowseImport->setText(GET_TEXT("CAMERAMAINTENANCE/38004", "Browse"));
    ui->pushButtonBrowseExport->setText(GET_TEXT("CAMERAMAINTENANCE/38004", "Browse"));

    ui->pushButtonRefresh->setText(GET_TEXT("COMMON/1035", "Refresh"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
}

void TabCameraExportConfiguration::ON_RESPONSE_FLAG_SET_IPC_CFG(MessageReceive *message)
{
    if (!message->data) {
        m_eventLoop.exit(-1);
        return;
    }
    int result = *(static_cast<int *>(message->data));
    return m_eventLoop.exit(result);
}

void TabCameraExportConfiguration::ON_RESPONSE_FLAG_GET_IPC_CFG(MessageReceive *message)
{
    if (!message->data) {
        m_eventLoop.exit(-1);
        return;
    }
    int result = *(static_cast<int *>(message->data));
    return m_eventLoop.exit(result);
}

void TabCameraExportConfiguration::setUpgradeEnabled(bool enable)
{
    ui->tableView->setCheckListEnable(enable);
    ui->pushButtonExport->setEnabled(enable);
    ui->pushButtonImport->setEnabled(enable);
    ui->pushButtonBrowseExport->setEnabled(enable);
    ui->pushButtonBrowseImport->setEnabled(enable);
}

void TabCameraExportConfiguration::importOrExportConfiguration(int mode, QString pwd)
{
    setUpgradeEnabled(false);
    for (int i = 0; i < ui->tableView->rowCount(); ++i) {
        const resq_get_ipcdev &ipcdev = ui->tableView->itemData(i, MaintenanceTableView::ColumnCheck, CameraInfoRole).value<resq_get_ipcdev>();
        MaintenanceTablelItemInfo &maintenanceTablelItemInfo = m_stateMap[ipcdev.chanid];
        if (ui->tableView->isItemChecked(i)) {
            maintenanceTablelItemInfo.isChecked = true;
            MsCameraVersion cameraVersion = qMsNvr->cameraVersion(ipcdev.chanid);
            if (ipcdev.state == RTSP_CLIENT_CONNECT && cameraVersion >= MsCameraVersion(7, 76)) {
                maintenanceTablelItemInfo.state = GET_TEXT("UPGRADE/75027", "Waiting");
            } else {
                maintenanceTablelItemInfo.state = GET_TEXT("UPGRADE/75024", "Failure");
            }
            ui->tableView->setItemText(i, MaintenanceTableView::ColumnProgress, GET_TEXT("UPGRADE/75027", "Waiting"));
        } else {
            maintenanceTablelItemInfo.isChecked = false;
        }
    }

    ReqIpcCfg reqIpcCfg;
    memset(&reqIpcCfg, 0, sizeof(ReqIpcCfg));
    if (mode == EXPORT_CONFIGURATION) {
        snprintf(reqIpcCfg.exportPath, sizeof(reqIpcCfg.exportPath), "%s", m_filePath.toStdString().c_str());
    } else {
        snprintf(reqIpcCfg.filename, sizeof(reqIpcCfg.filename), "%s", m_filePath.toStdString().c_str());
    }
    snprintf(reqIpcCfg.pwd, sizeof(reqIpcCfg.pwd), "%s", pwd.toStdString().c_str());
    for (auto iter = m_channelMap.constBegin(); iter != m_channelMap.constEnd(); ++iter) {
        if (m_isCancel) {
            break;
        }
        int channel = iter.key();

        if (m_stateMap.value(channel).state == GET_TEXT("UPGRADE/75024", "Failure")) {
            ui->tableView->setUpgradeProgressText(m_stateMap, channel, GET_TEXT("UPGRADE/75024", "Failure"));
            continue;
        }

        reqIpcCfg.chnId = channel;
        STRUCT(ReqIpcCfg, &reqIpcCfg,
               FIELD(int, chnId);
               FIELD(char *, pwd);
               FIELD(char *, exportPath);
               FIELD(char *, filename));
        if (mode == EXPORT_CONFIGURATION) {
            sendMessage(REQUEST_FLAG_GET_IPC_CFG, &reqIpcCfg, sizeof(ReqIpcCfg));
            ui->tableView->setUpgradeProgressText(m_stateMap, channel, GET_TEXT("CAMERAMAINTENANCE/152081", "Exporting"));
        } else {
            sendMessage(REQUEST_FLAG_SET_IPC_CFG, &reqIpcCfg, sizeof(ReqIpcCfg));
            ui->tableView->setUpgradeProgressText(m_stateMap, channel, GET_TEXT("CAMERAMAINTENANCE/152082", "Importing"));
        }
        // int result = m_eventLoop.exec();
        // if (result == 0) {
        //     ui->tableView->setUpgradeProgressText(m_stateMap, channel, GET_TEXT("UPGRADE/75023", "Success"));
        // } else {
        //     ui->tableView->setUpgradeProgressText(m_stateMap, channel, GET_TEXT("UPGRADE/75024", "Failure"));
        // }
    }
    setUpgradeEnabled(true);

    if (m_isCancel) {
        //closeWait();
        m_isCancel = false;
        if (m_isAboutToClose) {
            back();
        } else {
            initializeData();
        }
    }
}

void TabCameraExportConfiguration::ON_RESPONSE_FLAG_GET_IPCLIST(MessageReceive *message)
{
    //closeWait();

    m_allCameraList.clear();
    resq_get_ipcdev *ipcdev_array = (resq_get_ipcdev *)message->data;
    int count = message->header.size / sizeof(resq_get_ipcdev);
    for (int i = 0; i < count; ++i) {
        const resq_get_ipcdev &ipcdev = ipcdev_array[i];
        //不显示第三方
        if (!QString(ipcdev.sn).isEmpty() && QString(ipcdev.sn) != QString("TX_MS20119_X01201112250001000100010")) {
            continue;
        }

        //RTSP forbidden
        if (ipcdev.protocol == IPC_PROTOCOL_RTSP) {
            continue;
        }

        m_allCameraList.append(ipcdev);
    }
    ui->tableView->refreshTable(m_allCameraList, m_stateMap);
    if (!ui->pushButtonExport->isEnabled()) {
        ui->tableView->setCheckListEnable(false);
    }
}

void TabCameraExportConfiguration::on_pushButtonBrowseImport_clicked()
{
    ui->pushButtonBrowseImport->clearHover();

    QString filePath = MyFileSystemDialog::instance()->getOpenFileInfo();
    if (filePath.isEmpty()) {
        ui->lineEditImport->clear();
        return;
    }
    ui->lineEditImport->setText(filePath);
}

void TabCameraExportConfiguration::on_pushButtonImport_clicked()
{
    if (!ui->lineEditImport->checkValid()) {
        return;
    }
    m_filePath = ui->lineEditImport->text();
    QFileInfo fileInfo(m_filePath);
    if (!fileInfo.exists()) {
        ui->lineEditImport->setCustomValid(false, GET_TEXT("MYLINETIP/112001", "Invalid."));
        return;
    }

    m_channelMap.clear();
    for (int i = 0; i < ui->tableView->rowCount(); ++i) {
        if (ui->tableView->isItemChecked(i)) {
            const resq_get_ipcdev &ipcdev = ui->tableView->itemData(i, MaintenanceTableView::ColumnCheck, CameraInfoRole).value<resq_get_ipcdev>();
            m_channelMap.insert(ipcdev.chanid, 0);
        }
    }
    if (m_channelMap.isEmpty()) {
        ShowMessageBox(GET_TEXT("CAMERAMAINTENANCE/152003", "Please select at least one device."));
        return;
    }
    FilePassword filePassword(this);
    filePassword.setMode(FilePassword::ModeDecryption);
    int result = filePassword.exec();
    if (result == QDialog::Rejected) {
        return;
    }
    importOrExportConfiguration(IMPORT_CONFIGURATION, filePassword.getPassword());
}

void TabCameraExportConfiguration::on_pushButtonBrowseExport_clicked()
{
    ui->pushButtonBrowseExport->clearHover();
    QString filePath = MyFileSystemDialog::instance()->getOpenDirectory();
    if (filePath.isEmpty()) {
        ui->lineEditExport->clear();
        return;
    }
    ui->lineEditExport->setText(filePath);
}

void TabCameraExportConfiguration::on_pushButtonExport_clicked()
{
    if (!ui->lineEditExport->checkValid()) {
        ui->lineEditExport->setCustomValid(false, GET_TEXT("MYLINETIP/112001", "Invalid."));
        return;
    }
    m_filePath = ui->lineEditExport->text();

    m_channelMap.clear();
    for (int i = 0; i < ui->tableView->rowCount(); ++i) {
        if (ui->tableView->isItemChecked(i)) {
            const resq_get_ipcdev &ipcdev = ui->tableView->itemData(i, MaintenanceTableView::ColumnCheck, CameraInfoRole).value<resq_get_ipcdev>();
            m_channelMap.insert(ipcdev.chanid, 0);
        }
    }
    if (m_channelMap.isEmpty()) {
        ShowMessageBox(GET_TEXT("CAMERAMAINTENANCE/152003", "Please select at least one device."));
        return;
    }
    FilePassword filePassword(this);
    filePassword.setMode(FilePassword::ModeEncrption);
    int result = filePassword.exec();
    if (result == QDialog::Rejected) {
        return;
    }
    importOrExportConfiguration(EXPORT_CONFIGURATION, filePassword.getPassword());
}

void TabCameraExportConfiguration::on_pushButtonRefresh_clicked()
{
    //showWait();
    ui->tableView->updateChecked(m_stateMap);
    sendMessage(REQUEST_FLAG_GET_IPCLIST, nullptr, 0);
}

void TabCameraExportConfiguration::on_pushButtonBack_clicked()
{
    if (isCloseable()) {
        back();
    }
}
