#include "NetworkPageMore.h"
#include "ui_NetworkPageMore.h"
#include "MessageBox.h"
#include "MsApplication.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "PushEventEdit.h"
#include "centralmessage.h"

#define MINPORT 1024
#define MAXPORT 65535

NetworkPageMore::NetworkPageMore(QWidget *parent)
    : AbstractNetworkPage(parent)
    , ui(new Ui::NetworkPageMore)
{
    ui->setupUi(this);

    initNetMoreTab();

    if (qMsNvr->isSlaveMode()) {
        ui->label_pushStream->hide();
        ui->comboBox_pushStream->hide();
        ui->label_pushType->hide();
        ui->pushButton_pushType->hide();
        ui->label_pushMsg->hide();
        ui->comboBox_pushMsg->hide();
    }

    ui->lineEdit_sshPort->setCheckMode(MyLineEdit::SpecialRangeCheck, MINPORT, MAXPORT, 22);
    ui->lineEdit_httpPort->setCheckMode(MyLineEdit::SpecialRangeCheck, MINPORT, MAXPORT, 80);
    ui->lineEdit_httpsPort->setCheckMode(MyLineEdit::SpecialRangeCheck, MINPORT, MAXPORT, 443);
    ui->lineEdit_rtspPort->setCheckMode(MyLineEdit::SpecialRangeCheck, MINPORT, MAXPORT, 554);

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

NetworkPageMore::~NetworkPageMore()
{
    freeNetMoreTab();
    delete ui;
}

void NetworkPageMore::initNetMoreTab()
{
    QRegExp rx(QString("[0-9]*"));
    QValidator *validator = new QRegExpValidator(rx, this);
    ui->lineEdit_sshPort->setValidator(validator);
    ui->lineEdit_httpPort->setValidator(validator);
    ui->lineEdit_httpsPort->setValidator(validator);
    ui->lineEdit_rtspPort->setValidator(validator);
    ui->lineEdit_sdkPort->setValidator(validator);
    ui->lineEdit_sdkPort->hide();
    ui->label_sdkport->hide();

    ui->comboBox_pushMsg->clear();
    ui->comboBox_pushMsg->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBox_pushMsg->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);

    ui->comboBox_ssh->clear();
    ui->comboBox_ssh->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBox_ssh->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);

    ui->comboBox_pushStream->clear();
    ui->comboBox_pushStream->addItem(GET_TEXT("SYSTEMNETWORK/71207", "Auto"), 0);
    ui->comboBox_pushStream->addItem(GET_TEXT("SYSTEMNETWORK/71208", "Primary Stream"), 1);
    ui->comboBox_pushStream->addItem(GET_TEXT("SYSTEMNETWORK/71209", "Secondary Stream"), 2);

    connect(ui->comboBox_ssh, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSshEnableChanged(int)));

    if (NetSnmpPort == nullptr) {
        NetSnmpPort = new snmp;
    }
    if (pEmailPort == nullptr) {
        pEmailPort = new struct email;
    }
    memset(NetSnmpPort, 0, sizeof(snmp));
    memset(pEmailPort, 0, sizeof(struct email));
}

void NetworkPageMore::gotoNetMoreTab()
{
    if (pNetMoreDb == NULL) {
        pNetMoreDb = new network_more;
    }
    if (pNetMoreDbOri == NULL) {
        pNetMoreDbOri = new network_more;
    }
    memset(pNetMoreDb, 0, sizeof(network_more));
    memset(pNetMoreDbOri, 0, sizeof(network_more));

    read_port(SQLITE_FILE_NAME, pNetMoreDb);
    memcpy(pNetMoreDbOri, pNetMoreDb, sizeof(network_more));

    read_snmp(SQLITE_FILE_NAME, NetSnmpPort);
    read_email(SQLITE_FILE_NAME, pEmailPort);

    //
    IPCPushType.clear();
    int chnCnt = qMsNvr->maxChannel();
    push_msg_event event[chnCnt];
    memset(&event, 0, chnCnt * sizeof(push_msg_event));
    read_push_msg_events(SQLITE_FILE_NAME, event, chnCnt);
    for (int i = 0; i < chnCnt; i++) {
        IPCPushType.insert(event[i].chnid, event[i].push_type);
    }

    NVRAlarmInType = get_param_int(SQLITE_FILE_NAME, PARAM_PUSH_ALARMIN_TYPE, 0);
    //
    readMoreConfig();
}

void NetworkPageMore::freeNetMoreTab()
{
    if (pNetMoreDb) {
        delete pNetMoreDb;
        pNetMoreDb = nullptr;
    }
    if (pNetMoreDbOri) {
        delete pNetMoreDbOri;
        pNetMoreDbOri = nullptr;
    }
    if (pNetMoreDbOther) {
        delete pNetMoreDbOther;
        pNetMoreDbOther = nullptr;
    }
    if (NetSnmpPort) {
        delete NetSnmpPort;
        NetSnmpPort = nullptr;
    }
    if (pEmailPort) {
        delete pEmailPort;
        pEmailPort = nullptr;
    }
}

void NetworkPageMore::readMoreConfig()
{
    char value[2];
    const char *key = "enable_push_msg";

    if (pNetMoreDb->enable_ssh) {
        ui->comboBox_ssh->setCurrentIndex(1);
        slotSshEnableChanged(1);
    } else {
        ui->comboBox_ssh->setCurrentIndex(0);
        slotSshEnableChanged(0);
    }
    if (pNetMoreDb->ssh_port != 0) {
        ui->lineEdit_sshPort->setText(QString("%1").arg(pNetMoreDb->ssh_port));
    }
    if (pNetMoreDb->http_port != 0) {
        ui->lineEdit_httpPort->setText(QString("%1").arg(pNetMoreDb->http_port));
    }
    if (pNetMoreDb->https_port != 0) {
        ui->lineEdit_httpsPort->setText(QString("%1").arg(pNetMoreDb->https_port));
    }
    if (pNetMoreDb->rtsp_port != 0) {
        ui->lineEdit_rtspPort->setText(QString("%1").arg(pNetMoreDb->rtsp_port));
    }

    get_param_value(SQLITE_FILE_NAME, key, value, sizeof(value), "0");
    if (atoi(value) == 1) {
        ui->comboBox_pushMsg->setCurrentIndex(1);
    } else {
        ui->comboBox_pushMsg->setCurrentIndex(0);
    }

    ui->comboBox_pushStream->setCurrentIndex(get_param_int(SQLITE_FILE_NAME, PARAM_PUSH_VIDEO_STREAM, 0));

    //
    memset(&m_pushMsgNvrEvtDb, 0, sizeof(m_pushMsgNvrEvtDb));
    read_push_msg_nvr_event(SQLITE_FILE_NAME, &m_pushMsgNvrEvtDb);
}

void NetworkPageMore::saveMoreConfig()
{
    if (ui->comboBox_ssh->currentIndex() == 1) {
        pNetMoreDb->enable_ssh = 1;
    } else {
        pNetMoreDb->enable_ssh = 0;
    }
    pNetMoreDb->http_port = ui->lineEdit_httpPort->text().toInt();
    if (!ui->lineEdit_httpPort->text().trimmed().isEmpty()) {
        pNetMoreDb->http_port = ui->lineEdit_httpPort->text().trimmed().toInt();
    }
    if (!ui->lineEdit_httpsPort->text().trimmed().isEmpty()) {
        pNetMoreDb->https_port = ui->lineEdit_httpsPort->text().trimmed().toInt();
    }
    if (!ui->lineEdit_rtspPort->text().trimmed().isEmpty()) {
        pNetMoreDb->rtsp_port = ui->lineEdit_rtspPort->text().trimmed().toInt();
    }
    if (!ui->lineEdit_sdkPort->text().trimmed().isEmpty()) {
        pNetMoreDb->sdk_port = ui->lineEdit_sdkPort->text().trimmed().toInt();
    }
}

bool NetworkPageMore::isValidatorMoreInput()
{
    bool valid = true;
    valid &= ui->lineEdit_httpPort->checkValid();
    valid &= ui->lineEdit_httpsPort->checkValid();
    valid &= ui->lineEdit_rtspPort->checkValid();
    if (ui->comboBox_ssh->currentIndex() == 1) {
        valid &= ui->lineEdit_sshPort->checkValid();
    }

    if (!valid) {
        return false;
    }

    QSet<int> ports;
    ports.insert(NetSnmpPort->port);
    ports.insert(pEmailPort->port);
    //
    if (ui->lineEdit_sshPort->isEnabled()) {
        int sshPort = ui->lineEdit_sshPort->text().trimmed().toInt();
        if (ports.contains(sshPort)) {
            ShowMessageBox(GET_TEXT("MYLINETIP/112011", "Already existed."));
            return false;
        }
        ports.insert(sshPort);
    }
    //
    int httpPort = ui->lineEdit_httpPort->text().trimmed().toInt();
    if (ports.contains(httpPort)) {
        ShowMessageBox(GET_TEXT("MYLINETIP/112011", "Already existed."));
        return false;
    }
    ports.insert(httpPort);
    //
    int httpsPort = ui->lineEdit_httpsPort->text().trimmed().toInt();
    if (ports.contains(httpsPort)) {
        ShowMessageBox(GET_TEXT("MYLINETIP/112011", "Already existed."));
        return false;
    }
    ports.insert(httpsPort);
    //
    int rtspPort = ui->lineEdit_rtspPort->text().trimmed().toInt();
    if (ports.contains(rtspPort)) {
        ShowMessageBox(GET_TEXT("MYLINETIP/112011", "Already existed."));
        return false;
    }
    ports.insert(rtspPort);

    //
    return true;
}

void NetworkPageMore::initializeData()
{
    gotoNetMoreTab();
}

bool NetworkPageMore::checkPort(QString type, int default_port, int now, int min, int max)
{
    if (default_port != now) {
        if (now < min || now > max) {
            ShowMessageBox(GET_TEXT("SYSTEMNETWORK/71108", "%1 Port: %2 or %3-%4.").arg(type).arg(default_port).arg(min).arg(max));
            return false;
        }
        return true;
    }
    return true;
}

int NetworkPageMore::is_port_use(int port)
{
    FILE *fp;
    int result = 0;
    char cmd[256];

    snprintf(cmd, sizeof(cmd), "netstat -an | grep \"0.0.0.0:%d \" > /tmp/port", port);
    ms_system(cmd);
    fp = fopen("/tmp/port", "rb");
    if (!fp)
        return 0;

    while (fgets(cmd, sizeof(cmd), fp)) {
        if (cmd[0] != '\0') {
            result = 1;
            break;
        }
    }

    fclose(fp);
    ms_system("rm -f /tmp/port");

    return result;
}

int NetworkPageMore::is_port_check(int port)
{
    int max_poe_num = get_param_int(SQLITE_FILE_NAME, PARAM_POE_NUM, 0);
    if (max_poe_num <= 0)
        return 0;
    if (max_poe_num == 4) {
        if (port >= 23001 && port <= 23004) {
            return 1;
        }
    } else if (max_poe_num == 8) {
        if (port >= 23001 && port <= 23008) {
            return 1;
        }
    } else if (max_poe_num == 16) {
        if (port >= 23001 && port <= 23016) {
            return 1;
        }
    }
    return 0;
}

bool NetworkPageMore::isBrowserRestrictions(int port)
{
    if (port == 2049 || port == 3659 || port == 4045 || port == 6000 || port == 6665 || port == 6666 || port == 6667 || port == 6668 || port == 6669) {
        return false;
    }
    return true;
}

void NetworkPageMore::onLanguageChanged()
{
    ui->label_pushMsg->setText(GET_TEXT("SYSTEMNETWORK/71054", "Push Message"));
    ui->label_ssh->setText(GET_TEXT("SYSTEMNETWORK/71055", "SSH Enable"));
    ui->label_sshPort->setText(GET_TEXT("SYSTEMNETWORK/71056", "SSH Port"));
    ui->label_httpPort->setText(GET_TEXT("SYSTEMNETWORK/71057", "HTTP Port"));
    ui->label_httpsPort->setText(GET_TEXT("SYSTEMNETWORK/71059", "HTTPS Port"));
    ui->label_rtspPort->setText(GET_TEXT("SYSTEMNETWORK/71058", "RTSP Port"));
    ui->label_sdkport->setText(GET_TEXT("", "Service Port"));
    ui->label_pushStream->setText(GET_TEXT("SYSTEMNETWORK/71206", "Push Stream Type"));
    ui->label_pushType->setText(GET_TEXT("SYSTEMNETWORK/71016", "Push Message Settings"));
    ui->pushButton_pushType->setText(GET_TEXT("COMMON/1019", "Edit"));

    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->comboBox_pushMsg->setItemText(0, GET_TEXT("COMMON/1018", "Disable"));
    ui->comboBox_pushMsg->setItemText(1, GET_TEXT("COMMON/1009", "Enable"));

    ui->comboBox_ssh->setItemText(0, GET_TEXT("COMMON/1018", "Disable"));
    ui->comboBox_ssh->setItemText(1, GET_TEXT("COMMON/1009", "Enable"));
}

void NetworkPageMore::slotSshEnableChanged(int index)
{
    if (index == 1) {
        ui->lineEdit_sshPort->setEnabled(true);
    } else {
        ui->lineEdit_sshPort->setEnabled(false);
    }
}

bool NetworkPageMore::saveSetting()
{
    if (!isValidatorMoreInput()) {
        return false;
    }
    saveMoreConfig();
    //
    if (pNetMoreDbOther == NULL) {
        pNetMoreDbOther = new network_more;
    }
    memset(pNetMoreDbOther, 0, sizeof(network_more));
    read_port(SQLITE_FILE_NAME, pNetMoreDbOther);

    //
    if (pNetMoreDbOther->ssh_port != pNetMoreDb->ssh_port) {
        if (is_port_check(pNetMoreDb->ssh_port) || is_port_use(pNetMoreDb->ssh_port)) {
            ShowMessageBox(GET_TEXT("SYSTEMNETWORK/71204", "Internal Reserved port, please use other numbers."));
            return false;
        }
    }
    if (pNetMoreDbOther->http_port != pNetMoreDb->http_port) {
        if (is_port_check(pNetMoreDb->http_port) || is_port_use(pNetMoreDb->http_port)) {
            ShowMessageBox(GET_TEXT("SYSTEMNETWORK/71204", "Internal Reserved port, please use other numbers."));
            return false;
        }
    }
    if (pNetMoreDbOther->https_port != pNetMoreDb->https_port) {
        if (is_port_check(pNetMoreDb->https_port) || is_port_use(pNetMoreDb->https_port)) {
            ShowMessageBox(GET_TEXT("SYSTEMNETWORK/71204", "Internal Reserved port, please use other numbers."));
            return false;
        }
    }
    if (pNetMoreDbOther->rtsp_port != pNetMoreDb->rtsp_port) {
        if (is_port_check(pNetMoreDb->rtsp_port) || is_port_use(pNetMoreDb->rtsp_port)) {
            ShowMessageBox(GET_TEXT("SYSTEMNETWORK/71204", "Internal Reserved port, please use other numbers."));
            return false;
        }
    }
    if (pNetMoreDbOther->posPort != pNetMoreDb->posPort) {
        if (is_port_check(pNetMoreDb->posPort) || is_port_use(pNetMoreDb->posPort)) {
            ShowMessageBox(GET_TEXT("SYSTEMNETWORK/71204", "Internal Reserved port, please use other numbers."));
            return false;
        }
    }

    //Chrome、Edge、Firefox的默认非安全端口，设置后不可访问，需按增加提示
    if (!isBrowserRestrictions(pNetMoreDb->http_port) || !isBrowserRestrictions(pNetMoreDb->https_port)) {
        int result = MessageBox::question(this, GET_TEXT("SYSTEMNETWORK/157000", "HTTP Port or HTTPS Port is incompatible with some browsers, continue?"));
        if (result == MessageBox::Cancel) {
            return false;
        }
    }

    //
    if (ui->comboBox_pushMsg->currentIndex() == 1) {
        if (!check_param_key(SQLITE_FILE_NAME, PARAM_PUSH_MSG))
            set_param_value(SQLITE_FILE_NAME, PARAM_PUSH_MSG, "1");
        else
            add_param_value(SQLITE_FILE_NAME, PARAM_PUSH_MSG, "1");
    } else {
        set_param_value(SQLITE_FILE_NAME, PARAM_PUSH_MSG, "0");
    }

    set_param_int(SQLITE_FILE_NAME, PARAM_PUSH_VIDEO_STREAM, ui->comboBox_pushStream->currentIndex());

    if (memcmp(pNetMoreDb, pNetMoreDbOri, sizeof(network_more)) != 0) {
        int wantToReboot = 0;
        if (pNetMoreDbOri->rtsp_port != pNetMoreDb->rtsp_port
            || pNetMoreDbOri->sdk_port != pNetMoreDb->sdk_port) {
            wantToReboot = 1;
        }

        if (wantToReboot) {
            int result = MessageBox::question(this, GET_TEXT("SYSTEMNETWORK/71109", "The change will be effective after reboot. Do you want to reboot now?"));
            if (result == MessageBox::Yes) {
                qMsNvr->rebootLater(2000);
                qMsApp->setAboutToReboot(true);
            } else {
                return false;
            }
        }

        //qDebug()<<"DbWritePort";
        write_port(SQLITE_FILE_NAME, pNetMoreDb);

        //
        if (pNetMoreDbOri->posPort != pNetMoreDb->posPort) {
            qMsDebug() << "REQUEST_FLAG_SET_POS_PORT,"
                       << "port:" << pNetMoreDb->posPort;
            sendMessage(REQUEST_FLAG_SET_POS_PORT, (void *)&pNetMoreDb->posPort, sizeof(pNetMoreDb->posPort));
        }

        //
        struct req_set_sysconf timeconf;
        memset(&timeconf, 0, sizeof(timeconf));
        snprintf(timeconf.arg, sizeof(timeconf.arg), "%s", "more");
        sendMessageOnly(REQUEST_FLAG_SET_NETWORK, (void *)&timeconf, sizeof(timeconf));

        //
        memcpy(pNetMoreDbOri, pNetMoreDb, sizeof(network_more));

        //TODO: log
        //DbLogWrite(current_user.username, SUB_OP_CONFIG_LOCAL, &ch_mask, SUB_PARAM_NETWORK_MORE);
    }
    return true;
}

void NetworkPageMore::on_pushButton_pushType_clicked()
{
    ui->pushButton_pushType->clearUnderMouse();
    ui->pushButton_pushType->clearFocus();

    PushEventEdit dialog(this);
    dialog.setType(IPCPushType, NVRAlarmInType);
    dialog.setNvrEventPos(&m_pushMsgNvrEvtDb);
    int result = dialog.exec();

    if (result == PushEventEdit::Accepted) {
        IPCPushType = dialog.getIPCPushType();
        NVRAlarmInType = dialog.getNVRAlarmInType();
    }
}

void NetworkPageMore::on_pushButton_apply_clicked()
{
    if (!saveSetting()) {
        return;
    }

    int chnCnt = IPCPushType.size();
    push_msg_event event[chnCnt];
    memset(&event, 0, chnCnt * sizeof(push_msg_event));
    for (int i = 0; i < chnCnt; i++) {
        event[i].chnid = i;
        event[i].push_type = IPCPushType.value(i, 131071);
    }
    write_push_msg_events(SQLITE_FILE_NAME, event, chnCnt);
    set_param_int(SQLITE_FILE_NAME, PARAM_PUSH_ALARMIN_TYPE, NVRAlarmInType);
    write_push_msg_nvr_event(SQLITE_FILE_NAME, &m_pushMsgNvrEvtDb);
    sendMessageOnly(REQUEST_FLAG_PUSH_MSG, (void *)NULL, 0);
}

void NetworkPageMore::on_pushButton_back_clicked()
{
    emit sig_back();
}
