#include "SearchCameraAdd.h"
#include "ui_SearchCameraAdd.h"
#include "LogWrite.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "camerachanneladd.h"
#include "centralmessage.h"
#include "myqt.h"
#include <QFile>

extern "C" {
#include "msg.h"
}

SearchCameraAdd::SearchCameraAdd(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::SearchCameraAdd)
{
    ui->setupUi(this);

    QFile file(":/style/style/settingstyle.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(file.readAll());
    }
    file.close();

    //
    ui->comboBox_protocol->clear();
    QMap<int, ipc_protocol> protocolMap = qMsNvr->protocolMap();
    for (auto iter = protocolMap.constBegin(); iter != protocolMap.constEnd(); ++iter) {
        const ipc_protocol &protocol = iter.value();
        ui->comboBox_protocol->addItem(protocol.pro_name, protocol.pro_id);
    }
    if (!MsDevice::instance()->isMilesight()) {
        ui->comboBox_protocol->removeItemFromData(IPC_PROTOCOL_MSDOMAIN);
    }
    ui->comboBox_protocol->setCurrentIndex(0);

    ui->comboBox_transportProtocol->clear();
    ui->comboBox_transportProtocol->addItem("Auto", TRANSPROTOCOL_AUTO);
    ui->comboBox_transportProtocol->addItem("UDP", TRANSPROTOCOL_UDP);
    ui->comboBox_transportProtocol->addItem("TCP", TRANSPROTOCOL_TCP);

    ui->lineEdit_channelName->setCheckMode(MyLineEdit::EmptyCheck);
    ui->lineEdit_ip->setCheckMode(MyLineEdit::ServerCheck);
    ui->lineEdit_port->setCheckMode(MyLineEdit::RangeCheck, 1, 65535);
    ui->lineEditHTTPSPort->setCheckMode(MyLineEdit::RangeCheck, 1, 65535);
    ui->lineEdit_userName->setCheckMode(MyLineEdit::UserNameCheck);

    onLanguageChanged();
}

SearchCameraAdd::~SearchCameraAdd()
{
    delete ui;
}

void SearchCameraAdd::showAdd(const resq_search_ipc *search_ipc)
{
    memset(&m_sourceIpcInfo, 0, sizeof(resq_search_ipc));
    memcpy(&m_sourceIpcInfo, search_ipc, sizeof(resq_search_ipc));
    //protocol
    m_msddns = "ddns.milesight.com/" + QString(search_ipc->mac);
    m_ipAddress = "";
    m_perProtocol = IPC_PROTOCOL_ONVIF;

    //mac
    ui->lineEdit_mac->setText(search_ipc->mac);
    //channel
    ui->comboBox_channel->clear();
    const QList<int> &channelList = qMsNvr->disabledCameraList();
    for (int i = 0; i < channelList.size(); ++i) {
        const int &channel = channelList.at(i);
        ui->comboBox_channel->addItem(QString::number(channel + 1), channel);
    }
    if (channelList.size() > 0) {
        ui->comboBox_channel->setCurrentIndex(0);
        on_comboBox_channel_activated(0);
    }
    //protocol
    ui->comboBox_protocol->setCurrentIndexFromData(search_ipc->protocol);
    on_comboBox_protocol_activated(ui->comboBox_protocol->currentIndex());
    //ip
    ui->lineEdit_ip->setText(search_ipc->ipaddr);
    //port
    ui->lineEdit_port->setText(QString::number(search_ipc->port));
    ui->lineEditHTTPSPort->setText(QString::number(443));
    //
    ui->lineEdit_primary->setText(QString("rtsp://%1").arg(search_ipc->ipaddr));
    m_primary = QString("rtsp://%1").arg(search_ipc->ipaddr);
    m_secondary = "rtsp://";
    //
    int defaultTransport = get_param_int(SQLITE_FILE_NAME, PARAM_TRANSPORT_PROTOCOL, 0);
    ui->comboBox_transportProtocol->setCurrentIndexFromData(defaultTransport);
    on_comboBox_transportProtocol_activated(defaultTransport);
    ui->checkBox_timeSetting->setChecked(false);
    ui->lineEdit_password->clear();
    ui->lineEdit_userName->setText("admin");
}

void SearchCameraAdd::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_TEST_IPCCONNECT:
        ON_RESPONSE_FLAG_TEST_IPCCONNECT(message);
        message->accept();
        break;
    case RESPONSE_FLAG_CHECK_IPC_FISHEYE_INFO:
        ON_RESPONSE_FLAG_CHECK_IPC_FISHEYE_INFO(message);
        message->accept();
        break;
    }
}

void SearchCameraAdd::ON_RESPONSE_FLAG_TEST_IPCCONNECT(MessageReceive *message)
{
    if (message->data) {
        QString strMessage;
        m_ipcTestResult = static_cast<IPC_CONN_RES>(*((int *)message->data));
        switch (m_ipcTestResult) {
        case IPC_OK:
            return;
        case IPC_NETWORK_ERR:
            strMessage = GET_TEXT("CHANNELMANAGE/30040", "Network Error.");
            break;
        case IPC_INVALID_USER:
            strMessage = GET_TEXT("CHANNELMANAGE/30041", "User Name or Password Error.");
            break;
        case IPC_UNKNOWN_ERR:
            strMessage = GET_TEXT("CHANNELMANAGE/30042", "Unknown Error.");
            break;
        case IPC_PROTO_NOT_SUPPORT:
            strMessage = GET_TEXT("CHANNELMANAGE/30043", "Protocol not support.");
            break;
        case IPC_OUT_LIMIT_BANDWIDTH:
            strMessage = "Bandwidth reaches the limit.";
            break;
        default:
            strMessage = GET_TEXT("CHANNELMANAGE/30042", "Unknown Error.");
            break;
        }

        ShowMessageBox(strMessage);
    }
}

void SearchCameraAdd::ON_RESPONSE_FLAG_CHECK_IPC_FISHEYE_INFO(MessageReceive *message)
{
    memset(&m_fisheye_info, 0, sizeof(m_fisheye_info));
    memcpy(&m_fisheye_info, message->data, sizeof(m_fisheye_info));
    m_eventLoop.exit();
}

bool SearchCameraAdd::isInputValid()
{
    bool valid = true;
    valid &= ui->lineEdit_channelName->checkValid();
    switch (ui->comboBox_protocol->currentIntData()) {
    case IPC_PROTOCOL_RTSP:
        valid &= ui->lineEdit_primary->checkValid();
        if (!ui->lineEdit_secondary->isEmpty()) {
            valid &= ui->lineEdit_secondary->checkValid();
        }
        break;
    case IPC_PROTOCOL_MSDOMAIN: {
        QRegExp rx("(^ddns\\.milesight\\.com\\/)([0-9a-fA-F]{12})");
        if (rx.indexIn(ui->lineEdit_ip->text()) == -1) {
            ui->lineEdit_ip->setCustomValid(false, GET_TEXT("MYLINETIP/112001", "Invalid."));
            valid = false;
        }
        break;
    }
    default:
        valid &= ui->lineEdit_ip->checkValid();
        valid &= ui->lineEdit_port->checkValid();
        break;
    }
    valid &= ui->lineEdit_userName->checkValid();
    if (ui->comboBox_transportProtocol->currentData() == TRANSPROTOCOL_ROH) {
        valid &= ui->lineEditHTTPSPort->checkValid();
    }
    if (!valid) {
        return false;
    }
    return true;
}

void SearchCameraAdd::testIpcConnect()
{
    struct req_test_ipcconnect test_ipcconnect;
    memset(&test_ipcconnect, 0, sizeof(test_ipcconnect));
    snprintf(test_ipcconnect.ip, sizeof(test_ipcconnect.ip), "%s", ui->lineEdit_ip->text().toStdString().c_str());
    test_ipcconnect.port = ui->lineEdit_port->text().toInt();
    snprintf(test_ipcconnect.user, sizeof(test_ipcconnect.user), "%s", ui->lineEdit_userName->text().toStdString().c_str());
    snprintf(test_ipcconnect.password, sizeof(test_ipcconnect.password), "%s", ui->lineEdit_password->text().toStdString().c_str());
    test_ipcconnect.protocol = ui->comboBox_protocol->currentData().toInt();
    test_ipcconnect.transportProtocol = ui->comboBox_transportProtocol->currentIntData();
    //test_ipcconnect.model = ui->comboBox_model->itemData(ui->comboBox_model->currentIndex()).value<int>();
    sendMessage(REQUEST_FLAG_TEST_IPCCONNECT, (void *)&test_ipcconnect, sizeof(test_ipcconnect));
}

void SearchCameraAdd::lineEditIPShow(bool enable)
{
    if (enable) {
        ui->label_primary->hide();
        ui->label_secondary->hide();
        ui->lineEdit_primary->hide();
        ui->lineEdit_secondary->hide();

        ui->label_ip->show();
        ui->label_port->show();
        ui->lineEdit_ip->show();
        ui->lineEdit_port->show();

    } else {
        ui->label_ip->hide();
        ui->label_port->hide();
        ui->lineEdit_ip->hide();
        ui->lineEdit_port->hide();

        ui->label_primary->show();
        ui->label_secondary->show();
        ui->lineEdit_primary->show();
        ui->lineEdit_secondary->show();
    }
}

void SearchCameraAdd::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("CHANNELMANAGE/154001", "Camera Add"));
    ui->label_mac->setText(GET_TEXT("CHANNELMANAGE/30027", "MAC"));
    ui->label_channel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));
    ui->label_channelName->setText(GET_TEXT("CHANNELMANAGE/30009", "Channel Name"));
    ui->label_protocol->setText(GET_TEXT("CHANNELMANAGE/30014", "Protocol"));
    ui->label_ip->setText(GET_TEXT("COMMON/1033", "IP Address"));
    ui->label_port->setText(GET_TEXT("CHANNELMANAGE/30011", "Port"));
    ui->label_transportProtocol->setText(GET_TEXT("CHANNELMANAGE/30015", "Transport Protocol"));
    ui->label_userName->setText(GET_TEXT("COMMON/1007", "User Name"));
    ui->label_password->setText(GET_TEXT("COMMON/1008", "Password"));
    ui->label_timeSetting->setText(GET_TEXT("CHANNELMANAGE/30018", "Time Setting"));
    ui->checkBox_timeSetting->setText(GET_TEXT("CHANNELMANAGE/30019", "Sync Time With NVR"));
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
    ui->labelHTTPSPort->setText(GET_TEXT("SYSTEMNETWORK/71059", "HTTPS Port"));
}

void SearchCameraAdd::on_comboBox_protocol_activated(int index)
{
    const IPC_PROTOCOL &protocol = static_cast<IPC_PROTOCOL>(ui->comboBox_protocol->itemData(index).toInt());
    if (m_perProtocol == IPC_PROTOCOL_ONVIF || m_perProtocol == IPC_PROTOCOL_MILESIGHT) {
        m_ipAddress = ui->lineEdit_ip->text();
    }
    int transportProtocolIndex = ui->comboBox_transportProtocol->currentIndex();
    ui->widgetSpacer->hide();
    ui->comboBox_transportProtocol->removeItemFromData(TRANSPROTOCOL_ROH);
    switch (protocol) {
    case IPC_PROTOCOL_ONVIF:
        ui->label_ip->setText(GET_TEXT("COMMON/1033", "IP Address"));
        ui->lineEdit_ip->setText(m_ipAddress);
        lineEditIPShow(true);
        ui->comboBox_transportProtocol->removeItemFromData(TRANSPROTOCOL_ROH);
        break;
    case IPC_PROTOCOL_MILESIGHT:
        ui->label_ip->setText(GET_TEXT("COMMON/1033", "IP Address"));
        ui->lineEdit_ip->setText(m_ipAddress);
        lineEditIPShow(true);
        ui->comboBox_transportProtocol->addItem(GET_TEXT("CHANNELMANAGE/174000", "Encryption"), TRANSPROTOCOL_ROH);
        break;
    case IPC_PROTOCOL_RTSP:
        lineEditIPShow(false);
        ui->comboBox_transportProtocol->removeItemFromData(TRANSPROTOCOL_ROH);
        break;
    case IPC_PROTOCOL_MSDOMAIN:
        lineEditIPShow(true);
        ui->lineEdit_ip->setText(m_msddns);
        ui->label_ip->setText(GET_TEXT("CAMERA/143000", "Domain Address"));
        ui->label_port->hide();
        ui->lineEdit_port->hide();
        ui->widgetSpacer->show();
        ui->comboBox_transportProtocol->removeItemFromData(TRANSPROTOCOL_ROH);
        break;
    default:
        break;
    }
    if (transportProtocolIndex == TRANSPROTOCOL_ROH && ui->comboBox_transportProtocol->count() < TRANSPROTOCOL_MAX) {
        ui->comboBox_transportProtocol->setCurrentIndexFromData(TRANSPROTOCOL_AUTO);
    }
    on_comboBox_transportProtocol_activated(ui->comboBox_transportProtocol->currentIntData());
    m_perProtocol = protocol;
}

void SearchCameraAdd::on_comboBox_channel_activated(int index)
{
    Q_UNUSED(index)

    int channel = ui->comboBox_channel->currentData().toInt();
    ui->lineEdit_channelName->setText(qMsNvr->channelName(channel));
}

void SearchCameraAdd::on_pushButton_ok_clicked()
{

}

void SearchCameraAdd::on_pushButton_cancel_clicked()
{
    close();
}

void SearchCameraAdd::on_lineEdit_ip_textEdited(const QString &arg1)
{
    if (ui->comboBox_protocol->currentIntData() == IPC_PROTOCOL_MSDOMAIN) {
        QRegExp rx("(^ddns\\.milesight\\.com\\/)(.{0,12})");
        if (rx.indexIn(arg1) == -1) {
            ui->lineEdit_ip->setText(m_msddns);
            return;
        }
        QString strIP = rx.cap(2).toUpper();
        m_msddns = rx.cap(1) + strIP;
        ui->lineEdit_ip->setText(m_msddns);
    }
}

void SearchCameraAdd::on_lineEdit_primary_textEdited(const QString &arg1)
{
    if (ui->comboBox_protocol->currentIntData() == IPC_PROTOCOL_RTSP) {
        QRegExp rx("(^rtsp:\\/\\/)");
        if (rx.indexIn(arg1) == -1) {
            ui->lineEdit_primary->setText(m_primary);
            return;
        }
        m_primary = arg1;
    }
}

void SearchCameraAdd::on_lineEdit_secondary_textEdited(const QString &arg1)
{
    if (ui->comboBox_protocol->currentIntData() == IPC_PROTOCOL_RTSP) {
        QRegExp rx("(^rtsp:\\/\\/)");
        if (rx.indexIn(arg1) == -1) {
            ui->lineEdit_secondary->setText(m_secondary);
            return;
        }
        m_secondary = arg1;
    }
}

void SearchCameraAdd::on_comboBox_transportProtocol_activated(int index)
{
    Q_UNUSED(index)
    if (ui->comboBox_transportProtocol->currentIntData() == TRANSPROTOCOL_ROH) {
        ui->lineEditHTTPSPort->show();
        ui->labelHTTPSPort->show();
        ui->label_port->setText(GET_TEXT("SYSTEMNETWORK/71057", "HTTP Port"));
    } else {
        ui->lineEditHTTPSPort->hide();
        ui->labelHTTPSPort->hide();
        ui->label_port->setText(GET_TEXT("CHANNELMANAGE/30011", "Port"));
    }       
}