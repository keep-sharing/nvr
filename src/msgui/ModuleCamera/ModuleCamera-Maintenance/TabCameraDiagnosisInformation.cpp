#include "TabCameraDiagnosisInformation.h"
#include "ui_TabCameraDiagnosisInformation.h"
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
TabCameraDiagnosisInformation::TabCameraDiagnosisInformation(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::TabCameraDiagnosisInformation)
{
    ui->setupUi(this);
    ui->tableView->setHorizontalHeaderItem(MaintenanceTableView::ColumnProgress, QString(GET_TEXT("CAMERAMAINTENANCE/152000", "Backup Progress")));
    ui->lineEditExport->setCheckMode(MyLineEdit::EmptyCheck);
    onLanguageChanged();
}

TabCameraDiagnosisInformation::~TabCameraDiagnosisInformation()
{
    delete ui;
}

void TabCameraDiagnosisInformation::initializeData()
{
    ui->lineEditExport->clear();
    m_stateMap.clear();
    ui->tableView->scrollToTop();
    ui->tableView->clearSort();
    //showWait();
    sendMessage(REQUEST_FLAG_GET_IPCLIST, nullptr, 0);
}

void TabCameraDiagnosisInformation::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPCLIST:
        ON_RESPONSE_FLAG_GET_IPCLIST(message);
        break;
    case RESPONSE_FLAG_GET_IPC_DIAGNOSE:
        ON_RESPONSE_FLAG_GET_IPC_DIAGNOSE(message);
        break;
    }
}

bool TabCameraDiagnosisInformation::isCloseable()
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

bool TabCameraDiagnosisInformation::isChangeable()
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

bool TabCameraDiagnosisInformation::canAutoLogout()
{
    if (ui->pushButtonExport->isEnabled()) {
        return true;
    } else {
        return false;
    }
}

void TabCameraDiagnosisInformation::onLanguageChanged()
{
    ui->labelExport->setText(GET_TEXT("SYSTEMGENERAL/70063", "Export Directory"));
    ui->pushButtonExport->setText(GET_TEXT("PROFILE/76001", "Backup"));
    ui->pushButtonBrowseExport->setText(GET_TEXT("CAMERAMAINTENANCE/38004", "Browse"));

    ui->pushButtonRefresh->setText(GET_TEXT("COMMON/1035", "Refresh"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
}

void TabCameraDiagnosisInformation::ON_RESPONSE_FLAG_GET_IPCLIST(MessageReceive *message)
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
        MsCameraVersion version = MsCameraVersion(ipcdev.fwversion);
        if (version.oemType() != 0) {
            if ((version >= MsCameraVersion(8, 3, 3) && ipcdev.chipinfo[19] == '4') ||
                (version >= MsCameraVersion(8, 3, 1) && ipcdev.chipinfo[19] != '4')) {
                continue;
            }
        }

        m_allCameraList.append(ipcdev);
    }
    ui->tableView->refreshTable(m_allCameraList, m_stateMap);
    if (!ui->pushButtonExport->isEnabled()) {
        ui->tableView->setCheckListEnable(false);
    }
}

void TabCameraDiagnosisInformation::ON_RESPONSE_FLAG_GET_IPC_DIAGNOSE(MessageReceive *message)
{
    if (!message->data) {
        m_eventLoop.exit(-1);
        return;
    }
    int result = *(static_cast<int *>(message->data));
    return m_eventLoop.exit(result);
}

void TabCameraDiagnosisInformation::setUpgradeEnabled(bool enable)
{
    ui->tableView->setCheckListEnable(enable);
    ui->pushButtonExport->setEnabled(enable);
    ui->pushButtonBrowseExport->setEnabled(enable);
}

void TabCameraDiagnosisInformation::on_pushButtonBrowseExport_clicked()
{
    ui->pushButtonBrowseExport->clearHover();

    QString filePath = MyFileSystemDialog::instance()->getOpenDirectory();
    if (filePath.isEmpty()) {
        ui->lineEditExport->clear();
        return;
    }
    ui->lineEditExport->setText(filePath);
}

void TabCameraDiagnosisInformation::on_pushButtonExport_clicked()
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

    ReqIpcDiagnose reqIpcDiagnose;
    memset(&reqIpcDiagnose, 0, sizeof(ReqIpcDiagnose));
    snprintf(reqIpcDiagnose.exportPath, sizeof(reqIpcDiagnose.exportPath), "%s", m_filePath.toStdString().c_str());
    for (auto iter = m_channelMap.constBegin(); iter != m_channelMap.constEnd(); ++iter) {
        if (m_isCancel) {
            break;
        }
        int channel = iter.key();

        if (m_stateMap.value(channel).state == GET_TEXT("UPGRADE/75024", "Failure")) {
            ui->tableView->setUpgradeProgressText(m_stateMap, channel, GET_TEXT("UPGRADE/75024", "Failure"));
            continue;
        }

        reqIpcDiagnose.chnId = channel;
        STRUCT(ReqIpcDiagnose, &reqIpcDiagnose,
               FIELD(int, chnId);
               FIELD(char *, exportPath));
        sendMessage(REQUEST_FLAG_GET_IPC_DIAGNOSE, &reqIpcDiagnose, sizeof(ReqIpcCfg));
        ui->tableView->setUpgradeProgressText(m_stateMap, channel, GET_TEXT("CAMERAMAINTENANCE/152081", "Exporting"));
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

void TabCameraDiagnosisInformation::on_pushButtonRefresh_clicked()
{
    //showWait();
    ui->tableView->updateChecked(m_stateMap);
    sendMessage(REQUEST_FLAG_GET_IPCLIST, nullptr, 0);
}

void TabCameraDiagnosisInformation::on_pushButtonBack_clicked()
{
    if (isCloseable()) {
        back();
    }
}
