#include "DeviceSearch.h"
#include "ui_DeviceSearch.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "centralmessage.h"
#include "myqt.h"
#include <QFile>
#include <QHostAddress>
#include <QtDebug>

extern "C" {
#include "recortsp.h"
}

const int SearchCameraInfoRole = Qt::UserRole + 10;

DeviceSearch::DeviceSearch(QWidget *parent)
    : AbstractCameraPage(parent)
    , ui(new Ui::DeviceSearch)
{
    ui->setupUi(this);

    //table
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("CAMERASEARCH/32002", "NO.");
    headerList << GET_TEXT("COMMON/1033", "IP Address");
    headerList << GET_TEXT("CAMERASEARCH/32003", "IP Edit");
    headerList << GET_TEXT("CAMERASEARCH/32044", "Status");
    headerList << GET_TEXT("CHANNELMANAGE/30011", "Port");
    headerList << GET_TEXT("CHANNELMANAGE/30014", "Protocol");
    headerList << GET_TEXT("CAMERASEARCH/30012", "NIC");
    headerList << GET_TEXT("CHANNELMANAGE/30027", "MAC");
    headerList << GET_TEXT("CHANNELMANAGE/30054", "Firmware Version");
    headerList << GET_TEXT("CHANNELMANAGE/30029", "Model");
    headerList << GET_TEXT("CHANNELMANAGE/30028", "Vendor");
    headerList << QString("SN");
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setColumnCount(headerList.size());
    connect(ui->tableView, SIGNAL(itemClicked(int, int)), this, SLOT(onTableItemClicked(int, int)));
    connect(ui->tableView, SIGNAL(headerChecked(bool)), this, SLOT(onTableHeaderChecked(bool)));
    //delegate
    ui->tableView->setItemDelegateForColumn(ColumnEdit, new ItemButtonDelegate(QPixmap(":/common/common/edit.png"), this));
    //sort
    ui->tableView->setSortableForColumn(ColumnCheck, false);
    ui->tableView->setSortableForColumn(ColumnEdit, false);
    ui->tableView->setSortType(ColumnNumber, SortFilterProxyModel::SortInt);
    ui->tableView->setSortType(ColumnIP, SortFilterProxyModel::SortIP);
    ui->tableView->setSortType(ColumnPort, SortFilterProxyModel::SortPort);
    //
    ui->tableView->setColumnWidth(ColumnCheck, 50);
    ui->tableView->setColumnWidth(ColumnNumber, 60);
    ui->tableView->setColumnWidth(ColumnEdit, 70);
    ui->tableView->setColumnWidth(ColumnStatus, 70);
    ui->tableView->setColumnWidth(ColumnIP, 150);
    ui->tableView->setColumnWidth(ColumnPort, 60);
    ui->tableView->setColumnWidth(ColumnProtocol, 100);
    ui->tableView->setColumnWidth(ColumnNIC, 100);
    ui->tableView->setColumnWidth(ColumnMAC, 140);
    ui->tableView->setColumnWidth(ColumnFirmware, 200);
    ui->tableView->setColumnWidth(ColumnModel, 200);
    ui->tableView->setColumnWidth(ColumnVendor, 260);
    ui->tableView->setColumnWidth(ColumnSN, 200);

    //table poe
    QStringList headerListPoe;
    headerListPoe << "";
    headerListPoe << GET_TEXT("CAMERASEARCH/32024", "PoE Channel");
    headerListPoe << GET_TEXT("COMMON/1019", "Edit");
    headerListPoe << GET_TEXT("CAMERASEARCH/32044", "Status");
    headerListPoe << GET_TEXT("COMMON/1033", "IP Address");
    headerListPoe << GET_TEXT("CHANNELMANAGE/30011", "Port");
    headerListPoe << GET_TEXT("CHANNELMANAGE/30014", "Protocol");
    headerListPoe << GET_TEXT("CHANNELMANAGE/30027", "MAC");
    headerListPoe << GET_TEXT("CHANNELMANAGE/30054", "Firmware Version");
    headerListPoe << GET_TEXT("CHANNELMANAGE/30029", "Model");
    headerListPoe << GET_TEXT("CHANNELMANAGE/30028", "Vendor");
    ui->tableView_poe->setHorizontalHeaderLabels(headerListPoe);
    ui->tableView_poe->setColumnCount(headerListPoe.size());
    ui->tableView_poe->hideColumn(PoeColumnCheck);
    connect(ui->tableView_poe, SIGNAL(itemClicked(int, int)), this, SLOT(onTablePoeItemClicked(int, int)));
    //delegate
    ui->tableView_poe->setItemDelegateForColumn(PoeColumnEdit, new ItemButtonDelegate(QPixmap(":/common/common/edit_white.png"), this));
    //sort
    ui->tableView_poe->setSortableForColumn(PoeColumnEdit, false);
    ui->tableView_poe->setSortType(PoeColumnIP, SortFilterProxyModel::SortIP);
    //
    ui->tableView_poe->setColumnWidth(PoeColumnChannel, 100);
    ui->tableView_poe->setColumnWidth(PoeColumnEdit, 70);
    ui->tableView_poe->setColumnWidth(PoeColumnStatus, 70);
    ui->tableView_poe->setColumnWidth(PoeColumnIP, 150);
    ui->tableView_poe->setColumnWidth(PoeColumnPort, 60);
    ui->tableView_poe->setColumnWidth(PoeColumnProtocol, 100);
    ui->tableView_poe->setColumnWidth(PoeColumnMAC, 140);
    ui->tableView_poe->setColumnWidth(PoeColumnFirmware, 200);
    ui->tableView_poe->setColumnWidth(PoeColumnModel, 200);
    //
    ui->tableView_poe->setAlternatingRowColors(false);
    ui->tableView_poe->hide();

    //
    ui->lineEdit_ip_max->setCheckMode(MyLineEdit::IPCheck);
    ui->lineEdit_ip_min->setCheckMode(MyLineEdit::IPCheck);

    //
    initializeProtocol();
    initializeNIC();

    //
    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

DeviceSearch::~DeviceSearch()
{
    if (m_camera_array) {
        delete[] m_camera_array;
        m_camera_array = nullptr;
    }
    //
    delete ui;
}

void DeviceSearch::initializeData()
{
    memset(&m_dbNetwork, 0, sizeof(network));
    read_network(SQLITE_FILE_NAME, &m_dbNetwork);
    ui->tableView->clearSort();
    ui->tableView_poe->clearSort();

    ui->lineEdit_ip_min->setText("0.0.0.0");
    ui->lineEdit_ip_max->setText("255.255.255.255");
    ui->comboBox_protocol->setCurrentIndex(0);
    ui->comboBox_nic->setCurrentIndex(0);

    on_pushButton_search_clicked();
    if (m_isShowInWizard && qMsNvr->isPoe()) {
        ui->tableView_poe->show();
        sendMessage(REQUEST_FLAG_GET_IPCLIST, (void *)NULL, 0);
    }
}

void DeviceSearch::setShowInWizard()
{
    m_isShowInWizard = true;
    ui->tableView->setAlternatingRowColors(false);
    ui->tableView->setItemDelegateForColumn(ColumnEdit, new ItemButtonDelegate(QPixmap(":/common/common/edit_white.png"), this));
    ui->pushButton_back->hide();
}

void DeviceSearch::setShowInLiveView()
{
    //ui->pushButton_back->hide();
}

void DeviceSearch::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SEARCH_IPC:
        ON_RESPONSE_FLAG_SEARCH_IPC(message);
        break;
    case RESPONSE_FLAG_GET_IPCLIST:
        ON_RESPONSE_FLAG_GET_IPCLIST(message);
        break;
    case RESPONSE_FLAG_GET_CAMERA_CHALLENGE:
        ON_RESPONSE_FLAG_GET_CAMERA_CHALLENGE(message);
        break;
    default:
        break;
    }
}

void DeviceSearch::ON_RESPONSE_FLAG_SEARCH_IPC(MessageReceive *message)
{
    struct resq_search_ipc *search_ipc_array = (struct resq_search_ipc *)message->data;
    int count = message->header.size / sizeof(struct resq_search_ipc);

    quint32 minAddress = QHostAddress(ui->lineEdit_ip_min->text()).toIPv4Address();
    quint32 maxAddress = QHostAddress(ui->lineEdit_ip_max->text()).toIPv4Address();
    for (int i = 0; i < count; ++i) {
        const resq_search_ipc &ipc = search_ipc_array[i];
        quint32 address = QHostAddress(ipc.ipaddr).toIPv4Address();
        if (address >= minAddress && address <= maxAddress) {
            CameraKey key;
            key.ip = address;
            key.mac = ipc.mac;
            if (m_cameraMap.contains(key)) {
                resq_search_ipc imapipc = m_cameraMap.value(key);
                if (qMsNvr->isPoe() && !strcmp(ipc.netif_from, "eth1")) {
                    m_cameraMap.insert(key, ipc);
                } else {
                    if (imapipc.protocol < ipc.protocol
                        && !strcmp(imapipc.netif_from, ipc.netif_from)) {
                        m_cameraMap.remove(key);
                        m_cameraMap.insert(key, ipc);
                    }
                }
            } else {
                m_cameraMap.insert(key, ipc);
                if (m_cameraMapAllIp.contains(QString(ipc.ipaddr))) {
                    m_cameraMapRepeatIp.insert(ipc.ipaddr, ipc.ipaddr);
                } else {
                    m_cameraMapAllIp.insert(ipc.ipaddr, ipc.ipaddr);
                }
            }
        }
    }

    if (m_searchProtocolList.isEmpty()) {
        m_cameraMapAllIp.clear();
        showCameraList();
        ui->tableView->reorder();
        //MsWaitting::closeGlobalWait();
    } else {
        int id = m_searchProtocolList.takeFirst();
        Q_UNUSED(id)
    }
}

void DeviceSearch::ON_RESPONSE_FLAG_GET_IPCLIST(MessageReceive *message)
{
    //MsWaitting::closeGlobalWait();
    ui->tableView_poe->clearContent();

    resq_get_ipcdev *ipcdev_array = (resq_get_ipcdev *)message->data;
    if (!ipcdev_array) {
        return;
    }
    int count = message->header.size / sizeof(resq_get_ipcdev);
    int row = 0;
    for (int i = 0; i < count; ++i) {
        const resq_get_ipcdev &ipcdev = ipcdev_array[i];
        if (ipcdev.physical_port == 0) {
            continue;
        }
        ui->tableView_poe->insertRow(row);
        //channel
        ui->tableView_poe->setItemIntValue(row, PoeColumnChannel, ipcdev.chanid + 1);
        //status
        QString strState;
        if (ipcdev.state == RTSP_CLIENT_CONNECT) {
            strState = GET_TEXT("CHANNELMANAGE/30038", "Connected");
        } else {
            strState = GET_TEXT("CHANNELMANAGE/30039", "Disconnected");
        }
        ui->tableView_poe->setItemText(row, PoeColumnStatus, strState);
        //ip
        ui->tableView_poe->setItemText(row, PoeColumnIP, ipcdev.ipaddr);
        //port, protocol
        if (ipcdev.protocol == IPC_PROTOCOL_RTSP) {
            ui->tableView_poe->setItemIntValue(row, PoeColumnPort, ipcdev.main_rtsp_port);
            ui->tableView_poe->setItemText(row, PoeColumnProtocol, "RTSP");
        } else {
            ui->tableView_poe->setItemIntValue(row, PoeColumnPort, ipcdev.port);
            ui->tableView_poe->setItemText(row, PoeColumnProtocol, qMsNvr->protocolName(ipcdev.protocol));
        }
        if (qMsNvr->isPoe()) {
            if (ipcdev.poe_channel) {
                ui->tableView_poe->setItemIntValue(row, PoeColumnPort, ipcdev.physical_port);
            } else if (!ipcdev.poe_channel && ipcdev.physical_port) {
                if (ipcdev.protocol == IPC_PROTOCOL_RTSP) {
                    ui->tableView_poe->setItemText(row, PoeColumnPort, QString("%1, %2").arg(ipcdev.physical_port).arg(ipcdev.physical_port));
                } else {
                    ui->tableView_poe->setItemText(row, PoeColumnPort, QString("%1, %2").arg(ipcdev.physical_port).arg(ipcdev.port));
                }
            }
        }
        //mac
        ui->tableView_poe->setItemText(row, PoeColumnMAC, ipcdev.mac);
        //firmware
        ui->tableView_poe->setItemText(row, PoeColumnFirmware, ipcdev.fwversion);
        //model
        ui->tableView_poe->setItemText(row, PoeColumnModel, ipcdev.model);
        //vendor
        ui->tableView_poe->setItemText(row, PoeColumnVendor, ipcdev.manufacturer);
        //sn
        ui->tableView_poe->setItemText(row, PoeColumnSN, ipcdev.real_sn);
        QString tipText = ipcdev.real_sn;
        int p = 64;
        while(p < QString(ipcdev.real_sn).size()){
            tipText.insert(p, "\n");
            p+=64;
        }
        if (tipText != "-") {
            ui->tableView_poe->setItemToolTip(row, PoeColumnSN, tipText);
        }
    }
    ui->tableView_poe->sortByColumn(PoeColumnChannel, Qt::AscendingOrder);
}

void DeviceSearch::initializeProtocol()
{
    ui->comboBox_protocol->clear();

    m_protocolMap.clear();
    ipc_protocol *protocol_array = nullptr;
    int count = 0;
    read_ipc_protocols(SQLITE_FILE_NAME, &protocol_array, &count);
    if (protocol_array) {
        for (int i = 0; i < count; ++i) {
            const ipc_protocol &protocol = protocol_array[i];
            if (protocol.enable && protocol.function & IPC_FUNC_SEARCHABLE) {
                ui->comboBox_protocol->addItem(protocol.pro_name, protocol.pro_id);
                m_protocolMap.insert(protocol.pro_id, protocol);
            }
        }
        if (ui->comboBox_protocol->count() > 1) {
            ui->comboBox_protocol->insertItem(0, GET_TEXT("COMMON/1006", "All"), -1);
            ui->comboBox_protocol->setCurrentIndexFromData(-1);
        }
    }
    release_ipc_protocol(&protocol_array);
}

void DeviceSearch::initializeNIC()
{
    ui->comboBox_nic->clear();

    const device_info &device = qMsNvr->deviceInfo();
    const network &networkInfo = qMsNvr->networkInfo();
    if (device.max_lan > 1) {
        if (qMsNvr->isPoe()) {
            ui->comboBox_nic->addItem(GET_TEXT("COMMON/1006", "All"), -1);
            ui->comboBox_nic->addItem(GET_TEXT("CAMERASEARCH/32031", "LAN"), 0);
            ui->comboBox_nic->addItem(GET_TEXT("CAMERASEARCH/32038", "PoE"), 1);
        } else {
            if (networkInfo.mode == NETMODE_MULTI) {
                ui->comboBox_nic->addItem(GET_TEXT("COMMON/1006", "All"), -1);
                ui->comboBox_nic->addItem(GET_TEXT("CAMERASEARCH/32022", "LAN1"), 0);
                ui->comboBox_nic->addItem(GET_TEXT("CAMERASEARCH/32023", "LAN2"), 1);
            } else {
                ui->comboBox_nic->addItem("Bond0", 0);
            }
        }
    } else {
        ui->comboBox_nic->addItem(GET_TEXT("CAMERASEARCH/32022", "LAN1"), 0);
    }
}

void DeviceSearch::updateCameraMap(const req_set_ipcaddr &ipcAddr)
{
    qDebug() << QString("DeviceSearch::updateCameraMap, mac: %1, oldAddr: %2, newAddr: %3").arg(ipcAddr.mac).arg(ipcAddr.oldipaddr).arg(ipcAddr.newipaddr);

    CameraKey key;
    key.ip = QHostAddress(ipcAddr.oldipaddr).toIPv4Address();
    key.mac = ipcAddr.mac;

    if (m_cameraMap.contains(key)) {
        resq_search_ipc &ipc = m_cameraMap[key];
        key.ip = QHostAddress(ipcAddr.newipaddr).toIPv4Address();
        snprintf(ipc.ipaddr, sizeof(ipc.ipaddr), "%s", ipcAddr.newipaddr);
        snprintf(ipc.netmask, sizeof(ipc.netmask), "%s", ipcAddr.netmask);
        snprintf(ipc.gateway, sizeof(ipc.gateway), "%s", ipcAddr.gateway);
        snprintf(ipc.dns, sizeof(ipc.dns), "%s", ipcAddr.primarydns);
        ipc.port = ipcAddr.port;

        showCameraList();
    }
}

void DeviceSearch::showCameraList()
{
    qDebug() << QString("DeviceSearch::showCameraList, size: %1").arg(m_cameraMap.size());

    const device_info &device = qMsNvr->deviceInfo();
    const bool &isPoe = qMsNvr->isPoe();
    int currenptlValue = ui->comboBox_protocol->currentData().toInt();
    const QString &currenLanName = ui->comboBox_nic->currentText();

    m_checkedRowList.clear();
    //
    m_existIpMap.clear();
    camera cam_array[MAX_CAMERA];
    int camCount = 0;
    read_cameras(SQLITE_FILE_NAME, cam_array, &camCount);
    for (int i = 0; i < camCount; ++i) {
        const camera &cam = cam_array[i];
        if (cam.enable) {
            m_existIpMap.insert(cam.ip_addr, 0);
        }
    }

    //
    ui->tableView->clearContent();
    ui->tableView->setRowCount(m_cameraMap.size());
    int row = 0;
    for (auto iter = m_cameraMap.constBegin(); iter != m_cameraMap.constEnd(); ++iter) {
        const resq_search_ipc &ipc = iter.value();
        //
        QString strNIC;
        if (device.max_lan > 1 && m_dbNetwork.mode == NETMODE_MULTI) {
            if (QString(ipc.netif_from) == "eth0") {
                strNIC = isPoe ? GET_TEXT("CAMERASEARCH/32031", "LAN") : GET_TEXT("CAMERASEARCH/32022", "LAN1");
            } else if (QString(ipc.netif_from) == "eth1") {
                strNIC = isPoe ? GET_TEXT("CAMERASEARCH/32038", "PoE") : GET_TEXT("CAMERASEARCH/32023", "LAN2");
            }
            if (currenLanName != GET_TEXT("COMMON/1006", "All") && currenLanName != strNIC) {
                continue;
            }
            if (currenptlValue != -1 && currenptlValue != ipc.protocol) {
                continue;
            }
        } else {
            if (device.max_lan == 1) {
                strNIC = "eth0";
            } else if (m_dbNetwork.mode != NETMODE_MULTI) {
                strNIC = "Bond0";
            }
            if (currenptlValue != -1 && currenptlValue != ipc.protocol) {
                continue;
            }
        }
        //qDebug()<<"[david debug] max_lan:"<<device.max_lan<<"mode:"<<m_dbNetwork.mode<<"strNIC:"<<strNIC;
        //
        ui->tableView->setItemChecked(row, false);
        ui->tableView->setItemData(row, ColumnCheck, false, ItemCheckedRole);
        ui->tableView->setItemIntValue(row, ColumnNumber, row + 1);
        ui->tableView->setItemText(row, ColumnIP, ipc.ipaddr);
        ui->tableView->setItemData(row, ColumnEdit, QVariant::fromValue(ipc), SearchCameraInfoRole);
        ui->tableView->setItemIntValue(row, ColumnPort, ipc.port);
        if (!strcmp(m_protocolMap.value(ipc.protocol).pro_name, "ONVIF")) {
            ui->tableView->setItemText(row, ColumnStatus, "-");
        } else {
            if (ipc.active_status) {
                ui->tableView->setItemText(row, ColumnStatus, GET_TEXT("DISKMANAGE/72092", "Active"));
            } else {
                ui->tableView->setItemText(row, ColumnStatus, GET_TEXT("CAMERASEARCH/32043", "Inactive"));
            }
        }
        ui->tableView->setItemText(row, ColumnProtocol, m_protocolMap.value(ipc.protocol).pro_name);
        ui->tableView->setItemText(row, ColumnNIC, strNIC);
        ui->tableView->setItemText(row, ColumnMAC, ipc.mac);
        ui->tableView->setItemText(row, ColumnFirmware, ipc.fwversion);
        ui->tableView->setItemText(row, ColumnModel, ipc.model);
        ui->tableView->setItemText(row, ColumnVendor, ipc.manufacturer);
        QString realsn(ipc.real_sn);
        if (realsn.isEmpty()) {
            realsn = "-";
        }
        ui->tableView->setItemText(row, ColumnSN, realsn);
        QString tipText = realsn;
        int p = 64;
        while(p < QString(realsn).size()){
            tipText.insert(p, "\n");
            p+=64;
        }
        if (tipText != "-") {
            ui->tableView->setItemToolTip(row, ColumnSN, tipText);
        }

        if (m_existIpMap.contains(QString(ipc.ipaddr))) {
            ui->tableView->setRowColor(row, QColor(0, 162, 232));
        }
        if (m_cameraMapRepeatIp.contains(QString(ipc.ipaddr))) {
            ui->tableView->setRowColor(row, QColor(255, 0, 0));
        }

        row++;
    }
    ui->tableView->removeRows(row, m_cameraMap.size() - row);
    ui->tableView->reorder();
}

void DeviceSearch::onLanguageChanged()
{
    ui->label_protocol->setText(GET_TEXT("CHANNELMANAGE/30014", "Protocol"));
    ui->label_nic->setText(GET_TEXT("CAMERASEARCH/32033", "Select NIC"));
    ui->label_range->setText(GET_TEXT("CAMERASEARCH/32032", "IP Range"));

    ui->pushButton_search->setText(GET_TEXT("CAMERASEARCH/32001", "Search"));
    ui->pushButton_activate->setText(GET_TEXT("CAMERASEARCH/32008", "Activate"));
    ui->pushButton_ipEdit->setText(GET_TEXT("CAMERASEARCH/32003", "IP Edit"));
    ui->pushButton_add->setText(GET_TEXT("COMMON/1000", "Add"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
}

void DeviceSearch::onTableItemClicked(int row, int column)
{
    switch (column) {
    case ColumnNumber:
    case ColumnIP:
    case ColumnPort:
    case ColumnProtocol:
    case ColumnNIC:
    case ColumnMAC:
    case ColumnFirmware:
    case ColumnModel:
    case ColumnVendor: {
        bool checked = ui->tableView->isItemChecked(row);
        ui->tableView->setItemChecked(row, !checked);
        if (!checked) {
            m_checkedRowList.append(row);
        } else {
            m_checkedRowList.removeAll(row);
        }
        break;
    }
    case ColumnCheck: {
        bool checked = ui->tableView->isItemChecked(row);
        if (checked) {
            m_checkedRowList.append(row);
        } else {
            m_checkedRowList.removeAll(row);
        }
        break;
    }
    case ColumnEdit: {
        const resq_search_ipc &ipc = ui->tableView->itemData(row, column, SearchCameraInfoRole).value<resq_search_ipc>();
        if (ipc.protocol != IPC_PROTOCOL_ONVIF && !ipc.active_status) {
            ShowMessageBox(GET_TEXT("CAMERASEARCH/32006", "Camera is not activated, please activate the camera first."));
            break;
        }

        struct req_set_ipcaddr tmpaddr;
        memset(&tmpaddr, 0, sizeof(tmpaddr));
        snprintf(tmpaddr.mac, sizeof(tmpaddr.mac), "%s", ipc.mac);
        snprintf(tmpaddr.oldipaddr, sizeof(tmpaddr.oldipaddr), "%s", ipc.ipaddr);
        snprintf(tmpaddr.username, sizeof(tmpaddr.username), "%s", "admin");
        snprintf(tmpaddr.netmask, sizeof(tmpaddr.netmask), "%s", ipc.netmask);
        snprintf(tmpaddr.gateway, sizeof(tmpaddr.gateway), "%s", ipc.gateway);
        snprintf(tmpaddr.primarydns, sizeof(tmpaddr.primarydns), "%s", ipc.dns);
        tmpaddr.protocol_id = ipc.protocol;
        snprintf(tmpaddr.netif_form, sizeof(tmpaddr.netif_form), "%s", ipc.netif_from);
        tmpaddr.port = ipc.port;
        if (!m_searchCameraEdit) {
            m_searchCameraEdit = new SearchCameraEdit(this);
        }
        m_searchCameraEdit->showEdit(tmpaddr);
        const int result = m_searchCameraEdit->exec();
        if (result == SearchCameraEdit::Accepted) {
            const req_set_ipcaddr &tmpaddr = m_searchCameraEdit->currentIpcaddrInfo();
            updateCameraMap(tmpaddr);
        }
        break;
    }
    default:
        break;
    }
}

void DeviceSearch::onTableHeaderChecked(bool checked)
{
    qDebug() << "[david debug] onTableHeaderChecked checked:" << checked << " row:" << ui->tableView->rowCount();
    m_checkedRowList.clear();
    if (checked) {
        for (int i = 0; i < ui->tableView->rowCount(); i++) {
            m_checkedRowList.append(i);
        }
    }

    return;
}

void DeviceSearch::onTablePoeItemClicked(int row, int column)
{
    switch (column) {
    case PoeColumnEdit: {
        int channel = ui->tableView_poe->itemIntValue(row, PoeColumnChannel) - 1;
        if (!m_cameraEdit) {
            m_cameraEdit = new CameraEdit(this);
        }
        m_cameraEdit->showEdit(channel);
        const int result = m_cameraEdit->exec();
        if (result == CameraEdit::Accepted) {
            //MsWaitting::showGlobalWait();
            sendMessage(REQUEST_FLAG_GET_IPCLIST, (void *)NULL, 0);
        }
        break;
    }
    default:
        break;
    }
}

void DeviceSearch::on_pushButton_search_clicked()
{
    bool valid = ui->lineEdit_ip_max->checkValid();
    valid = ui->lineEdit_ip_min->checkValid() && valid;
    if (!valid) {
        return;
    }
    QString strIP1 = ui->lineEdit_ip_min->text();
    QString strIP2 = ui->lineEdit_ip_max->text();
    if (QHostAddress(strIP1).toIPv4Address() > QHostAddress(strIP2).toIPv4Address()) {
        ShowMessageBox(GET_TEXT("CAMERASEARCH/32037", "Incorrect IP Format"));
        return;
    }

    m_cameraMap.clear();
    m_cameraMapAllIp.clear();
    m_cameraMapRepeatIp.clear();
    m_searchProtocolList.clear();

    int pro_id = ui->comboBox_protocol->currentData().toInt();
    if (pro_id != -1) {
        m_searchProtocolList.append(pro_id);
    } else {
        //all
        for (int i = 1; i < ui->comboBox_protocol->count(); ++i) {
            pro_id = ui->comboBox_protocol->itemData(i).toInt();
            m_searchProtocolList.append(pro_id);
        }
    }
    if (!m_searchProtocolList.isEmpty()) {
        int id = m_searchProtocolList.takeFirst();
        Q_UNUSED(id)
    }
}

void DeviceSearch::on_comboBox_protocol_activated(int index)
{
    Q_UNUSED(index)

    showCameraList();
}

void DeviceSearch::on_comboBox_nic_activated(int index)
{
    Q_UNUSED(index)

    showCameraList();
}

void DeviceSearch::on_pushButton_add_clicked()
{
    //
    QList<resq_search_ipc> addList;
    for (int i = 0; i < ui->tableView->rowCount(); ++i) {
        if (ui->tableView->isItemChecked(i)) {
            const resq_search_ipc &ipc = ui->tableView->itemData(i, ColumnEdit, SearchCameraInfoRole).value<resq_search_ipc>();
            if (ipc.protocol != IPC_PROTOCOL_ONVIF && !ipc.active_status) {
                ShowMessageBox(GET_TEXT("CAMERASEARCH/32006", "Camera is not activated, please activate the camera first."));
                return;
            }
            addList.append(ipc);
        }
    }
    if (addList.isEmpty()) {
        ShowMessageBox(GET_TEXT("CHANNELMANAGE/30034", "Please select at least one camera."));
        return;
    }
    //
    QList<int> unusedChannels = qMsNvr->disabledCameraList();
    if (addList.size() > unusedChannels.size()) {
        ShowMessageBox(GET_TEXT("CAMERASEARCH/32029", "No extra channel. Channel available: %1; Camera amount: %2").arg(unusedChannels.size()).arg(addList.size()));
        return;
    }
    //
    if (addList.size() == 1) {
        const resq_search_ipc &ipc = addList.first();
        if (!m_searchCameraAdd) {
            m_searchCameraAdd = new SearchCameraAdd(this);
        }
        m_searchCameraAdd->showAdd(&ipc);
        int result = m_searchCameraAdd->exec();
        if (result == SearchCameraAdd::Rejected) {
            return;
        }
    } else {
        if (!m_searchCameraAddMulti) {
            m_searchCameraAddMulti = new SearchCameraAddMulti(this);
        }
        m_searchCameraAddMulti->setAddList(addList);
        int result = m_searchCameraAddMulti->exec();
        if (result == SearchCameraAddMulti::Rejected) {
            return;
        }
    }
    //
    on_pushButton_search_clicked();
}

void DeviceSearch::on_pushButton_back_clicked()
{
    emit sig_back();
}

void DeviceSearch::on_pushButton_activate_clicked()
{
    int i = 0, row = 0;
    if (m_checkedRowList.isEmpty()) {
        ShowMessageBox(GET_TEXT("CAMERAMAINTENANCE/152003", "Please select at least one device."));
        return;
    }
    if (m_checkedRowList.size() > 64) {
        ShowMessageBox(GET_TEXT("CAMERASEARCH/32035", "Only supports %1 devices at the most").arg(64));
        return;
    }
    for (i = 0; i != m_checkedRowList.size(); ++i) {
        row = m_checkedRowList.at(i);
        const resq_search_ipc &ipc = ui->tableView->itemData(row, ColumnEdit, SearchCameraInfoRole).value<resq_search_ipc>();
        if (ipc.protocol == IPC_PROTOCOL_ONVIF) {
            ShowMessageBox(GET_TEXT("CAMERASEARCH/32005", "Cannot activate the selected device which searched by ONVIF protocol."));
            return;
        }
        if (ipc.active_status) {
            ShowMessageBox(GET_TEXT("CAMERASEARCH/32041", "The choosing camera has been activated, cannot be activated twice."));
            return;
        }
    }

    if (!m_searchCameraActivate) {
        m_searchCameraActivate = new SearchCameraActivate(this);
        connect(m_searchCameraActivate, SIGNAL(sigCameraActive(QString)), this, SLOT(onActiveCameraChanged(QString)));
    }
    if (m_searchCameraActivate) {
        m_searchCameraActivate->onLanguageChanged();
        m_searchCameraActivate->show();
    }
}

void DeviceSearch::makeMacformatInfo(char *inMac, int inlen, char *outMac)
{
    int n = 0;
    if (inMac[0] == '\0' || inlen != 12) {
        qDebug() << "[david debug] mac: " << inMac << " inlen:" << inlen;
        return;
    }
    for (int i = 0; i < inlen; i++, n++) {
        outMac[n] = inMac[i];
        if (n > 0 && ((i + 1) % 2 == 0) && i < 11) {
            n++;
            outMac[n] = ':';
        }
    }
    outMac[n] = '\0';

    qDebug() << "[david debug] inMac:" << inMac << " outMac:" << outMac;
    return;
}

void DeviceSearch::onActiveCameraChanged(QString password)
{
    active_camera_info req;
    memset(&req, 0x0, sizeof(struct active_camera_info));
    req.type = ACTIVATE;
    req.camera_list = m_checkedRowList.size();
    snprintf(req.password, sizeof(req.password), "%s", password.toStdString().c_str());
    for (int i = 0, row = 0; i != m_checkedRowList.size(); ++i) {
        row = m_checkedRowList.at(i);
        const resq_search_ipc &ipc = ui->tableView->itemData(row, ColumnEdit, SearchCameraInfoRole).value<resq_search_ipc>();
        makeMacformatInfo((char *)ipc.mac, (int)strlen(ipc.mac), (char *)&req.mac[i]);
    }

    sendMessage(REQUEST_FLAG_GET_CAMERA_CHALLENGE, (void *)&req, sizeof(active_camera_info));
}

void DeviceSearch::on_pushButton_ipEdit_clicked()
{
    bool hasInactive = false;
    int checkedCount = 0;
    QList<req_set_ipcaddr_batch> listSetIpcAddr;
    for (int row = 0; row < ui->tableView->rowCount(); ++row) {
        bool checked = ui->tableView->isItemChecked(row);
        if (checked) {
            checkedCount++;
            //
            const resq_search_ipc &ipc = ui->tableView->itemData(row, ColumnEdit, SearchCameraInfoRole).value<resq_search_ipc>();
            if (ipc.protocol != IPC_PROTOCOL_ONVIF && !ipc.active_status) {
                hasInactive = true;
                break;
            }
            //
            struct req_set_ipcaddr_batch tmpaddr;
            memset(&tmpaddr, 0, sizeof(tmpaddr));
            snprintf(tmpaddr.mac, sizeof(tmpaddr.mac), "%s", ipc.mac);
            snprintf(tmpaddr.oldipaddr, sizeof(tmpaddr.oldipaddr), "%s", ipc.ipaddr);
            snprintf(tmpaddr.username, sizeof(tmpaddr.username), "%s", "admin");
            snprintf(tmpaddr.netmask, sizeof(tmpaddr.netmask), "%s", ipc.netmask);
            snprintf(tmpaddr.gateway, sizeof(tmpaddr.gateway), "%s", ipc.gateway);
            snprintf(tmpaddr.primarydns, sizeof(tmpaddr.primarydns), "%s", ipc.dns);
            tmpaddr.protocol_id = ipc.protocol;
            snprintf(tmpaddr.netif_form, sizeof(tmpaddr.netif_form), "%s", ipc.netif_from);
            tmpaddr.port = ipc.port;

            listSetIpcAddr.append(tmpaddr);
        }
    }
    if (hasInactive) {
        ShowMessageBox(GET_TEXT("CAMERASEARCH/32006", "Camera is not activated, please activate the camera first."));
        return;
    }
    if (checkedCount == 0) {
        ShowMessageBox(GET_TEXT("CAMERAMAINTENANCE/152003", "Please select at least one device."));
        return;
    }
    if (checkedCount > 64) {
        ShowMessageBox(GET_TEXT("CAMERASEARCH/32035", "Only supports %1 devices at the most").arg(64));
        return;
    }
    //
    if (listSetIpcAddr.size() == 1) {
        if (!m_searchCameraEdit) {
            m_searchCameraEdit = new SearchCameraEdit(this);
        }
        m_searchCameraEdit->showEdit(listSetIpcAddr.first());
        const int result = m_searchCameraEdit->exec();
        if (result == SearchCameraEdit::Accepted) {
            const req_set_ipcaddr &tmpaddr = m_searchCameraEdit->currentIpcaddrInfo();
            updateCameraMap(tmpaddr);
        }
    } else {
        //
        if (!m_searchCameraEditMulti) {
            m_searchCameraEditMulti = new SearchCameraEditMulti(this);
        }
        const int result = m_searchCameraEditMulti->execEdit(listSetIpcAddr);
        if (result == SearchCameraEditMulti::Accepted) {
            on_pushButton_search_clicked();
        }
    }
}

void DeviceSearch::ON_RESPONSE_FLAG_GET_CAMERA_CHALLENGE(MessageReceive *message)
{
    if (message->data) {
        int ret = *(int *)message->data;
        qDebug() << "[david debug] ret:" << ret;
    }
}
