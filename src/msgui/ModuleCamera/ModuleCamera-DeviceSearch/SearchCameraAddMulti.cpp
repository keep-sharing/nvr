#include "SearchCameraAddMulti.h"
#include "ui_SearchCameraAddMulti.h"
#include "LogWrite.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "msuser.h"
#include "MsWaitting.h"
#include "myqt.h"
#include <QFile>

extern "C" {
#include "msg.h"
}

SearchCameraAddMulti::SearchCameraAddMulti(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::SearchCameraAddMulti)
{
    ui->setupUi(this);

    QFile file(":/style/style/settingstyle.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(file.readAll());
    }
    file.close();

    //
    setTitleWidget(ui->label_title);

    ui->comboBox_transportProtocol->clear();
    ui->comboBox_transportProtocol->addItem("Auto", TRANSPROTOCOL_AUTO);
    ui->comboBox_transportProtocol->addItem("UDP", TRANSPROTOCOL_UDP);
    ui->comboBox_transportProtocol->addItem("TCP", TRANSPROTOCOL_TCP);

    //table
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("COMMON/1033", "IP Address");
    headerList << GET_TEXT("CHANNELMANAGE/30027", "MAC");
    headerList << GET_TEXT("PLAYBACK/80113", "Result");
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setColumnCount(headerList.size());
    //sort
    ui->tableView->setSortingEnabled(false);
    //
    ui->tableView->hideColumn(ColumnChecked);
    ui->tableView->setColumnWidth(ColumnIP, 150);
    ui->tableView->setColumnWidth(ColumnMAC, 150);

    ui->lineEdit_userName->setCheckMode(MyLineEdit::UserNameCheck);

    //
    onLanguageChanged();
}

SearchCameraAddMulti::~SearchCameraAddMulti()
{
    delete ui;
}

void SearchCameraAddMulti::setAddList(const QList<resq_search_ipc> &list)
{
    m_addList = list;

    ui->lineEdit_userName->setEnabled(true);
    ui->lineEdit_userName->setText("admin");
    ui->lineEdit_password->setEnabled(true);
    ui->lineEdit_password->clear();
    ui->comboBox_transportProtocol->setEnabled(true);
    ui->pushButton_close->setEnabled(true);
    ui->pushButton_close->setVisible(false);
    ui->pushButton_ok->setEnabled(true);
    ui->pushButton_ok->setVisible(true);
    ui->pushButton_cancel->setEnabled(true);
    ui->pushButton_cancel->setVisible(true);

    ui->tableView->clearContent();
    ui->tableView->setRowCount(list.size());
    for (int i = 0; i < m_addList.size(); ++i) {
        const resq_search_ipc &ipc = m_addList.at(i);
        ui->tableView->setItemText(i, ColumnIP, QString(ipc.ipaddr));
        ui->tableView->setItemText(i, ColumnMAC, QString(ipc.mac));
    }

    int defaultTransport = get_param_int(SQLITE_FILE_NAME, PARAM_TRANSPORT_PROTOCOL, 0);
    ui->comboBox_transportProtocol->setCurrentIndexFromData(defaultTransport);
}

void SearchCameraAddMulti::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_CHECK_IPC_FISHEYE_INFO:
        ON_RESPONSE_FLAG_CHECK_IPC_FISHEYE_INFO(message);
        break;
    }
}

void SearchCameraAddMulti::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("CAMERASEARCH/32017", "Batch Add"));
    ui->label_username->setText(GET_TEXT("COMMON/1007", "User Name"));
    ui->label_password->setText(GET_TEXT("COMMON/1008", "Password"));
    ui->label_transportProtocol->setText(GET_TEXT("CHANNELMANAGE/30015", "Transport Protocol"));
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void SearchCameraAddMulti::on_pushButton_ok_clicked()
{
   
}

bool SearchCameraAddMulti::isInputValid()
{
    
#if 0
    QString strPassword = ui->lineEdit_password->text();
    if (strPassword.trimmed().isEmpty())
    {
        ShowMessageBox(GET_TEXT("CHANNELMANAGE/30050","Password cannot be empty."));
        ui->lineEdit_password->setFocus();
        return false;
    }
#endif
    return ui->lineEdit_userName->checkValid();;
}

int SearchCameraAddMulti::waitForTestIpcConnect(const resq_search_ipc *search_ipc)
{
    struct req_test_ipcconnect test_ipcconnect;
    qDebug() << "testIpcConnect multi ip:" << search_ipc->ipaddr;
    memset(&test_ipcconnect, 0, sizeof(test_ipcconnect));
    snprintf(test_ipcconnect.ip, sizeof(test_ipcconnect.ip), "%s", search_ipc->ipaddr);
    test_ipcconnect.port = search_ipc->port;
    snprintf(test_ipcconnect.user, sizeof(test_ipcconnect.user), "%s", ui->lineEdit_userName->text().toStdString().c_str());
    snprintf(test_ipcconnect.password, sizeof(test_ipcconnect.password), "%s", ui->lineEdit_password->text().toStdString().c_str());
    test_ipcconnect.protocol = search_ipc->protocol;

    sendMessage(REQUEST_FLAG_TEST_IPCCONNECT, (void *)&test_ipcconnect, sizeof(test_ipcconnect));
    return 1;
}

void SearchCameraAddMulti::ON_RESPONSE_FLAG_TEST_IPCCONNECT(MessageReceive *message)
{
    m_ipcTestResult = IPC_UNKNOWN_ERR;
    if (message->data) {
        m_ipcTestResult = static_cast<IPC_CONN_RES>(*((int *)message->data));
    }

    m_eventLoop.exit();
}

void SearchCameraAddMulti::ON_RESPONSE_FLAG_CHECK_IPC_FISHEYE_INFO(MessageReceive *message)
{
    memset(&m_fisheye_info, 0, sizeof(m_fisheye_info));
    memcpy(&m_fisheye_info, message->data, sizeof(m_fisheye_info));
    m_eventLoop.exit();
}

void SearchCameraAddMulti::addCamera(int channel, int stream_id, const resq_search_ipc &search_ipc)
{
    qMsDebug() << QString("channel: %1, channel_id: %2").arg(channel).arg(stream_id);
    const IPC_PROTOCOL &protocol = static_cast<IPC_PROTOCOL>(ui->comboBox_transportProtocol->currentData().toInt());

    //camera
    camera cameraInfo;
    memset(&cameraInfo, 0, sizeof(cameraInfo));
    read_camera(SQLITE_FILE_NAME, &cameraInfo, channel);

    cameraInfo.minorid = stream_id;
    cameraInfo.enable = true;
    cameraInfo.camera_protocol = protocol;
    cameraInfo.poe_channel = 0;
    memset(cameraInfo.mac_addr, 0, sizeof(cameraInfo.mac_addr));
    snprintf(cameraInfo.ip_addr, sizeof(cameraInfo.ip_addr), "%s", search_ipc.ipaddr);
    cameraInfo.manage_port = search_ipc.port;
    cameraInfo.sub_rtsp_enable = true;
    snprintf(cameraInfo.username, sizeof(cameraInfo.username), "%s", ui->lineEdit_userName->text().toStdString().c_str());
    snprintf(cameraInfo.password, sizeof(cameraInfo.password), "%s", ui->lineEdit_password->text().toStdString().c_str());
    cameraInfo.transmit_protocol = ui->comboBox_transportProtocol->currentData().toInt();
    qMsNvr->writeDatabaseCamera(&cameraInfo);

    //
    sendMessageOnly(REQUEST_FLAG_ADDNEW_IPC, &channel, sizeof(channel));

    //log
    struct log_data log_data;
    struct op_lr_a_d_e_ip_channel olic;
    memset(&log_data, 0, sizeof(struct log_data));
    memset(&olic, 0, sizeof(struct op_lr_a_d_e_ip_channel));
    log_data.log_data_info.subType = SUP_OP_ADD_IP_CHANNEL_LOCK;
    log_data.log_data_info.parameter_type = SUB_PARAM_ADD_IPC;
    snprintf(log_data.log_data_info.user, sizeof(log_data.log_data_info.user), "%s", gMsUser.userName().toStdString().c_str());
    log_data.log_data_info.chan_no = channel + 1;

    snprintf(olic.ip, sizeof(olic.ip), "%s", cameraInfo.ip_addr);
    olic.action = SUB_PARAM_ADD_IPC;
    olic.port = cameraInfo.manage_port;
    olic.channel = channel + 1;
    olic.timeset = cameraInfo.sync_time;
    if (cameraInfo.camera_protocol == IPC_PROTOCOL_ONVIF)
        snprintf(olic.protocal, sizeof(olic.protocal), "%s", "ONVIF");
    else if (cameraInfo.camera_protocol == IPC_PROTOCOL_RTSP)
        snprintf(olic.protocal, sizeof(olic.protocal), "%s", "RTSP");
    else
        snprintf(olic.protocal, sizeof(olic.protocal), "%s", "MSSF");
    if (cameraInfo.transmit_protocol == TRANSPROTOCOL_UDP)
        snprintf(olic.tran_protocal, sizeof(olic.tran_protocal), "%s", "UDP");
    else if (cameraInfo.transmit_protocol == TRANSPROTOCOL_TCP)
        snprintf(olic.tran_protocal, sizeof(olic.tran_protocal), "%s", "TCP");
    else
        snprintf(olic.tran_protocal, sizeof(olic.tran_protocal), "%s", "Auto");
    msfs_log_pack_detail(&log_data, OP_A_D_E_IP_CHANNEL, &olic, sizeof(olic));
    MsWriteLog(log_data);
}

void SearchCameraAddMulti::on_pushButton_close_clicked()
{
    accept();
}

void SearchCameraAddMulti::on_pushButton_cancel_clicked()
{
    reject();
}
