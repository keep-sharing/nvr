#include "HotSpare.h"
#include "ui_HotSpare.h"
#include "MessageBox.h"
#include "MsApplication.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "centralmessage.h"
#include "myqt.h"
#include <QtDebug>

extern "C" {
#include "log.h"
}

enum SLAVE_STATUS {
    LINK_UP_READY = 0,
    LINK_UP_BUSY,
    LINK_UP_RETURNING,
    LINK_DOWN_NETWORK_DISCONNECT,
    LINK_DOWN_NOT_SUPPORT,
    LINK_DOWN_UNPAIRED,
    LINK_DOWN_AUTH_FAILED, //password error
    LINK_DOWN_ERROR //Error -1/-2/……

};

HotSpare::HotSpare(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::HotSpare)
{
    ui->setupUi(this);

    initHotSpare();
    ui->lineEdit_ip->setCheckMode(MyLineEdit::IPv4Check);
    ui->lineEdit_password->setCheckMode(MyLineEdit::EmptyCheck);

    onLanguageChanged();
}

HotSpare::~HotSpare()
{
    if (statusTimer) {
        statusTimer->stop();
        statusTimer->deleteLater();
        statusTimer = nullptr;
    }

    delete ui;
}

void HotSpare::hideEvent(QHideEvent *)
{
    stopStatusTimer();
}

bool HotSpare::isSlaveMode()
{
    int mode = get_param_int(SQLITE_FILE_NAME, PARAM_FAILOVER_MODE, 0);
    return mode == FAILOVER_MODE_SLAVE;
}

void HotSpare::initHotSpare()
{
    memset(m_failoverBak, 0x0, sizeof(struct ms_failover_master_status) * MAX_FAILOVER);

    ui->comboBox_mode->clear();
    ui->comboBox_mode->addItem(GET_TEXT("COMMON/1018", "Disable"), FAILOVER_MODE_DISABLE);
    ui->comboBox_mode->addItem(GET_TEXT("HOTSPARE/79007", "Master Mode"), FAILOVER_MODE_MASTER);
    ui->comboBox_mode->addItem(GET_TEXT("HOTSPARE/79008", "Slave Mode"), FAILOVER_MODE_SLAVE);

    //master list
    QStringList list;
    list << QString("")
         << GET_TEXT("CAMERASEARCH/32002", "No.")
         << GET_TEXT("COMMON/1033", "IP Address")
         << GET_TEXT("DEVICEINFO/60006", "MAC Address")
         << GET_TEXT("CHANNELMANAGE/30029", "Model");
    ui->tableView_master_list->setHorizontalHeaderLabels(list);
    ui->tableView_master_list->setColumnCount(list.size());
    connect(ui->tableView_master_list, SIGNAL(itemClicked(int, int)), this, SLOT(onListItemClicked(int, int)));
    ui->tableView_master_list->setColumnWidth(0, 100);
    ui->tableView_master_list->setColumnWidth(1, 200);
    ui->tableView_master_list->setColumnWidth(2, 400);
    ui->tableView_master_list->setColumnWidth(3, 400);
    ui->tableView_master_list->setColumnWidth(4, 200);
    ui->tableView_master_list->setMaximumHeight(180);

    //master status
    QStringList list2;
    list2 << GET_TEXT("CAMERASEARCH/32002", "No.")
          << GET_TEXT("COMMON/1033", "IP Address")
          << GET_TEXT("DEVICEINFO/60006", "MAC Address")
          << GET_TEXT("CHANNELMANAGE/30029", "Model")
          << GET_TEXT("HOTSPARE/79009", "Connection Status")
          << GET_TEXT("HOTSPARE/79010", "Working Status")
          << GET_TEXT("CHANNELMANAGE/30023", "Delete");
    ui->tableView_master_status->setHorizontalHeaderLabels(list2);
    ui->tableView_master_status->setColumnCount(list2.size());
    ui->tableView_master_status->setHeaderCheckable(false);
    ui->tableView_master_status->setSortableForColumn(6, false);
    ui->tableView_master_status->setItemDelegateForColumn(6, new ItemButtonDelegate(QPixmap(":/common/common/delete.png"), this));
    connect(ui->tableView_master_status, SIGNAL(itemClicked(int, int)), this, SLOT(onStatusItemClicked(int, int)));
    ui->tableView_master_status->setColumnWidth(0, 100);
    ui->tableView_master_status->setColumnWidth(1, 200);
    ui->tableView_master_status->setColumnWidth(2, 200);
    ui->tableView_master_status->setColumnWidth(3, 200);
    ui->tableView_master_status->setColumnWidth(4, 200);
    ui->tableView_master_status->setColumnWidth(5, 200);
    ui->tableView_master_status->setColumnWidth(6, 100);
    ui->tableView_master_status->setMaximumHeight(180);

    ui->label_ip->hide();
    ui->lineEdit_ip->hide();
    ui->label_password->hide();
    ui->lineEdit_password->hide();
    ui->label_status->hide();
    ui->lineEdit_status->hide();

    ui->groupBox_master_list->hide();
    ui->groupBox_master_status->hide();

    net_get_ifaddr("eth0", m_lan1Ip, sizeof(m_lan1Ip));
    net_get_ifaddr("eth1", m_lan2Ip, sizeof(m_lan2Ip));
    net_get_ifaddr("bond0", m_bondIp, sizeof(m_bondIp));

    statusTimer = nullptr;
}

void HotSpare::gotoHotSparePage()
{
    m_addIpc.clear();
    m_deleteIpc.clear();
    ui->tableView_master_list->clearSort();
    ui->tableView_master_status->clearSort();
    ui->tableView_master_list->clearContent();
    ui->tableView_master_status->clearContent();
    ui->lineEdit_status->clear();
    m_failoverMode = get_param_int(SQLITE_FILE_NAME, PARAM_FAILOVER_MODE, 0);
    ui->comboBox_mode->setCurrentIndex(m_failoverMode);
    on_comboBox_mode_activated(m_failoverMode);

    int count = 0;
    memset(m_failoverList, 0x0, sizeof(struct failover_list) * MAX_FAILOVER);
    read_failovers(SQLITE_FILE_NAME, m_failoverList, &count);

    memset(&m_failover, 0x0, sizeof(struct failover_list));
    read_failover(SQLITE_FILE_NAME, &m_failover, 0);

    if (m_failoverMode != FAILOVER_MODE_SLAVE) {
        ui->pushButton_search->setEnabled(false);
        ui->pushButton_add->setEnabled(false);

        ui->lineEdit_ip->setText(m_failover.ipaddr);
        ui->lineEdit_password->setText(m_failover.password);
    } else {
        ui->label_note->hide();
    }

    onStatusTimeout();
    if (m_failoverMode == FAILOVER_MODE_DISABLE) {
        stopStatusTimer();
    } else {
        startStatusTimer();
    }
}

void HotSpare::ON_RESPONSE_FLAG_FAILOVER_GET_SLAVE_STATUS(MessageReceive *message)
{
    struct ms_failover_slave_status *slave = (struct ms_failover_slave_status *)message->data;
    if (!slave) {
        qWarning() << "HotSpare::ON_RESPONSE_FLAG_FAILOVER_GET_SLAVE_STATUS, data is null.";
        return;
    }
    if (m_failoverMode != FAILOVER_MODE_MASTER) {
        return;
    }
    int status = slave->status;
    switch (status) {
    case CONNECT_INIT:
        ui->lineEdit_status->clear();
        break;
    case CONNECT_READY:
        ui->lineEdit_status->setText(GET_TEXT("HOTSPARE/79026", "Link is up (Ready)"));
        break;
    case CONNECT_BUSY:
        ui->lineEdit_status->setText(GET_TEXT("HOTSPARE/79027", "Link is up (Busy)"));
        break;
    case CONNECT_RETURN:
        ui->lineEdit_status->setText(GET_TEXT("HOTSPARE/79028", "Link is up (Returning %1%)").arg(slave->percent));
        break;
    case CONNECT_DISCONNECTED:
        ui->lineEdit_status->setText(GET_TEXT("HOTSPARE/79029", "Link is down (Network Disconnected)"));
        break;
    case CONNECT_NOT_AVAILABLE:
        ui->lineEdit_status->setText(GET_TEXT("HOTSPARE/79030", "Link is down (Not Available)"));
        break;
    case CONNECT_NOT_SUPPORTED:
        ui->lineEdit_status->setText(GET_TEXT("HOTSPARE/79037", "Link is down (Not Supported)"));
        break;
    case CONNECT_UNPAIRED:
        ui->lineEdit_status->setText(GET_TEXT("HOTSPARE/79031", "Link is down (Unpaired)"));
        break;
    case CONNECT_AUTH_FAILED:
        ui->lineEdit_status->setText(GET_TEXT("HOTSPARE/79032", "Link is down (Authentication Failed)"));
        break;
    case NO_STORAGE_SPACE:
        ui->lineEdit_status->setText(GET_TEXT("HOTSPARE/79039", "No Storage Space"));
        break;
    case CONNECT_ERROR_OTHER:
    case CONNECT_ERROR_OTHER_1:
    case CONNECT_ERROR_OTHER_2:
    case CONNECT_ERROR_OTHER_3:
    case CONNECT_ERROR_OTHER_4:
    case CONNECT_ERROR_OTHER_5:
        ui->lineEdit_status->setText(GET_TEXT("HOTSPARE/79033", "Link is down (Error-%1)").arg(status - CONNECT_ERROR_OTHER));
        break;

    default:
        ui->lineEdit_status->clear();
        break;
    }
    m_slaveStatus = status;
}

void HotSpare::ON_RESPONSE_FLAG_FAILOVER_GET_MASTER_STATUS(MessageReceive *message)
{
    struct ms_failover_master_status *master_status_array = (struct ms_failover_master_status *)message->data;
    int count = message->header.size / sizeof(struct ms_failover_master_status);
    if (!master_status_array) {
        qWarning() << "HotSpare::ON_RESPONSE_FLAG_FAILOVER_GET_MASTER_STATUS, data is null.";
        return;
    }

    if (m_failoverMode != FAILOVER_MODE_SLAVE) {
        return;
    }

    memcpy(m_failoverBak, master_status_array, sizeof(struct ms_failover_master_status) * count);

    ui->tableView_master_status->clearContent();
    for (int i = 0, rowCount = 0; i < count; i++) {
        ms_failover_master_status &master_status = master_status_array[i];
        //防止多端操作，跳过操作过的nvr
        if (m_addIpc.contains(QString(master_status.mac).toUpper()) || m_deleteIpc.contains(QString(master_status.mac).toUpper())) {
            continue;
        }
        ui->tableView_master_status->setItemIntValue(rowCount, 0, i + 1);

        ui->tableView_master_status->setItemText(rowCount, 1, QString(master_status.ipaddr));
        ui->tableView_master_status->setItemText(rowCount, 2, QString(master_status.mac).toUpper());
        ui->tableView_master_status->setItemText(rowCount, 3, QString(master_status.model));

        ui->tableView_master_status->setItemText(rowCount, 4, getConnectStatusString(master_status.connect_status));
        ui->tableView_master_status->setItemText(rowCount, 5, getWorkStatusString(master_status.work_status, master_status.percent));
        ui->tableView_master_status->setItemData(rowCount, 5, master_status.work_status == WORKING_STATUS_RETURNING, SortIntRole);
        rowCount++;
    }

    for (auto item = m_addIpc.begin(); item != m_addIpc.end(); item++) {
        int rowCount = ui->tableView_master_status->rowCount();
        ui->tableView_master_status->setItemIntValue(rowCount, 0, rowCount + 1);
        ui->tableView_master_status->setItemText(rowCount, 1, item.value().ipAddr);
        ui->tableView_master_status->setItemText(rowCount, 2, item.value().mac);
        ui->tableView_master_status->setItemText(rowCount, 3, item.value().model);
        ui->tableView_master_status->setItemText(rowCount, 4, "-");
        ui->tableView_master_status->setItemText(rowCount, 5, "-");
        ui->tableView_master_status->setItemData(rowCount, 5, static_cast<float>(0), SortIntRole);
    }
}

void HotSpare::ON_RESPONSE_FLAG_FAILOVER_SEARCH_NVR(MessageReceive *message)
{
    //closeWait();

    struct resq_search_nvr *nvrs = static_cast<struct resq_search_nvr *>(message->data);
    if (!nvrs) {
        qWarning() << "HotSpare::ON_RESPONSE_FLAG_FAILOVER_SEARCH_NVR, data is null.";
        return;
    }

    int cnt = message->header.size / sizeof(struct resq_search_nvr);
    int rowCount = 0;
    ui->tableView_master_list->clearContent();
    for (int i = 0; i < cnt; i++) {
        resq_search_nvr &nvrItem = nvrs[i];
        if (!MyQt::isValidIP(QString(nvrItem.ipaddr))) {
            continue;
        }
        //防止多端操作，跳过操作过的nvr
        if (failoversContains(nvrItem.ipaddr) || m_addIpc.contains(QString(nvrItem.mac).toUpper()) || m_deleteIpc.contains(QString(nvrItem.mac).toUpper())) {
            continue;
        }
        ui->tableView_master_list->setItemIntValue(rowCount, 1, rowCount + 1);
        ui->tableView_master_list->setItemText(rowCount, 2, QString(nvrItem.ipaddr));
        ui->tableView_master_list->setItemText(rowCount, 3, QString(nvrItem.mac).toUpper());
        ui->tableView_master_list->setItemData(rowCount, 3, nvrItem.max_camera, HotSpareMaxCameraNumberRole);
        ui->tableView_master_list->setItemText(rowCount, 4, QString(nvrItem.model));

        rowCount++;
    }
    for (auto item = m_deleteIpc.begin(); item != m_deleteIpc.end(); item++) {
        ui->tableView_master_list->setItemIntValue(rowCount, 1, rowCount + 1);
        ui->tableView_master_list->setItemText(rowCount, 2, item.value().ipAddr);
        ui->tableView_master_list->setItemText(rowCount, 3, item.value().mac);
        ui->tableView_master_list->setItemData(rowCount, 3, item.value().maxCamera, HotSpareMaxCameraNumberRole);
        ui->tableView_master_list->setItemText(rowCount, 4, item.value().model);
        rowCount++;
    }
}

void HotSpare::ON_RESPONSE_FLAG_FAILOVER_CHANGE_MODE(MessageReceive *message)
{
    Q_UNUSED(message);
}

void HotSpare::ON_RESPONSE_FLAG_FAILOVER_UPDATE_MASTER(MessageReceive *message)
{
    Q_UNUSED(message);
    m_eventLoop.exit(0);
}

int HotSpare::getSelectMaster()
{
    int count = 0;

    for (int i = 0; i < ui->tableView_master_list->rowCount(); i++) {
        if (ui->tableView_master_list->isItemChecked(i)) {
            count++;
        }
    }

    return count;
}

void HotSpare::writeLog(int action, char *ip)
{
    struct log_data log_data;
    memset(&log_data, 0, sizeof(struct log_data));
    struct detail_failover_operation info;
    memset(&info, 0x0, sizeof(struct detail_failover_operation));

    if (action == 0) {
        log_data.log_data_info.subType = SUP_OP_FAILOVER_SLAVE_ADD_MASTER;
        info.action = OP_FAILOVER_ADD_MASTER;
    } else if (action == 1) {
        log_data.log_data_info.subType = SUP_OP_FAILOVER_SLAVE_DEL_MASTER;
        info.action = OP_FAILOVER_DEL_MASTER;
    } else {
        return;
    }

    log_data.log_data_info.parameter_type = SUB_PARAM_NONE; //SUB_PARAM_DEL_MASTER;
    snprintf(log_data.log_data_info.user, sizeof(log_data.log_data_info.user), "%s", gMsUser.userName().toStdString().c_str());
    snprintf(info.ipaddr, sizeof(info.ipaddr), "%s", ip);
    msfs_log_pack_detail(&log_data, OP_FAILOVER_OPERATION, &info, sizeof(struct detail_failover_operation));

    MsWriteLog(log_data);
}

bool HotSpare::isSlaveParaValid()
{
    bool valid = true;
    valid &= ui->lineEdit_ip->checkValid();
    QString ip = ui->lineEdit_ip->text();
    if (valid && (!ip.compare(QString(m_lan1Ip)) || !ip.compare(QString(m_lan2Ip)) || !ip.compare(QString(m_bondIp)))) {
        ui->lineEdit_ip->setCustomValid(false, GET_TEXT("MYLINETIP/112001", "Invalid."));
        valid = false;
    }
    valid &= ui->lineEdit_password->checkValid();
    return valid;
}

bool HotSpare::failoversContains(QString ipAddr)
{
    for (int j = 0; j < MAX_FAILOVER; j++) {
        if (m_failoverList[j].enable == 0) {
            continue;
        }
        if (!QString(m_failoverList[j].ipaddr).compare(ipAddr)) {
            return true;
        }
    }
    return false;
}

QString HotSpare::getConnectStatusString(int status)
{
    QString str;
    switch (status) {
    case CONNECT_STATUS_ONLINE:
        str = GET_TEXT("HOTSPARE/79014", "Online");
        break;
    case CONNECT_STATUS_OFFLINE:
        str = GET_TEXT("HOTSPARE/79012", "Offline");
        break;
    default:
        str = "-";
    }
    return str;
}

QString HotSpare::getWorkStatusString(int status, float percent)
{
    QString str;
    switch (status) {
    case WORKING_STATUS_NO_NEED:
        str = GET_TEXT("HOTSPARE/79040", "Master Hot Spare Disabled");
        break;
    case WORKING_STATUS_NORMAL:
        str = GET_TEXT("DISKMANAGE/72099", "Normal");
        break;
    case WORKING_STATUS_BACKING_UP:
        str = GET_TEXT("HOTSPARE/79013", "Backing up");
        break;
    case WORKING_STATUS_RETURNING:
        str = GET_TEXT("HOTSPARE/79015", "Returning (%1%)").arg(QString().setNum(percent, 'f', 1));
        break;
    case WORKING_STATUS_SLAVE_BUSY:
        str = GET_TEXT("HOTSPARE/79034", "Slave is busy");
        break;
    case WORKING_STATUS_NO_SPACE:
        str = GET_TEXT("HOTSPARE/79039", "No Storage Space");
        break;
    case WORKING_STATUS_BANDWIDTH:
        str = GET_TEXT("LOG/64064", "Error");
        break;
    default:
        str = "-";
    }
    return str;
}

void HotSpare::saveDisableMode(int beforeModel)
{
    int mode = FAILOVER_MODE_DISABLE;
    struct ms_failover_change_mode info;

    info.mode = mode;
    if (beforeModel == mode) {
        return;
    } else if (beforeModel == FAILOVER_MODE_SLAVE) {
        int result = MessageBox::question(this, GET_TEXT("HOTSPARE/160001", "NVR will reboot after function is disabled, continue?"));
        if (result == MessageBox::Yes) {
            //showWait();
            set_param_int(SQLITE_FILE_NAME, PARAM_FAILOVER_MODE, mode);
            memset(&m_failover, 0x0, sizeof(struct failover_list));
            write_failover(SQLITE_FILE_NAME, &m_failover);
            Q_UNUSED(info)
            m_failoverMode = mode;
        }
    } else if (beforeModel == FAILOVER_MODE_MASTER) {
        if (m_slaveStatus == CONNECT_RETURN) {
            int result = MessageBox::question(this, GET_TEXT("HOTSPARE/160005", "Deleting master will cause video loss when video is being returned, continue?"));
            if (result == MessageBox::Cancel) {
                return;
            }
        }
        set_param_int(SQLITE_FILE_NAME, PARAM_FAILOVER_MODE, mode);
        sendMessage(REQUEST_FLAG_FAILOVER_CHANGE_MODE, &info, sizeof(struct ms_failover_change_mode));

        waitSecond();
        gotoHotSparePage();
    }
}

void HotSpare::saveMasterMode(int beforeModel)
{
    int mode = FAILOVER_MODE_MASTER;
    struct ms_failover_change_mode info;

    info.mode = mode;
    if (!isSlaveParaValid()) {
        return;
    }

    snprintf(info.ipaddr, sizeof(info.ipaddr), "%s", ui->lineEdit_ip->text().trimmed().toStdString().c_str());
    snprintf(info.password, sizeof(info.password), "%s", ui->lineEdit_password->text().trimmed().toStdString().c_str());
    if (!QString(m_failover.ipaddr).compare(info.ipaddr) && !QString(m_failover.password).compare(info.password) && mode == beforeModel) {
        //none of change
        return;
    }

    if (beforeModel == FAILOVER_MODE_SLAVE) {
        //need to reboot
        int result = MessageBox::question(this, GET_TEXT("HOTSPARE/160002", "NVR will reboot after switching to Master Mode, continue?"));
        if (result == MessageBox::Yes) {
            set_param_int(SQLITE_FILE_NAME, PARAM_FAILOVER_MODE, mode);
            snprintf(m_failover.ipaddr, sizeof(m_failover.ipaddr), "%s", ui->lineEdit_ip->text().trimmed().toStdString().c_str());
            snprintf(m_failover.password, sizeof(m_failover.password), "%s", ui->lineEdit_password->text().trimmed().toStdString().c_str());
            write_failover(SQLITE_FILE_NAME, &m_failover);
            sendMessage(REQUEST_FLAG_FAILOVER_CHANGE_MODE, (void *)&info, sizeof(struct ms_failover_change_mode));
            m_failoverMode = mode;
        }
    } else {
        if (m_slaveStatus == CONNECT_RETURN) {
            int result = MessageBox::question(this, GET_TEXT("HOTSPARE/160005", "Deleting master will cause video loss when video is being returned, continue?"));
            if (result == MessageBox::Cancel) {
                return;
            }
        }
        set_param_int(SQLITE_FILE_NAME, PARAM_FAILOVER_MODE, mode);
        snprintf(m_failover.ipaddr, sizeof(m_failover.ipaddr), "%s", ui->lineEdit_ip->text().trimmed().toStdString().c_str());
        snprintf(m_failover.password, sizeof(m_failover.password), "%s", ui->lineEdit_password->text().trimmed().toStdString().c_str());
        write_failover(SQLITE_FILE_NAME, &m_failover);
        sendMessage(REQUEST_FLAG_FAILOVER_CHANGE_MODE, (void *)&info, sizeof(struct ms_failover_change_mode));

        waitSecond();
        gotoHotSparePage();
    }
}

void HotSpare::saveSlaveMode(int beforeModel)
{
    int mode = FAILOVER_MODE_SLAVE;
    struct ms_failover_change_mode info;
    info.mode = mode;

    if (m_slaveStatus == CONNECT_RETURN) {
        int result = MessageBox::question(this, GET_TEXT("HOTSPARE/79035", "Video is returning, device change will cause video lost, continue?"));
        if (result == MessageBox::Cancel) {
            return;
        }
    }
    if (beforeModel != mode) {
        int result = MessageBox::question(this, GET_TEXT("HOTSPARE/160000", "NVR will reboot and channel settings will be erased after switching to Slave Mode, continue?"));
        if (result == MessageBox::Yes) {
            set_param_int(SQLITE_FILE_NAME, PARAM_FAILOVER_MODE, mode);
            m_failoverMode = mode;
            Q_UNUSED(info)
            waitSecond();
        }
    } else {
        Uint32 masterMask = 0;
        for (auto iter = m_deleteIpc.begin(); iter != m_deleteIpc.end(); iter++) {
            for (int i = 0; i < MAX_FAILOVER; i++) {
                if (m_failoverList[i].enable == 0) {
                    continue;
                }
                if (!iter.value().ipAddr.compare(m_failoverList[i].ipaddr)) {
                    masterMask |= (1 << i);
                    m_failoverList[i].enable = 0;
                    write_failover(SQLITE_FILE_NAME, &m_failoverList[i]);
                    writeLog(1, m_failoverList[i].ipaddr);
                    break;
                }
            }
        }
        sendMessage(REQUEST_FLAG_FAILOVER_UPDATE_MASTER, &masterMask, sizeof(Uint32));
        //m_eventLoop.exec();
        for (auto iter = m_addIpc.begin(); iter != m_addIpc.end(); iter++) {
            for (int j = 0; j < MAX_FAILOVER; j++) {
                if (m_failoverList[j].enable == 1) {
                    continue;
                }
                m_failoverList[j].enable = 1;
                snprintf(m_failoverList[j].ipaddr, sizeof(m_failoverList[j].ipaddr), "%s", iter.value().ipAddr.toStdString().c_str());
                snprintf(m_failoverList[j].mac, sizeof(m_failoverList[j].mac), "%s", iter.value().mac.toStdString().c_str());
                snprintf(m_failoverList[j].model, sizeof(m_failoverList[j].model), "%s", iter.value().model.toStdString().c_str());
                m_failoverList[j].maxCamera = iter.value().maxCamera;
                write_failover(SQLITE_FILE_NAME, &m_failoverList[j]);
                writeLog(0, m_failoverList[j].ipaddr);
                break;
            }
        }
        masterMask = 0;
        sendMessage(REQUEST_FLAG_FAILOVER_UPDATE_MASTER, &masterMask, sizeof(Uint32));
        //m_eventLoop.exec();

        waitSecond();
        gotoHotSparePage();
    }
}

void HotSpare::waitSecond()
{
    //showWait();
    // QEventLoop eventloop;
    // QTimer::singleShot(1000, &eventloop, SLOT(quit()));
    //eventloop.exec();
    //closeWait();
}

void HotSpare::stopStatusTimer()
{
    if (statusTimer) {
        statusTimer->stop();
    }
}

void HotSpare::startStatusTimer()
{
    if (!statusTimer) {
        statusTimer = new QTimer(this);
        connect(statusTimer, SIGNAL(timeout()), this, SLOT(onStatusTimeout()));
    }
    statusTimer->start(3000);
}

void HotSpare::initializeData()
{
    gotoHotSparePage();
}

void HotSpare::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_FAILOVER_GET_SLAVE_STATUS:
        ON_RESPONSE_FLAG_FAILOVER_GET_SLAVE_STATUS(message);
        break;
    case RESPONSE_FLAG_FAILOVER_GET_MASTER_STATUS:
        ON_RESPONSE_FLAG_FAILOVER_GET_MASTER_STATUS(message);
        break;
    case RESPONSE_FLAG_FAILOVER_SEARCH_NVR:
        ON_RESPONSE_FLAG_FAILOVER_SEARCH_NVR(message);
        break;
    case RESPONSE_FLAG_FAILOVER_CHANGE_MODE:
        ON_RESPONSE_FLAG_FAILOVER_CHANGE_MODE(message);
        break;
    case RESPONSE_FLAG_FAILOVER_UPDATE_MASTER:
        ON_RESPONSE_FLAG_FAILOVER_UPDATE_MASTER(message);
        break;
    }
}

void HotSpare::onLanguageChanged()
{
    ui->label_mode->setText(GET_TEXT("HOTSPARE/79001", "Hot Spare Mode"));
    ui->label_ip->setText(GET_TEXT("HOTSPARE/79002", "Slave IP Address"));
    ui->label_password->setText(GET_TEXT("HOTSPARE/79003", "Slave Admin Password"));
    ui->label_status->setText(GET_TEXT("HOTSPARE/79004", "Slave Status"));

    ui->comboBox_mode->setItemText(0, GET_TEXT("COMMON/1018", "Disable"));
    ui->comboBox_mode->setItemText(1, GET_TEXT("HOTSPARE/79007", "Master Mode"));
    ui->comboBox_mode->setItemText(2, GET_TEXT("HOTSPARE/79008", "Slave Mode"));

    ui->groupBox_master_list->setTitle(GET_TEXT("HOTSPARE/79005", "Master List"));

    ui->pushButton_search->setText(GET_TEXT("CAMERASEARCH/32001", "Search"));
    ui->pushButton_add->setText(GET_TEXT("COMMON/1000", "Add"));

    ui->groupBox_master_status->setTitle(GET_TEXT("HOTSPARE/79006", "Master Status"));

    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->label_note->setText(GET_TEXT("HOTSPARE/160003", "Note: After enabling the hot spare function, you need to add the Master to the Slave NVR. Otherwise, it will not work properly."));

    m_slaveStatus = 0;

    QStringList list;
    list << QString("")
         << GET_TEXT("CAMERASEARCH/32002", "No.")
         << GET_TEXT("COMMON/1033", "IP Address")
         << GET_TEXT("DEVICEINFO/60006", "MAC Address")
         << GET_TEXT("CHANNELMANAGE/30029", "Model");
    ui->tableView_master_list->setHorizontalHeaderLabels(list);
    ui->tableView_master_list->setColumnWidth(0, 100);
    ui->tableView_master_list->setColumnWidth(1, 200);
    ui->tableView_master_list->setColumnWidth(2, 400);
    ui->tableView_master_list->setColumnWidth(3, 400);
    ui->tableView_master_list->setColumnWidth(4, 200);

    //status_model->clear();
    QStringList list2;
    list2 << GET_TEXT("CAMERASEARCH/32002", "No.")
          << GET_TEXT("COMMON/1033", "IP Address")
          << GET_TEXT("DEVICEINFO/60006", "MAC Address")
          << GET_TEXT("CHANNELMANAGE/30029", "Model")
          << GET_TEXT("HOTSPARE/79009", "Connection Status")
          << GET_TEXT("HOTSPARE/79010", "Working Status")
          << GET_TEXT("CHANNELMANAGE/30023", "Delete");
    ui->tableView_master_status->setHorizontalHeaderLabels(list2);
    ui->tableView_master_status->setColumnWidth(0, 100);
    ui->tableView_master_status->setColumnWidth(1, 200);
    ui->tableView_master_status->setColumnWidth(2, 200);
    ui->tableView_master_status->setColumnWidth(3, 200);
    ui->tableView_master_status->setColumnWidth(4, 200);
    ui->tableView_master_status->setColumnWidth(5, 200);
    ui->tableView_master_status->setColumnWidth(6, 100);
}

void HotSpare::onListItemClicked(int row, int column)
{
    Q_UNUSED(row)
    Q_UNUSED(column)
}

void HotSpare::onStatusItemClicked(int row, int column)
{
    if (!isVisible()) {
        return;
    }

    if (cursor().shape() == Qt::WaitCursor) {
        return;
    }
    if (!ui->tableView_master_status->currentIndex().isValid()) {
        return;
    }

    //delete
    if (column == 6) {
        QString workStatus = ui->tableView_master_status->itemText(row, column);
        bool isReturn = ui->tableView_master_status->itemData(row, 5, SortIntRole).toInt();
        if (isReturn) {
            int result = MessageBox::question(this, GET_TEXT("HOTSPARE/160005", "Deleting master will cause video loss when video is being returned, continue?"));
            if (result == MessageBox::Cancel) {
                return;
            }
        }
        QString ip = ui->tableView_master_status->itemText(row, 1);
        QString mac = ui->tableView_master_status->itemText(row, 2);
        QString model = ui->tableView_master_status->itemText(row, 3);
        int maxCamera = ui->tableView_master_status->itemData(row, 3, HotSpareMaxCameraNumberRole).toInt();
        //如果add中已存在，说明原先在List表中的，等于没有修改
        if (!m_addIpc.contains(mac)) {
            m_deleteIpc.insert(mac, HotSpareIPCInfo(ip, mac, model, maxCamera));
        } else {
            m_addIpc.remove(mac);
        }
        ui->tableView_master_status->removeRow(row);
    }
}

void HotSpare::on_comboBox_mode_activated(int index)
{
    int mode = ui->comboBox_mode->itemData(index).toInt();
    switch (mode) {
    case FAILOVER_MODE_DISABLE:
        ui->label_ip->hide();
        ui->lineEdit_ip->hide();
        ui->label_password->hide();
        ui->lineEdit_password->hide();
        ui->label_status->hide();
        ui->lineEdit_status->hide();

        ui->groupBox_master_list->hide();
        ui->groupBox_master_status->hide();
        ui->label_note->show();
        break;
    case FAILOVER_MODE_MASTER:
        ui->groupBox_master_list->hide();
        ui->groupBox_master_status->hide();
        ui->label_note->show();

        ui->label_ip->show();
        ui->lineEdit_ip->show();
        ui->label_password->show();
        ui->lineEdit_password->show();
        ui->label_status->show();
        ui->lineEdit_status->show();
        break;
    case FAILOVER_MODE_SLAVE:
        ui->label_ip->hide();
        ui->lineEdit_ip->hide();
        ui->label_password->hide();
        ui->lineEdit_password->hide();
        ui->label_status->hide();
        ui->lineEdit_status->hide();
        ui->label_note->hide();

        ui->groupBox_master_list->show();
        ui->groupBox_master_status->show();
        break;
    }
}

void HotSpare::on_pushButton_search_clicked()
{
    sendMessage(REQUEST_FLAG_FAILOVER_SEARCH_NVR, nullptr, 0);
    //showWait();
}

void HotSpare::on_pushButton_add_clicked()
{
    int count = getSelectMaster();
    if (count == 0) {
        ShowMessageBox(GET_TEXT("HOTSPARE/160004", "Please select one master NVR at least."));
        return;
    }

    if (count + ui->tableView_master_status->rowCount() > MAX_FAILOVER) {
        ShowMessageBox(GET_TEXT("HOTSPARE/79025", "Only supports 32 master at the most"));
        return;
    }

    for (int i = 0; i < ui->tableView_master_list->rowCount(); i++) {
        if (ui->tableView_master_list->isItemChecked(i)) {
            QString ip = ui->tableView_master_list->itemText(i, 2);
            QString mac = ui->tableView_master_list->itemText(i, 3);
            QString model = ui->tableView_master_list->itemText(i, 4);
            int maxCamera = ui->tableView_master_list->itemData(i, 3, HotSpareMaxCameraNumberRole).toInt();
            ui->tableView_master_list->removeRow(i);
            i--;

            //表面添加，实际保存后才生效
            int rowCount = ui->tableView_master_status->rowCount();
            ui->tableView_master_status->setItemIntValue(rowCount, 0, rowCount + 1);
            ui->tableView_master_status->setItemText(rowCount, 1, ip);
            ui->tableView_master_status->setItemText(rowCount, 2, mac);
            ui->tableView_master_status->setItemText(rowCount, 3, model);
            ui->tableView_master_status->setItemData(rowCount, 3, maxCamera, HotSpareMaxCameraNumberRole);
            ui->tableView_master_status->setItemText(rowCount, 4, "-");
            ui->tableView_master_status->setItemText(rowCount, 5, "-");
            ui->tableView_master_status->setItemData(rowCount, 5, static_cast<float>(0), SortIntRole);

            //如果delete中已存在，说明是原先在Status表中的，等于没有修改
            if (!m_deleteIpc.contains(mac)) {
                m_addIpc.insert(mac, HotSpareIPCInfo(ip, mac, model, maxCamera));
            } else {
                m_deleteIpc.remove(mac);
            }
        }
    }
    ui->tableView_master_list->setHeaderChecked(false);
}

void HotSpare::on_pushButton_apply_clicked()
{
    int mode = ui->comboBox_mode->currentIndex();
    if (mode == FAILOVER_MODE_DISABLE) {
        saveDisableMode(m_failoverMode);
    } else if (mode == FAILOVER_MODE_MASTER) {
        saveMasterMode(m_failoverMode);
    } else if (mode == FAILOVER_MODE_SLAVE) {
        saveSlaveMode(m_failoverMode);
    }
}

void HotSpare::on_pushButton_back_clicked()
{
    emit sig_back();
}

void HotSpare::onStatusTimeout()
{
    if (m_failoverMode == FAILOVER_MODE_MASTER) {
        sendMessage(REQUEST_FLAG_FAILOVER_GET_SLAVE_STATUS, nullptr, 0);
    } else if (m_failoverMode == FAILOVER_MODE_SLAVE) {
        sendMessage(REQUEST_FLAG_FAILOVER_GET_MASTER_STATUS, nullptr, 0);
    }
}
