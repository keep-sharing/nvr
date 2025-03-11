#include "TabCameraLocalUpgrade.h"
#include "ui_TabCameraLocalUpgrade.h"
#include "CameraStatusWidget.h"
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

TabCameraLocalUpgrade::TabCameraLocalUpgrade(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::CameraLocalUpgrade)
{
    ui->setupUi(this);

    connect(ui->tableView, SIGNAL(itemClicked(int, int)), this, SLOT(onTableItemClicked(int, int)));
    connect(ui->tableView, SIGNAL(headerChecked(bool)), this, SLOT(onTableHeaderChecked(bool)));
    ui->lineEdit_firmware->setCheckMode(MyLineEdit::EmptyCheck);
    onLanguageChanged();
}

TabCameraLocalUpgrade::~TabCameraLocalUpgrade()
{
    delete ui;
}

void TabCameraLocalUpgrade::initializeData()
{
    ui->lineEdit_firmware->clear();
    ui->checkBox_reset->setChecked(false);
    m_stateMap.clear();
    ui->tableView->scrollToTop();
    ui->tableView->clearSort();
    //showWait();
    sendMessage(REQUEST_FLAG_GET_IPCLIST, nullptr, 0);
}

void TabCameraLocalUpgrade::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPCLIST:
        ON_RESPONSE_FLAG_GET_IPCLIST(message);
        break;
    case RESPONSE_FLAG_UPGRADE_IPC_IMAGE:
        ON_RESPONSE_FLAG_UPGRADE_IPC_IMAGE(message);
        break;
    }
}

bool TabCameraLocalUpgrade::isCloseable()
{
    if (ui->pushButton_upgrade->isEnabled()) {
        return true;
    }
    int result = MessageBox::question(this, GET_TEXT("CAMERAMAINTENANCE/38010", "Exit will stop all unfinished progress, continue?"));
    if (result == MessageBox::Cancel) {
        return false;
    }

    if (ui->pushButton_upgrade->isEnabled()) {
        return true;
    }
    m_isCancel = true;
    m_isAboutToClose = true;
    //showWait();
    return false;
}

bool TabCameraLocalUpgrade::isChangeable()
{
    if (ui->pushButton_upgrade->isEnabled()) {
        return true;
    }
    int result = MessageBox::question(this, GET_TEXT("CAMERAMAINTENANCE/38010", "Exit will stop all unfinished progress, continue?"));
    if (result == MessageBox::Cancel) {
        return false;
    } else {
        if (ui->pushButton_upgrade->isEnabled()) {
            return true;
        }
        m_isCancel = true;
        m_isAboutToClose = false;
        m_upgradeMap.clear();
        //showWait();
        m_quitLoop.exec();
        setUpgradeEnabled(true);
        return true;
    }
}

bool TabCameraLocalUpgrade::canAutoLogout()
{
    if (ui->pushButton_upgrade->isEnabled()) {
        return true;
    } else {
        return false;
    }
}

void TabCameraLocalUpgrade::onLanguageChanged()
{
    ui->label_firmware->setText(GET_TEXT("CAMERAMAINTENANCE/38001", "Firmware"));
    ui->label_reset->setText(GET_TEXT("COMMON/1057", "Reset"));
    ui->checkBox_reset->setText(GET_TEXT("CAMERAMAINTENANCE/38003", "Reset settings to factory default (except IP Address and User information)"));
    ui->pushButton_browse->setText(GET_TEXT("CAMERAMAINTENANCE/38004", "Browse"));

    ui->pushButton_upgrade->setText(GET_TEXT("SYSTEMGENERAL/70006", "Upgrade"));
    ui->pushButton_refresh->setText(GET_TEXT("COMMON/1035", "Refresh"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
}

void TabCameraLocalUpgrade::ON_RESPONSE_FLAG_GET_IPCLIST(MessageReceive *message)
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
    if (!ui->pushButton_upgrade->isEnabled()) {
        ui->tableView->setCheckListEnable(false);
    }
}

void TabCameraLocalUpgrade::ON_RESPONSE_FLAG_UPGRADE_IPC_IMAGE(MessageReceive *message)
{
    resp_upgrade_ipc_image *resp_upgrade = (resp_upgrade_ipc_image *)message->data;
    if (!resp_upgrade) {
        qWarning() << QString("CameraMaintenance::ON_RESPONSE_FLAG_UPGRADE_IPC_IMAGE, data is null.");
        return;
    }
    m_quitLoop.exit();
    m_eventLoop.exit(resp_upgrade->status);

    qMsDebug() << QString("RESPONSE_FLAG_UPGRADE_IPC_IMAGE, channel: %1, status: %2").arg(resp_upgrade->chnid).arg(resp_upgrade->status);
}

void TabCameraLocalUpgrade::setUpgradeEnabled(bool enable)
{
    ui->tableView->setCheckListEnable(enable);
    ui->pushButton_browse->setEnabled(enable);
    ui->checkBox_reset->setEnabled(enable);
    ui->pushButton_upgrade->setEnabled(enable);
    ui->pushButton_refresh->clearHover();
}

void TabCameraLocalUpgrade::onTableItemClicked(int row, int column)
{
    Q_UNUSED(row)
    Q_UNUSED(column)
}

void TabCameraLocalUpgrade::onTableHeaderChecked(bool checked)
{
    Q_UNUSED(checked)
}

void TabCameraLocalUpgrade::on_pushButton_browse_clicked()
{
    ui->pushButton_browse->clearHover();
    QString filePath = MyFileSystemDialog::instance()->getOpenFileInfo();
    if (filePath.isEmpty()) {
        ui->lineEdit_firmware->clear();
        return;
    }
    ui->lineEdit_firmware->setText(filePath);
}

void TabCameraLocalUpgrade::on_pushButton_upgrade_clicked()
{
    if (!ui->lineEdit_firmware->checkValid()) {
        return;
    }
    m_filePath = ui->lineEdit_firmware->text();
    QFileInfo fileInfo(m_filePath);
    if (!fileInfo.exists()) {
        ui->lineEdit_firmware->setCustomValid(false, GET_TEXT("MYLINETIP/112001", "Invalid."));
        return;
    }
    //镜像不能大于120M
    if ((fileInfo.size() >> 20) > 120) {
        ShowMessageBox(GET_TEXT("CAMERAMAINTENANCE/152080", "Firmware file size cannot exceed 120Mb"));
        return;
    }

    m_upgradeMap.clear();
    for (int i = 0; i < ui->tableView->rowCount(); ++i) {
        if (ui->tableView->isItemChecked(i)) {
            const resq_get_ipcdev &ipcdev = ui->tableView->itemData(i, MaintenanceTableView::ColumnCheck, CameraInfoRole).value<resq_get_ipcdev>();
            m_upgradeMap.insert(ipcdev.chanid, 0);
        }
    }
    if (m_upgradeMap.isEmpty()) {
        ShowMessageBox(GET_TEXT("CAMERAMAINTENANCE/152003", "Please select at least one device."));
        return;
    }

    int result = MessageBox::question(this, GET_TEXT("CAMERAMAINTENANCE/38006", "The device will reboot automatically after upgrade, continue?"));
    if (result == MessageBox::Cancel) {
        return;
    }
    setUpgradeEnabled(false);

    for (int i = 0; i < ui->tableView->rowCount(); ++i) {
        const resq_get_ipcdev &ipcdev = ui->tableView->itemData(i, MaintenanceTableView::ColumnCheck, CameraInfoRole).value<resq_get_ipcdev>();
        MaintenanceTablelItemInfo &maintenanceTablelItemInfo = m_stateMap[ipcdev.chanid];
        if (ui->tableView->isItemChecked(i)) {
            maintenanceTablelItemInfo.isChecked = true;
            if (ipcdev.state == RTSP_CLIENT_CONNECT) {
                maintenanceTablelItemInfo.state = GET_TEXT("UPGRADE/75027", "Waiting");
            } else {
                maintenanceTablelItemInfo.state = GET_TEXT("UPGRADE/75024", "Failure");
            }
            ui->tableView->setItemText(i, MaintenanceTableView::ColumnProgress, GET_TEXT("UPGRADE/75027", "Waiting"));
        } else {
            maintenanceTablelItemInfo.isChecked = false;
        }
    }

    req_upgrade_ipc_image upgrade_ipc;
    upgrade_ipc.keepconfig = !ui->checkBox_reset->isChecked();
    snprintf(upgrade_ipc.filepath, sizeof(upgrade_ipc.filepath), "%s", m_filePath.toStdString().c_str());
    for (auto iter = m_upgradeMap.constBegin(); iter != m_upgradeMap.constEnd(); ++iter) {
        if (m_isCancel) {
            break;
        }
        int channel = iter.key();

        if (m_stateMap.value(channel).state == GET_TEXT("UPGRADE/75024", "Failure")) {
            ui->tableView->setUpgradeProgressText(m_stateMap, channel, GET_TEXT("UPGRADE/75024", "Failure"));
            continue;
        }
        upgrade_ipc.chnid = channel;
        sendMessage(REQUEST_FLAG_UPGRADE_IPC_IMAGE, &upgrade_ipc, sizeof(req_upgrade_ipc_image));
        ui->tableView->setUpgradeProgressText(m_stateMap, channel, GET_TEXT("UPGRADE/75022", "Upgrading"));
        // int result = m_eventLoop.exec();
        // switch (result) {
        // case UPGRADE_ING:
        //     ui->tableView->setUpgradeProgressText(m_stateMap, channel, GET_TEXT("UPGRADE/75022", "Upgrading"));
        //     break;
        // case UPGRADE_SUCCESS:
        //     ui->tableView->setUpgradeProgressText(m_stateMap, channel, GET_TEXT("UPGRADE/75023", "Success"));
        //     break;
        // case UPGRADE_FAILED:
        //     ui->tableView->setUpgradeProgressText(m_stateMap, channel, GET_TEXT("UPGRADE/75024", "Failure"));
        //     break;
        // case UPGRADE_MISMATCH:
        //     ui->tableView->setUpgradeProgressText(m_stateMap, channel, GET_TEXT("UPGRADE/75025", "Firmware Mismatch"));
        //     break;
        // default:
        //     ui->tableView->setUpgradeProgressText(m_stateMap, channel, GET_TEXT("UPGRADE/75026", "Unknow"));
        //     break;
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

void TabCameraLocalUpgrade::on_pushButton_refresh_clicked()
{
    //showWait();
    ui->tableView->updateChecked(m_stateMap);
    sendMessage(REQUEST_FLAG_GET_IPCLIST, nullptr, 0);
}

void TabCameraLocalUpgrade::on_pushButton_back_clicked()
{
    if (isCloseable()) {
        back();
    }
}
