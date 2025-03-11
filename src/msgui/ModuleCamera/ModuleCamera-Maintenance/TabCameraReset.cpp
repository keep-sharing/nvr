#include "TabCameraReset.h"
#include "ui_TabCameraReset.h"
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

TabCameraReset::TabCameraReset(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::TabCameraReset)
{
    ui->setupUi(this);
    ui->tableView->setHorizontalHeaderItem(MaintenanceTableView::ColumnProgress, QString(GET_TEXT("CAMERAMAINTENANCE/152008", "Reset Progress")));
    onLanguageChanged();
}

TabCameraReset::~TabCameraReset()
{
    delete ui;
}

void TabCameraReset::initializeData()
{
    ui->checkBoxIP->setChecked(true);
    ui->checkBoxUser->setChecked(true);
    m_stateMap.clear();
    ui->tableView->scrollToTop();
    ui->tableView->clearSort();
    //showWait();
    sendMessage(REQUEST_FLAG_GET_IPCLIST, nullptr, 0);
}

void TabCameraReset::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPCLIST:
        ON_RESPONSE_FLAG_GET_IPCLIST(message);
        break;
    case RESPONSE_FLAG_SET_IPC_RESET:
        ON_RESPONSE_FLAG_SET_IPC_RESET(message);
        break;
    }
}

bool TabCameraReset::isCloseable()
{
    if (ui->pushButtonReset->isEnabled()) {
        return true;
    }
    int result = MessageBox::question(this, GET_TEXT("CAMERAMAINTENANCE/38010", "Exit will stop all unfinished progress, continue?"));
    if (result == MessageBox::Cancel) {
        return false;
    }

    if (ui->pushButtonReset->isEnabled()) {
        return true;
    }
    m_isCancel = true;
    m_isAboutToClose = true;
    //showWait();
    return false;
}

bool TabCameraReset::isChangeable()
{
    if (ui->pushButtonReset->isEnabled()) {
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

bool TabCameraReset::canAutoLogout()
{
    if (ui->pushButtonReset->isEnabled()) {
        return true;
    } else {
        return false;
    }
}

void TabCameraReset::onLanguageChanged()
{
    ui->checkBoxIP->setText(GET_TEXT("PROFILE/76017", "Keep the IP Configuration"));
    ui->checkBoxUser->setText(GET_TEXT("PROFILE/76018", "Keep the User Information"));

    ui->pushButtonReset->setText(GET_TEXT("OCCUPANCY/74221", "Reset"));
    ui->pushButtonRefresh->setText(GET_TEXT("COMMON/1035", "Refresh"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
}

void TabCameraReset::ON_RESPONSE_FLAG_GET_IPCLIST(MessageReceive *message)
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
    if (!ui->pushButtonReset->isEnabled()) {
        ui->tableView->setCheckListEnable(false);
    }
}

void TabCameraReset::ON_RESPONSE_FLAG_SET_IPC_RESET(MessageReceive *message)
{
    if (!message->data) {
        m_eventLoop.exit(-1);
        return;
    }
    int result = *(static_cast<int *>(message->data));
    return m_eventLoop.exit(result);
}

void TabCameraReset::setUpgradeEnabled(bool enable)
{
    ui->checkBoxIP->setEnabled(enable);
    ui->checkBoxUser->setEnabled(enable);
    ui->pushButtonReset->setEnabled(enable);
    ui->tableView->setCheckListEnable(enable);
    ui->pushButtonRefresh->clearHover();
}

void TabCameraReset::on_pushButtonReset_clicked()
{
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

    int result = MessageBox::question(this, GET_TEXT("CAMERAMAINTENANCE/152002", "Devices will reboot automatically after resetting，continue?"));
    if (result == MessageBox::Cancel) {
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

    ReqIpcReset reqIpcReset;
    memset(&reqIpcReset, 0, sizeof(ReqIpcReset));
    reqIpcReset.options |= (ui->checkBoxIP->isChecked() << 0);
    reqIpcReset.options |= (ui->checkBoxUser->isChecked() << 1);
    for (auto iter = m_channelMap.constBegin(); iter != m_channelMap.constEnd(); ++iter) {
        if (m_isCancel) {
            break;
        }
        int channel = iter.key();

        if (m_stateMap.value(channel).state == GET_TEXT("UPGRADE/75024", "Failure")) {
            ui->tableView->setUpgradeProgressText(m_stateMap, channel, GET_TEXT("UPGRADE/75024", "Failure"));
            continue;
        }

        reqIpcReset.chnId = channel;
        STRUCT(ReqIpcReset, &reqIpcReset,
               FIELD(int, chnId);
               FIELD(int, options));
        sendMessage(REQUEST_FLAG_SET_IPC_RESET, &reqIpcReset, sizeof(ReqIpcReset));
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

void TabCameraReset::on_pushButtonRefresh_clicked()
{
    //showWait();
    ui->tableView->updateChecked(m_stateMap);
    sendMessage(REQUEST_FLAG_GET_IPCLIST, nullptr, 0);
}

void TabCameraReset::on_pushButtonBack_clicked()
{
    if (isCloseable()) {
        back();
    }
}
