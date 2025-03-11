#include "TabCameraOnlineUpgrade.h"
#include "ui_TabCameraOnlineUpgrade.h"
#include "CameraStatusWidget.h"
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

TabCameraOnlineUpgrade::TabCameraOnlineUpgrade(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::CameraOnlineUpgrade)
{
    ui->setupUi(this);

    //initialize table
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("CHANNELMANAGE/30008", "Channel");
    headerList << GET_TEXT("CHANNELMANAGE/30009", "Channel Name");
    headerList << GET_TEXT("MENU/10006", "Status");
    headerList << GET_TEXT("COMMON/1033", "IP Address");
    headerList << GET_TEXT("FISHEYE/12016", "Channel ID");
    headerList << GET_TEXT("CHANNELMANAGE/30014", "Protocol");
    headerList << GET_TEXT("CHANNELMANAGE/30027", "MAC");
    headerList << GET_TEXT("CHANNELMANAGE/30054", "Firmware Version");
    headerList << GET_TEXT("CHANNELMANAGE/30029", "Model");
    headerList << GET_TEXT("UPGRADE/75018", "Latest Version");
    headerList << GET_TEXT("SYSTEMGENERAL/70006", "Upgrade");
    headerList << GET_TEXT("CAMERAMAINTENANCE/38005", "Upgrade Progress");
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setColumnCount(headerList.size());
    ui->tableView->setItemDelegateForColumn(ColumnUpgrade, new ItemIconDelegate(this));
    connect(ui->tableView, SIGNAL(itemClicked(int, int)), this, SLOT(onTableItemClicked(int, int)));
    connect(ui->tableView, SIGNAL(headerChecked(bool)), this, SLOT(onTableHeaderChecked(bool)));
    //sort
    ui->tableView->setSortableForColumn(ColumnCheck, false);
    ui->tableView->setSortType(ColumnChannel, SortFilterProxyModel::SortInt);
    ui->tableView->setSortType(ColumnIP, SortFilterProxyModel::SortIP);
    ui->tableView->setSortType(ColumnStatus, SortFilterProxyModel::SortInt);
    //column width
    ui->tableView->setColumnWidth(ColumnCheck, 50);
    ui->tableView->setColumnWidth(ColumnChannel, 70);
    ui->tableView->setColumnWidth(ColumnName, 120);
    ui->tableView->setColumnWidth(ColumnStatus, 120);
    ui->tableView->setColumnWidth(ColumnIP, 120);
    ui->tableView->setColumnWidth(ColumnChannelId, 100);
    ui->tableView->setColumnWidth(ColumnProtocol, 90);
    ui->tableView->setColumnWidth(ColumnMAC, 120);
    ui->tableView->setColumnWidth(ColumnFirmware, 160);
    ui->tableView->setColumnWidth(ColumnModel, 150);
    ui->tableView->setColumnWidth(ColumnLatest, 160);
    ui->tableView->setColumnWidth(ColumnUpgrade, 70);

    onLanguageChanged();
}

TabCameraOnlineUpgrade::~TabCameraOnlineUpgrade()
{
    delete ui;
}

void TabCameraOnlineUpgrade::initializeData()
{
    m_onlineMap.clear();
    ui->tableView->scrollToTop();
    ui->tableView->clearSort();
    //showWait();
    sendMessage(REQUEST_FLAG_GET_IPCLIST, nullptr, 0);

    ui->pushButton_upgrade->setEnabled(!m_onlineMap.isEmpty());
}

void TabCameraOnlineUpgrade::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPCLIST:
        ON_RESPONSE_FLAG_GET_IPCLIST(message);
        break;
    case RESPONSE_FLAG_CHECK_ONLINE_IPC:
        ON_RESPONSE_FLAG_CHECK_ONLINE_IPC(message);
        break;
    case RESPONSE_FLAG_ONLINE_UPGRADE_CAMERA:
        ON_RESPONSE_FLAG_ONLINE_UPGRADE_CAMERA(message);
        break;
    }
}

bool TabCameraOnlineUpgrade::isCloseable()
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
    int stop = 1;
    m_isCancel = true;
    m_isAboutToClose = true;
    m_upgradeListMap.clear();
    m_upgradeIndex.clear();
    qMsDebug() << "About to back.";
    Q_UNUSED(stop)
    return false;
}

bool TabCameraOnlineUpgrade::isChangeable()
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
    int stop = 1;
    Q_UNUSED(stop)
    m_isCancel = true;
    m_isAboutToClose = false;
    m_upgradeListMap.clear();
    m_upgradeIndex.clear();
    qMsDebug() << "isChangeable About to change.111";
    //showWait();
    m_upgradeLoop.exec();
    //closeWait();
    qMsDebug() << "isChangeable About to change.222";
    return true;
}

bool TabCameraOnlineUpgrade::canAutoLogout()
{
    if (!m_isUpgrading) {
        return true;
    } else {
        return false;
    }
}

void TabCameraOnlineUpgrade::ON_RESPONSE_FLAG_GET_IPCLIST(MessageReceive *message)
{
    //closeWait();

    QString str;
    QStringList list;

    m_allCameraList.clear();
    resq_get_ipcdev *ipcdev_array = (resq_get_ipcdev *)message->data;
    int count = message->header.size / sizeof(resq_get_ipcdev);
    for (int i = 0; i < count; ++i) {
        const resq_get_ipcdev &ipcdev = ipcdev_array[i];
        //不显示第三方
        //MsDebug()<<QString("empty:[%1]SN:[%2]").arg(QString(ipcdev.sn).isEmpty()).arg(ipcdev.sn);
        if (!QString(ipcdev.sn).isEmpty() && QString(ipcdev.sn) != QString("TX_MS20119_X01201112250001000100010")) {
            continue;
        }

        //RTSP forbidden
        if (ipcdev.protocol == IPC_PROTOCOL_RTSP) {
            continue;
        }

        str = QString(ipcdev.fwversion);
        if (ipcdev.state == RTSP_CLIENT_CONNECT && !str.isEmpty()) {
            list = str.split(".");
            str = list[2];
            //只显示标准版本
            if (str.toInt() == 0) {
                m_allCameraList.append(ipcdev);
            }
            //MSHN-9761 QT-camera Maintenance：在线升级界面，标准版NVR需可搜索出OEM且带Milesight logo的IPC，并进行升级
            else if (QString(ipcdev.manufacturer) == QString("Milesight Technology Co.,Ltd.")) {
                m_allCameraList.append(ipcdev);
            }
        } else {
            m_allCameraList.append(ipcdev);
        }
    }

    showCameraTable();
}

void TabCameraOnlineUpgrade::ON_RESPONSE_FLAG_CHECK_ONLINE_IPC(MessageReceive *message)
{
    int result = -1;

    if (!message->data) {
        qWarning() << "CameraOnlineUpgrade::ON_RESPONSE_FLAG_CHECK_ONLINE_IPC, ### 222 data is null.";
        m_upgradeLoop.exit(result);
        return;
    }

    resp_check_online_upgrade_ipc *ipc_check_info = (resp_check_online_upgrade_ipc *)message->data;

    UpgradeInfo &upgradeInfo = m_onlineMap[ipc_check_info->chnid];
    upgradeInfo.info = *ipc_check_info;
    upgradeInfo.checkState = StateChecked;

    m_upgradeLoop.exit(result);
}

void TabCameraOnlineUpgrade::ON_RESPONSE_FLAG_ONLINE_UPGRADE_CAMERA(MessageReceive *message)
{
    RespUpgradeCamera *result = static_cast<RespUpgradeCamera *>(message->data);
    if (result) {
        qMsDebug() << QString("chnIdMask:%1").arg(result->chnIdMask, 64, 2, QLatin1Char('0'));
        for (int i = 0; i < 64; ++i) {
            if (result->chnIdMask & ((quint64)1 << i)) {
                int state = result->statusArr[i];
                qMsDebug() << QString("channel:%1, state:%2").arg(i).arg(state);
                m_onlineMap[i].upgradeState = state;
                setTextByChannel(i, ColumnProgress, progressString(state));
            }
        }
    }
    startUpgrede();
}

void TabCameraOnlineUpgrade::showCameraTable()
{
    ui->tableView->clearContent();
    ui->tableView->setRowCount(m_allCameraList.size());
    int row = 0;
    for (int i = 0; i < m_allCameraList.size(); ++i) {
        const resq_get_ipcdev &ipcdev = m_allCameraList.at(i);
        const UpgradeInfo &value = m_onlineMap.value(ipcdev.chanid);
        ui->tableView->setItemData(row, ColumnCheck, QVariant::fromValue(ipcdev), CameraInfoRole);
        ui->tableView->setItemChecked(row, value.isChecked);
        //channel
        ui->tableView->setItemIntValue(row, ColumnChannel, ipcdev.chanid + 1);
        //channel name
        ui->tableView->setItemText(row, ColumnName, qMsNvr->channelName(ipcdev.chanid));
        ui->tableView->setItemToolTip(row, ColumnName, ui->tableView->itemText(row, ColumnName));
        //status
        CameraStatusWidget *statusWidget = new CameraStatusWidget();
        statusWidget->setState(ipcdev.state);
        statusWidget->setStateString(ipcdev.connect_state);
        ui->tableView->setItemWidget(i, ColumnStatus, statusWidget);
        ui->tableView->setItemData(i, ColumnStatus, statusWidget->stateValue(), SortIntRole);
        //ip
        ui->tableView->setItemText(row, ColumnIP, ipcdev.ipaddr);
        //channel id
        ui->tableView->setItemText(i, ColumnChannelId, "-");
        //protocol
        ui->tableView->setItemText(row, ColumnProtocol, ipcdev.protocol == IPC_PROTOCOL_RTSP ? "RTSP" : qMsNvr->protocolName(ipcdev.protocol));
        //mac
        ui->tableView->setItemText(row, ColumnMAC, ipcdev.mac);
        //firmware
        ui->tableView->setItemText(row, ColumnFirmware, ipcdev.fwversion);
        //model
        ui->tableView->setItemText(row, ColumnModel, ipcdev.model);
        //latest version
        switch (value.checkState) {
        case StateNone:
            ui->tableView->setItemText(row, ColumnLatest, "-");
            ui->tableView->setItemPixmap(row, ColumnUpgrade, QPixmap(":/common/common/camer_upgrade_disable.png"));
            break;
        case StateDisconnect:
            ui->tableView->setItemText(row, ColumnLatest, "N/A");
            ui->tableView->setItemPixmap(row, ColumnUpgrade, QPixmap(":/common/common/camer_upgrade_disable.png"));
            break;
        case StateChecked:
            if (QString(value.info.upInfo.pDescription) == QString("The current version is the latest version.")) {
                ui->tableView->setItemText(row, ColumnLatest, GET_TEXT("UPGRADE/75029", "Already Latest"));
                ui->tableView->setItemPixmap(row, ColumnUpgrade, QPixmap(":/common/common/camer_upgrade_disable.png"));
            } else if (QString(value.info.upInfo.pSoftversion).isEmpty()) {
                ui->tableView->setItemText(row, ColumnLatest, GET_TEXT("CHANNELMANAGE/30081", "Network Error"));
                ui->tableView->setItemPixmap(row, ColumnUpgrade, QPixmap(":/common/common/camer_upgrade_disable.png"));
            } else {
                ui->tableView->setItemText(row, ColumnLatest, value.info.upInfo.pSoftversion);
                ui->tableView->setItemPixmap(row, ColumnUpgrade, QPixmap(":/common/common/camer_upgrade.png"));
            }
            break;
        }
        //upgrade progress
        ui->tableView->setItemText(row, ColumnProgress, progressString(value.upgradeState));

        row++;
    }
    ui->tableView->reorder();
}

void TabCameraOnlineUpgrade::setTextByChannel(int channel, int Column, const QString &text)
{
    for (int i = 0; i < ui->tableView->rowCount(); ++i) {
        const resq_get_ipcdev &ipcdev = ui->tableView->itemData(i, ColumnCheck, CameraInfoRole).value<resq_get_ipcdev>();
        if (ipcdev.chanid == channel) {
            ui->tableView->setItemText(i, Column, text);
            break;
        }
    }
}

void TabCameraOnlineUpgrade::onTableItemClicked(int row, int column)
{
    if (column == ColumnUpgrade) {
        const resq_get_ipcdev &ipcdev = ui->tableView->itemData(row, ColumnCheck, CameraInfoRole).value<resq_get_ipcdev>();
        if (m_onlineMap.contains(ipcdev.chanid)) {
            const UpgradeInfo &value = m_onlineMap.value(ipcdev.chanid);
            if (value.canUpgrade()) {
                int result = MessageBox::question(this, GET_TEXT("CAMERAMAINTENANCE/38006", "The device will reboot automatically after upgrade, continue?"));
                if (result == MessageBox::Yes) {
                    m_upgradeListMap[value.info.upInfo.pUrl].append(value);
                    if (!m_upgradeIndex.contains(value.info.upInfo.pUrl)) {
                        m_upgradeIndex.append(value.info.upInfo.pUrl);
                    }
                    ui->tableView->setItemText(row, ColumnProgress, GET_TEXT("UPGRADE/75027", "Waiting"));
                    m_onlineMap[ipcdev.chanid].upgradeState = UPGRADE_WAITING;
                    if (!m_isUpgrading) {
                        m_isUpgrading = 1;
                        startUpgrede();
                    }
                }
            }
        }
    } else if (column == ColumnCheck) {
        const resq_get_ipcdev &ipcdev = ui->tableView->itemData(row, ColumnCheck, CameraInfoRole).value<resq_get_ipcdev>();
        UpgradeInfo &value = m_onlineMap[ipcdev.chanid];
        value.isChecked = ui->tableView->isItemChecked(row);
    }
}

void TabCameraOnlineUpgrade::onTableHeaderChecked(bool checked)
{
    Q_UNUSED(checked)
}

void TabCameraOnlineUpgrade::on_pushButton_check_clicked()
{
    //showWait();
    for (int i = 0; i < m_allCameraList.size(); ++i) {
        const resq_get_ipcdev &ipcdev = m_allCameraList.at(i);
        QString model = ipcdev.model;
        int chanid = ipcdev.chanid;
        if (ipcdev.state != RTSP_CLIENT_CONNECT) {
            UpgradeInfo &upgradeInfo = m_onlineMap[chanid];
            upgradeInfo.info.chnid = chanid;
            upgradeInfo.checkState = StateDisconnect;
        } else if (ipcdev.state == RTSP_CLIENT_CONNECT) {
            sendMessage(REQUEST_FLAG_CHECK_ONLINE_IPC, (void *)&chanid, sizeof(int));
            m_upgradeLoop.exec();
        }
    }

    QString text("\n");
    for (auto iter = m_onlineMap.constBegin(); iter != m_onlineMap.constEnd(); ++iter) {
        const UpgradeInfo &value = iter.value();
        const resp_check_online_upgrade_ipc &online = value.info;
        text += QString("chnid:%1, state:%2, fileSize:%3, pSoftversion:%4, pUrl:%5, pDescription:%6\n")
                    .arg(online.chnid)
                    .arg(online.upInfo.state)
                    .arg(online.upInfo.fileSize)
                    .arg(online.upInfo.pSoftversion)
                    .arg(online.upInfo.pUrl)
                    .arg(online.upInfo.pDescription);
    }
    qMsDebug() << qPrintable(text);

    bool allEmpty = true;
    for (auto iter = m_onlineMap.constBegin(); iter != m_onlineMap.constEnd(); ++iter) {
        const UpgradeInfo &value = iter.value();
        if (!QString(value.info.upInfo.pSoftversion).isEmpty()) {
            allEmpty = false;
            break;
        }
    }
    if (allEmpty) {
        MessageBox::information(this, GET_TEXT("UPGRADE/75019", "Network error."));
        //closeWait();
        return;
    }

    for (int row = 0; row < ui->tableView->rowCount(); ++row) {
        const resq_get_ipcdev &ipcdev = ui->tableView->itemData(row, ColumnCheck, CameraInfoRole).value<resq_get_ipcdev>();

        const UpgradeInfo &value = m_onlineMap.value(ipcdev.chanid);
        switch (value.checkState) {
        case StateDisconnect:
            ui->tableView->setItemText(row, ColumnLatest, "N/A");
            ui->tableView->setItemPixmap(row, ColumnUpgrade, QPixmap(":/common/common/camer_upgrade_disable.png"));
            break;
        case StateChecked:
            if (QString(value.info.upInfo.pDescription) == QString("The current version is the latest version.")) {
                ui->tableView->setItemText(row, ColumnLatest, GET_TEXT("UPGRADE/75029", "Already Latest"));
                ui->tableView->setItemPixmap(row, ColumnUpgrade, QPixmap(":/common/common/camer_upgrade_disable.png"));
            } else if (QString(value.info.upInfo.pSoftversion).isEmpty()) {
                ui->tableView->setItemText(row, ColumnLatest, GET_TEXT("CHANNELMANAGE/30081", "Network Error"));
                ui->tableView->setItemPixmap(row, ColumnUpgrade, QPixmap(":/common/common/camer_upgrade_disable.png"));
            } else {
                ui->tableView->setItemText(row, ColumnLatest, value.info.upInfo.pSoftversion);
                ui->tableView->setItemPixmap(row, ColumnUpgrade, QPixmap(":/common/common/camer_upgrade.png"));
            }
            break;
        }
    }
    //closeWait();

    ui->pushButton_upgrade->setEnabled(!m_onlineMap.isEmpty());
}

void TabCameraOnlineUpgrade::on_pushButton_upgrade_clicked()
{
    //    m_isUpgrading = 0;
    //    m_upgradeListMap.clear();
    QMap<QString, QList<UpgradeInfo>> tempUpgradeListMap;

    for (int row = 0; row < ui->tableView->rowCount(); ++row) {
        if (ui->tableView->isItemChecked(row)) {
            const resq_get_ipcdev &ipcdev = ui->tableView->itemData(row, ColumnCheck, CameraInfoRole).value<resq_get_ipcdev>();
            if (m_onlineMap.contains(ipcdev.chanid)) {
                const UpgradeInfo &value = m_onlineMap.value(ipcdev.chanid);
                if (value.canUpgrade()) {
                    tempUpgradeListMap[value.info.upInfo.pUrl].append(value);
                    if (!m_upgradeIndex.contains(value.info.upInfo.pUrl)) {
                        m_upgradeIndex.append(value.info.upInfo.pUrl);
                    }
                }
            } else {
            }
        }
    }
    if (tempUpgradeListMap.isEmpty()) {
        ShowMessageBox(GET_TEXT("CAMERAMAINTENANCE/152003", "Please select at least one device."));
        return;
    }

    int result = MessageBox::question(this, GET_TEXT("CAMERAMAINTENANCE/38006", "The device will reboot automatically after upgrade, continue?"));
    if (result == MessageBox::Cancel) {
        return;
    }

    for(auto iter = tempUpgradeListMap.begin(); iter != tempUpgradeListMap.end(); iter++) {
        m_upgradeListMap[iter.key()].append(iter.value());
    }

    //显示Waiting
    for (int row = 0; row < ui->tableView->rowCount(); ++row) {
        if (ui->tableView->isItemChecked(row)) {
            const resq_get_ipcdev &ipcdev = ui->tableView->itemData(row, ColumnCheck, CameraInfoRole).value<resq_get_ipcdev>();
            if (m_onlineMap.contains(ipcdev.chanid)) {
                const UpgradeInfo &value = m_onlineMap.value(ipcdev.chanid);
                if (value.canUpgrade()) {
                    ui->tableView->setItemText(row, ColumnProgress, GET_TEXT("UPGRADE/75027", "Waiting"));
                    m_onlineMap[ipcdev.chanid].upgradeState = UPGRADE_WAITING;
                }
            }
        }
    }

    if (!m_isUpgrading) {
        m_isUpgrading = 1;
        startUpgrede();
    }
    ui->pushButton_refresh->clearFocus();
}

void TabCameraOnlineUpgrade::on_pushButton_refresh_clicked()
{
    ui->pushButton_refresh->clearUnderMouse();
    //showWait();
    for (int i = 0; i < ui->tableView->rowCount(); ++i) {
        const resq_get_ipcdev &ipcdev = ui->tableView->itemData(i, ColumnCheck, CameraInfoRole).value<resq_get_ipcdev>();
        UpgradeInfo &value = m_onlineMap[ipcdev.chanid];
        if (ui->tableView->isItemChecked(i)) {
            value.isChecked = true;
        } else {
            value.isChecked = false;
        }
    }
    sendMessage(REQUEST_FLAG_GET_IPCLIST, nullptr, 0);
}

void TabCameraOnlineUpgrade::on_pushButton_back_clicked()
{
    if (isCloseable()) {
        back();
    }
}

void TabCameraOnlineUpgrade::onLanguageChanged()
{
    ui->pushButton_check->setText(GET_TEXT("UPGRADE/75013", "Check"));
    ui->pushButton_upgrade->setText(GET_TEXT("SYSTEMGENERAL/70006", "Upgrade"));
    ui->pushButton_refresh->setText(GET_TEXT("COMMON/1035", "Refresh"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
}

void TabCameraOnlineUpgrade::startUpgrede()
{
    if (m_upgradeListMap.isEmpty()) {
        stopUpgrede();
    } else {
        ReqUpgradeCamera req;
        memset(&req, 0, sizeof(req));
        auto iter = m_upgradeIndex.begin();
        const QList<UpgradeInfo> &list = m_upgradeListMap.value(*iter);
        for (int i = 0; i < list.size(); ++i) {
            const resp_check_online_upgrade_ipc &online = list.at(i).info;
            req.chnIdMask |= ((quint64)1 << online.chnid);
            req.keepConfMask |= ((quint64)1 << online.chnid);
            snprintf(req.url, sizeof(req.url), "%s", online.upInfo.pUrl);
            setTextByChannel(online.chnid, ColumnProgress, GET_TEXT("UPGRADE/75022", "Upgrading"));
            m_onlineMap[online.chnid].upgradeState = UPGRADE_ING;
        }
        sendMessage(REQUEST_FLAG_ONLINE_UPGRADE_CAMERA, &req, sizeof(req));
        m_upgradeListMap.remove(*iter);
        m_upgradeIndex.erase(iter);
    }
}

void TabCameraOnlineUpgrade::stopUpgrede()
{
    qMsDebug() << "stop upgrade! m_isupgrading:" << m_isUpgrading
               << " m_isCancel:" << m_isCancel
               << " m_isAboutToClose:" << m_isAboutToClose;
    m_isUpgrading = 0;

    if (m_isCancel) {
        //cancel
        if (m_isAboutToClose) {
            //closeWait();
            back();
        } else {
            m_upgradeLoop.exit();
            //event loop
        }
    }

    m_isCancel = false;
    m_isAboutToClose = false;
    return;
}

QString TabCameraOnlineUpgrade::progressString(int state) const
{
    switch (state) {
    case -1:
        return "-";
    case UPGRADE_ING:
        return GET_TEXT("UPGRADE/75022", "Upgrading");
    case UPGRADE_SUCCESS:
        return GET_TEXT("UPGRADE/75023", "Success");
    case UPGRADE_FAILED:
        return GET_TEXT("UPGRADE/75024", "Failure");
    case UPGRADE_MISMATCH:
        return GET_TEXT("UPGRADE/75025", "Firmware Mismatch");
    case UPGRADE_WAITING:
        return GET_TEXT("UPGRADE/75027", "Waiting");
    default:
        return GET_TEXT("UPGRADE/75026", "Unknow");
    }
}
