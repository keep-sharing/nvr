#include "TabCameraReboot.h"
#include "ui_TabCameraReboot.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"

extern "C" {
#include "msg.h"
#include "recortsp.h"
}

const int CameraInfoRole = Qt::UserRole + 100;

TabCameraReboot::TabCameraReboot(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::TabCameraReboot)
{
    ui->setupUi(this);
    ui->tableView->setHorizontalHeaderItem(MaintenanceTableView::ColumnProgress, QString(GET_TEXT("CAMERAMAINTENANCE/152007", "Reboot Progress")));
    onLanguageChanged();
}

TabCameraReboot::~TabCameraReboot()
{
    delete ui;
}

void TabCameraReboot::initializeData()
{
    m_stateMap.clear();
    m_channelMap.clear();
    ui->tableView->scrollToTop();
    ui->tableView->clearSort();
    //showWait();
    sendMessage(REQUEST_FLAG_GET_IPCLIST, nullptr, 0);
}

void TabCameraReboot::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPCLIST:
        ON_RESPONSE_FLAG_GET_IPCLIST(message);
        break;
    case RESPONSE_FLAG_SET_IPC_REBOOT:
        ON_RESPONSE_FLAG_SET_IPC_REBOOT(message);
        break;
    }
}

bool TabCameraReboot::isCloseable()
{
    if (!m_isUpgrading) {
        return true;
    }
    int result = MessageBox::question(this, GET_TEXT("CAMERAMAINTENANCE/38010", "Exit will stop all unfinished progress, continue?"));
    if (result == MessageBox::Cancel) {
        return false;
    }

    if (!m_isUpgrading) {
        return true;
    }
    m_isCancel = true;
    m_isAboutToClose = true;
    //showWait();
    return false;
}

bool TabCameraReboot::isChangeable()
{
    if (!m_isUpgrading) {
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

bool TabCameraReboot::canAutoLogout()
{
    if (!m_isUpgrading) {
        return true;
    } else {
        return false;
    }
}

void TabCameraReboot::onLanguageChanged()
{
    ui->pushButtonReboot->setText(GET_TEXT("USER/74045", "Reboot"));
    ui->pushButtonRefresh->setText(GET_TEXT("COMMON/1035", "Refresh"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
}

void TabCameraReboot::ON_RESPONSE_FLAG_GET_IPCLIST(MessageReceive *message)
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
        if (ipcdev.protocol == IPC_PROTOCOL_RTSP)
            continue;

        m_allCameraList.append(ipcdev);
    }
    ui->tableView->refreshTable(m_allCameraList, m_stateMap);
}

void TabCameraReboot::ON_RESPONSE_FLAG_SET_IPC_REBOOT(MessageReceive *message)
{
    if (!message->data) {
        ui->tableView->setUpgradeProgressText(m_stateMap, m_currentChannel, GET_TEXT("UPGRADE/75024", "Failure"));
        startUpgrede();
        return;
    }
    int result = *(static_cast<int *>(message->data));
    if (result == 0) {
        ui->tableView->setUpgradeProgressText(m_stateMap, m_currentChannel, GET_TEXT("UPGRADE/75023", "Success"));
    } else {
        ui->tableView->setUpgradeProgressText(m_stateMap, m_currentChannel, GET_TEXT("UPGRADE/75024", "Failure"));
    }
    startUpgrede();
}

void TabCameraReboot::startUpgrede()
{
    if (m_channelMap.isEmpty() || m_isCancel) {
        stopUpgrede();
    } else {
        auto iter = m_channelMap.begin();
        m_currentChannel = iter.key();

        if (m_stateMap.value(m_currentChannel).state == GET_TEXT("UPGRADE/75024", "Failure")) {
            ui->tableView->setUpgradeProgressText(m_stateMap, m_currentChannel, GET_TEXT("UPGRADE/75024", "Failure"));
            m_channelMap.erase(iter);
            startUpgrede();
        } else {
            sendMessage(REQUEST_FLAG_SET_IPC_REBOOT, &m_currentChannel, sizeof(int));
            m_channelMap.erase(iter);
        }
    }
}

void TabCameraReboot::stopUpgrede()
{
    m_isUpgrading = 0;

    if (m_isCancel) {
        if (m_isAboutToClose) {
            //closeWait();
            back();
        } else {
            //m_eventLoop.exit();
            initializeData();
        }
    }

    m_isCancel = false;
    m_isAboutToClose = false;
    return;
}

void TabCameraReboot::on_pushButtonReboot_clicked()
{
    QMap<int, int> tempMap;
    for (int i = 0; i < ui->tableView->rowCount(); ++i) {
        if (ui->tableView->isItemChecked(i)) {
            const resq_get_ipcdev &ipcdev = ui->tableView->itemData(i, MaintenanceTableView::ColumnCheck, CameraInfoRole).value<resq_get_ipcdev>();
            tempMap.insert(ipcdev.chanid, 0);
        }
    }
    if (tempMap.isEmpty()) {
        ShowMessageBox(GET_TEXT("CAMERAMAINTENANCE/152003", "Please select at least one device."));
        return;
    }

    int result = MessageBox::question(this, GET_TEXT("CAMERAMAINTENANCE/152001", "Do you want to reboot the selected devices?"));
    if (result == MessageBox::Cancel) {
        return;
    }

    for (auto iter = tempMap.begin(); iter != tempMap.end(); ++iter) {
        m_channelMap.insert(iter.key(), iter.value());
    }

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
    if (!m_isUpgrading) {
        m_isUpgrading = 1;
        startUpgrede();
    }
}

void TabCameraReboot::on_pushButtonRefresh_clicked()
{
    //showWait();
    ui->tableView->updateChecked(m_stateMap);
    sendMessage(REQUEST_FLAG_GET_IPCLIST, nullptr, 0);
}

void TabCameraReboot::on_pushButtonBack_clicked()
{
    if (isCloseable()) {
        back();
    }
}
