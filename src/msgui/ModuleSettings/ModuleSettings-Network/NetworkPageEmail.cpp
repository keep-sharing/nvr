#include "NetworkPageEmail.h"
#include "ui_NetworkPageEmail.h"
#include "centralmessage.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include <QtDebug>

extern "C" {
#include "msdb.h"
}

NetworkPageEmail::NetworkPageEmail(QWidget *parent)
    : AbstractNetworkPage(parent)
    , ui(new Ui::NetworkPageEmail)
{
    ui->setupUi(this);

    initNetMailTab();

    ui->lineEdit_smtpServer->setCheckMode(MyLineEdit::DDNSCheck);
    ui->lineEdit_smtpPort->setCheckMode(MyLineEdit::RangeCheck, 1, 65535);
    ui->lineEdit_mailHostname->setCheckMode(MyLineEdit::DDNSCheck);
    ui->lineEdit_mailPassword->setCheckMode(MyLineEdit::SpecialStrCheck);
    ui->lineEdit_mailUserName->setCheckMode(MyLineEdit::SpecialStrCheck);
    ui->lineEdit_receiverName->setCheckMode(MyLineEdit::EmailCanEmptyCheck);
    ui->lineEdit_receiverName2->setCheckMode(MyLineEdit::EmailCanEmptyCheck);
    ui->lineEdit_receiverName3->setCheckMode(MyLineEdit::EmailCanEmptyCheck);
    ui->lineEdit_senderAddr->setCheckMode(MyLineEdit::EmailCheck);

    onLanguageChanged();
}

NetworkPageEmail::~NetworkPageEmail()
{
    freeNetMailTab();
    delete ui;
}

void NetworkPageEmail::initializeData()
{
    gotoNetMailTab();
}

void NetworkPageEmail::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_TEST_MAIL:
        ON_RESPONSE_FLAG_TEST_MAIL(message);
        break;
    }
}

void NetworkPageEmail::initNetMailTab()
{
    QRegExp rx(QString("[0-9]*"));
    QValidator *portValidator = new QRegExpValidator(rx, this);
    ui->lineEdit_smtpPort->setValidator(portValidator);

    ui->comboBox_snapshotInterval->clear();
    ui->comboBox_snapshotInterval->addItem(QString("2s"), 2);
    ui->comboBox_snapshotInterval->addItem(QString("3s"), 3);
    ui->comboBox_snapshotInterval->addItem(QString("4s"), 4);
    ui->comboBox_snapshotInterval->addItem(QString("5s"), 5);

    connect(ui->checkBox_mailEnableAttach, SIGNAL(toggled(bool)), this, SLOT(slotEailEnableAttach(bool)));

    connect(ui->pushButton_apply, SIGNAL(clicked(bool)), this, SLOT(slotMailApplyBtn()));
    connect(ui->pushButton_back, SIGNAL(clicked(bool)), this, SLOT(slotMailBackBtn()));
    connect(ui->pushButton_test, SIGNAL(clicked(bool)), this, SLOT(slotMailTestBtn()));

    _cur_receiver_index = 0;

    pEmailDb = NULL;
    pEmailDbOri = NULL;
    pMailNetMoreDb = NULL;
    pMailNetMoreDbOri = NULL;

    ui->checkBox_mailEnableAttach->hide();
    ui->comboBox_snapshotInterval->hide();
    ui->label_snapshotInterval->hide();
}

void NetworkPageEmail::gotoNetMailTab()
{
    if (pEmailDb == NULL) {
        pEmailDb = new struct email;
    }
    if (pEmailDbOri == NULL) {
        pEmailDbOri = new struct email;
    }

    if (pMailNetMoreDb == NULL) {
        pMailNetMoreDb = new struct network_more;
    }
    if (pMailNetMoreDbOri == NULL) {
        pMailNetMoreDbOri = new struct network_more;
    }
    if (NetSnmpPort == nullptr) {
        NetSnmpPort = new snmp;
    }

    memset(NetSnmpPort, 0, sizeof(snmp));

    memset(pEmailDb, 0, sizeof(struct email));
    memset(pEmailDbOri, 0, sizeof(struct email));

    memset(pMailNetMoreDb, 0, sizeof(struct network_more));
    memset(pMailNetMoreDbOri, 0, sizeof(struct network_more));

    read_email(SQLITE_FILE_NAME, pEmailDb);
    memcpy(pEmailDbOri, pEmailDb, sizeof(struct email));

    read_port(SQLITE_FILE_NAME, pMailNetMoreDb);
    memcpy(pMailNetMoreDbOri, pMailNetMoreDb, sizeof(struct network_more));

    read_snmp(SQLITE_FILE_NAME, NetSnmpPort);

    readMailConfig();
}

void NetworkPageEmail::freeNetMailTab()
{
    if (pEmailDb) {
        delete pEmailDb;
        pEmailDb = nullptr;
    }
    if (pEmailDbOri) {
        delete pEmailDbOri;
        pEmailDbOri = nullptr;
    }
    if (pMailNetMoreDb) {
        delete pMailNetMoreDb;
        pMailNetMoreDb = nullptr;
    }
    if (pMailNetMoreDbOri) {
        delete pMailNetMoreDbOri;
        pMailNetMoreDbOri = nullptr;
    }
    if (NetSnmpPort) {
        delete NetSnmpPort;
        NetSnmpPort = nullptr;
    }
}

bool NetworkPageEmail::checkMail(const char *pszEmail)
{
    if (pszEmail == NULL) {
        return false;
    }
    int iAtPos = 0;
    int iLastDotPos = 0;
    int i = 0;
    int iAtTimes = 0;
    while (*(pszEmail + i) != '\0') {
        char ch = *(pszEmail + i);
        if (!isprint(ch) || isspace(ch)) //空格和控制字符是非法的，限制得还比较宽松
        {
            iAtTimes = 0;
            break;
        }
        if (ch == '@') {
            iAtPos = i;
            iAtTimes++;
        } else if (ch == '.') {
            iLastDotPos = i;
        }
        i++;
    }
    if (i > 64 || iAtPos < 1 || (iLastDotPos - 2) < iAtPos || (i - iLastDotPos) < 3 || (i - iLastDotPos) > 5 || iAtTimes > 1 || iAtTimes == 0) //对@以及域名依靠位置来判断，限制长度为64
    {
        return false;
    }
    return true;
}

void NetworkPageEmail::readMailConfig()
{
    ui->comboBox_mailEnable->setCurrentIndex(pEmailDb->enable);
    ui->lineEdit_mailUserName->setText(QString(pEmailDb->username));
    ui->lineEdit_mailPassword->setText(QString(pEmailDb->password));
    ui->lineEdit_smtpServer->setText(QString(pEmailDb->smtp_server));
    if (pEmailDb->port != 0)
        ui->lineEdit_smtpPort->setText(QString("%1").arg(pEmailDb->port));

    ui->lineEdit_senderAddr->setText(QString(pEmailDb->sender_addr));

    _cur_receiver_index = 0;
    ui->lineEdit_receiverName->setText(QString(pEmailDb->receiver[0].address));
    ui->lineEdit_receiverName2->setText(QString(pEmailDb->receiver[1].address));
    ui->lineEdit_receiverName3->setText(QString(pEmailDb->receiver[2].address));

    if (pEmailDb->enable_tls == 0) {
        ui->checkBox_enableTls->setChecked(false);
        ui->checkBox_enableSsl->setChecked(false);
    } else if (pEmailDb->enable_tls == 1) {
        ui->checkBox_enableTls->setChecked(true);
        ui->checkBox_enableSsl->setChecked(false);
    } else if (pEmailDb->enable_tls == 2) {
        ui->checkBox_enableTls->setChecked(false);
        ui->checkBox_enableSsl->setChecked(true);
    }

    if (pEmailDb->enable_attach) {
        ui->checkBox_mailEnableAttach->setChecked(true);
        slotEailEnableAttach(true);
    } else {
        ui->checkBox_mailEnableAttach->setChecked(false);
        slotEailEnableAttach(false);
    }
    if (pEmailDb->capture_interval != 0) {
        ui->comboBox_snapshotInterval->setCurrentIndex(ui->comboBox_snapshotInterval->findData(pEmailDb->capture_interval));
    }

    ui->checkBox_enableHostname->setChecked(pMailNetMoreDb->url_enable);
    ui->lineEdit_mailHostname->setText(QString(pMailNetMoreDb->url));
}

void NetworkPageEmail::saveMailConfig()
{
    pEmailDb->enable = ui->comboBox_mailEnable->currentIndex();
    snprintf(pEmailDb->username, sizeof(pEmailDb->username), "%s", ui->lineEdit_mailUserName->text().trimmed().toStdString().c_str());
    snprintf(pEmailDb->password, sizeof(pEmailDb->password), "%s", ui->lineEdit_mailPassword->text().trimmed().toStdString().c_str());
    snprintf(pEmailDb->smtp_server, sizeof(pEmailDb->smtp_server), "%s", ui->lineEdit_smtpServer->text().trimmed().toStdString().c_str());
    if (!ui->lineEdit_smtpPort->text().trimmed().isEmpty())
        pEmailDb->port = ui->lineEdit_smtpPort->text().trimmed().toInt();
    snprintf(pEmailDb->sender_addr, sizeof(pEmailDb->sender_addr), "%s", ui->lineEdit_senderAddr->text().trimmed().toStdString().c_str());

    char *name = strchr(pEmailDb->sender_addr, '@');
    //if (name)
    if (0) {
        snprintf(pEmailDb->sender_name, (int)(name - pEmailDb->sender_addr) + 1, "%s", pEmailDb->sender_addr);
    } else {
        snprintf(pEmailDb->sender_name, sizeof(pEmailDb->sender_addr), "%s", pEmailDb->username);
    }
    memset(pEmailDb->receiver[0].name, 0x0, sizeof(pEmailDb->receiver[0].name));
    snprintf(pEmailDb->receiver[0].address, sizeof(pEmailDb->receiver[0].address), "%s", ui->lineEdit_receiverName->text().trimmed().toStdString().c_str());
    memset(pEmailDb->receiver[1].name, 0x0, sizeof(pEmailDb->receiver[1].name));
    snprintf(pEmailDb->receiver[1].address, sizeof(pEmailDb->receiver[1].address), "%s", ui->lineEdit_receiverName2->text().trimmed().toStdString().c_str());
    memset(pEmailDb->receiver[2].name, 0x0, sizeof(pEmailDb->receiver[2].name));
    snprintf(pEmailDb->receiver[2].address, sizeof(pEmailDb->receiver[2].address), "%s", ui->lineEdit_receiverName3->text().trimmed().toStdString().c_str());

    if (!ui->checkBox_enableSsl->isChecked() && !ui->checkBox_enableTls->isChecked()) {
        pEmailDb->enable_tls = 0;
    } else if (!ui->checkBox_enableSsl->isChecked() && ui->checkBox_enableTls->isChecked()) {
        pEmailDb->enable_tls = 1;
    } else if (ui->checkBox_enableSsl->isChecked() && !ui->checkBox_enableTls->isChecked()) {
        pEmailDb->enable_tls = 2;
    }

    if (ui->checkBox_mailEnableAttach->isChecked()) {
        pEmailDb->enable_attach = 1;
    } else {
        pEmailDb->enable_attach = 0;
    }
    //pEmailDb->capture_interval = ui->comboBox_snapshotInterval->currentIndex()+1;
    pEmailDb->capture_interval = ui->comboBox_snapshotInterval->itemData(ui->comboBox_snapshotInterval->currentIndex()).value<int>();

    pMailNetMoreDb->url_enable = (int)ui->checkBox_enableHostname->isChecked();
    snprintf(pMailNetMoreDb->url, sizeof(pMailNetMoreDb->url), "%s", ui->lineEdit_mailHostname->text().trimmed().toStdString().c_str());
}

bool NetworkPageEmail::isValidatorMailInput()
{
    bool valid = ui->lineEdit_mailUserName->checkValid();
    valid = ui->lineEdit_mailPassword->checkValid() && valid;
    valid = ui->lineEdit_smtpServer->checkValid() && valid;
    valid = ui->lineEdit_smtpPort->checkValid() && valid;
    valid = ui->lineEdit_senderAddr->checkValid() && valid;
    if (ui->lineEdit_mailHostname->isEnabled() && !ui->lineEdit_mailHostname->checkValid()) {
        valid = false;
    }
    QSet<int> ports;
    ports.insert(pMailNetMoreDb->ssh_port);
    ports.insert(pMailNetMoreDb->http_port);
    ports.insert(pMailNetMoreDb->https_port);
    ports.insert(pMailNetMoreDb->posPort);
    ports.insert(pMailNetMoreDb->rtsp_port);
    ports.insert(NetSnmpPort->port);

    int  emailPort = ui->lineEdit_smtpPort->text().trimmed().toInt();
    if (ports.contains(emailPort)) {
        ui->lineEdit_smtpPort->setCustomValid(false, GET_TEXT("MYLINETIP/112011", "Already existed."));
        valid = false;
    }

    //receiver adress
    if (ui->lineEdit_receiverName->text().isEmpty() && ui->lineEdit_receiverName2->text().isEmpty() && ui->lineEdit_receiverName3->text().isEmpty()) {
        ui->lineEdit_receiverName->setCustomValid(false, GET_TEXT("MYLINETIP/112000", "Cannot be empty."));
        valid = false;
    }


    //rece 1
    if (!ui->lineEdit_receiverName->text().isEmpty()) {
        if (!ui->lineEdit_receiverName->checkValid()) {
            valid = false;
        }
    }

    //rece 2
    if (!ui->lineEdit_receiverName2->text().isEmpty()) {
        if (!ui->lineEdit_receiverName2->checkValid()) {
            valid = false;
        }
    }

    //rece 3
    if (!ui->lineEdit_receiverName3->text().isEmpty()) {
        if (!ui->lineEdit_receiverName3->checkValid()) {
            valid = false;
        }
    }

    return valid;
}

void NetworkPageEmail::onLanguageChanged()
{
    ui->label_mailUserName->setText(GET_TEXT("COMMON/1007", "User Name"));
    ui->label_mailPassword->setText(GET_TEXT("COMMON/1008", "Password"));
    ui->label_smtpServer->setText(GET_TEXT("SYSTEMNETWORK/71030", "SMTP Server"));
    ui->label_smtpPort->setText(GET_TEXT("SYSTEMNETWORK/71031", "SMTP Port"));
    ui->label_senderAddress->setText(GET_TEXT("SYSTEMNETWORK/71033", "Sender Email Address"));
    ui->label_receiverName->setText(GET_TEXT("SYSTEMNETWORK/71047", "Receiver Email Address 1"));
    ui->label_receiverName2->setText(GET_TEXT("SYSTEMNETWORK/71048", "Receiver Email Address 2"));
    ui->label_receiverName3->setText(GET_TEXT("SYSTEMNETWORK/71049", "Receiver Email Address 3"));

    ui->label_encryption->setText(GET_TEXT("SYSTEMNETWORK/71037", "Encryption"));
    ui->checkBox_enableHostname->setText(GET_TEXT("SYSTEMNETWORK/71038", "Host Name"));
    ui->checkBox_mailEnableAttach->setText(GET_TEXT("SYSTEMNETWORK/71039", "Attached Image"));
    ui->label_snapshotInterval->setText(GET_TEXT("SYSTEMNETWORK/71040", "Snapshot Interval"));

    ui->pushButton_test->setText(GET_TEXT("CHANNELMANAGE/30020", "Test"));
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->label_mailEnable->setText(GET_TEXT("SYSTEMNETWORK/71003", "Email"));
    ui->comboBox_mailEnable->clear();
    ui->comboBox_mailEnable->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBox_mailEnable->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);
}

void NetworkPageEmail::ON_RESPONSE_FLAG_TEST_MAIL(MessageReceive *message)
{
    //closeWait();
    if (!message->data) {
        qWarning() << "NetworkPageEmail::ON_RESPONSE_FLAG_TEST_MAIL, data is null.";
        return;
    }
    int ret = *((int *)message->data);
    if (ret) {
        ShowMessageBox(GET_TEXT("SYSTEMNETWORK/71097", "Failed to send email,please check the parameters or network status."));
    } else {
        ShowMessageBox(GET_TEXT("LIVEVIEW/20049", "Email send successfully."));
    }
}

void NetworkPageEmail::slotEailEnableAttach(bool state)
{
    if (state == true) {
        ui->label_snapshotInterval->setEnabled(true);
        ui->comboBox_snapshotInterval->setEnabled(true);
    } else {
        ui->label_snapshotInterval->setEnabled(false);
        ui->comboBox_snapshotInterval->setEnabled(false);
        ;
    }
}

void NetworkPageEmail::on_checkBox_enableTls_clicked(bool checked)
{
    if (checked) {
        ui->checkBox_enableSsl->setChecked(false);
    }
}

void NetworkPageEmail::on_checkBox_enableSsl_clicked(bool checked)
{
    if (checked) {
        ui->checkBox_enableTls->setChecked(false);
    }
}

void NetworkPageEmail::slotMailTestBtn()
{
    if (!isValidatorMailInput())
        return;

    saveMailConfig();

    if (memcmp(pEmailDb, pEmailDbOri, sizeof(struct email)) != 0) {
        memcpy(pEmailDbOri, pEmailDb, sizeof(struct email));
        write_email(SQLITE_FILE_NAME, pEmailDb);

        struct req_set_sysconf timeconf;
        memset(&timeconf, 0, sizeof(timeconf));
        snprintf(timeconf.arg, sizeof(timeconf.arg), "%s", "mail");
        sendMessageOnly(REQUEST_FLAG_SET_NETWORK, (void *)&timeconf, sizeof(timeconf));
        //TODO: log
        //DbLogWrite(current_user.username, SUB_OP_CONFIG_LOCAL, &ch_mask, SUB_PARAM_NETWORK_MAIL);
    }

    struct req_set_sysconf sysconf;
    memset(&sysconf, 0, sizeof(req_set_sysconf));
    snprintf(sysconf.arg, sizeof(sysconf.arg), "%s", "testmail");
    //showWait();
    sendMessage(REQUEST_FLAG_TEST_MAIL, (void *)&sysconf, sizeof(sysconf));
}

void NetworkPageEmail::slotMailApplyBtn()
{
    pEmailDb->enable = ui->comboBox_mailEnable->currentIndex() == 1;
    if (pEmailDb->enable) {
        if (!isValidatorMailInput()) {
            return;
        }
    }

    saveMailConfig();
    if (memcmp(pEmailDb, pEmailDbOri, sizeof(struct email)) != 0) {
        memcpy(pEmailDbOri, pEmailDb, sizeof(struct email));
        //qDebug()<<"DbWriteEmail";
        write_email(SQLITE_FILE_NAME, pEmailDb);

        struct req_set_sysconf timeconf;
        memset(&timeconf, 0, sizeof(timeconf));
        snprintf(timeconf.arg, sizeof(timeconf.arg), "%s", "mail");
        //qDebug()<<"send set mail timeconf.arg="<<timeconf.arg;
        sendMessageOnly(REQUEST_FLAG_SET_NETWORK, (void *)&timeconf, sizeof(timeconf));
        //TODO: log
        //DbLogWrite(current_user.username, SUB_OP_CONFIG_LOCAL, &ch_mask, SUB_PARAM_NETWORK_MAIL);
    }

    if (memcmp(pMailNetMoreDb, pMailNetMoreDbOri, sizeof(struct network_more)) != 0) {
        memcpy(pMailNetMoreDbOri, pMailNetMoreDb, sizeof(struct network_more));
        write_port(SQLITE_FILE_NAME, pMailNetMoreDb);
    }
}

void NetworkPageEmail::slotMailBackBtn()
{
    freeNetMailTab();

    emit sig_back();
}

void NetworkPageEmail::on_comboBox_mailEnable_indexSet(int index)
{
    if (index == 0) {
        ui->lineEdit_mailUserName->setEnabled(false);
        ui->lineEdit_mailPassword->setEnabled(false);
        ui->lineEdit_smtpServer->setEnabled(false);
        ui->lineEdit_smtpPort->setEnabled(false);
        ui->lineEdit_senderAddr->setEnabled(false);
        ui->lineEdit_receiverName->setEnabled(false);
        ui->lineEdit_receiverName2->setEnabled(false);
        ui->lineEdit_receiverName3->setEnabled(false);
        ui->checkBox_enableTls->setEnabled(false);
        ui->checkBox_enableSsl->setEnabled(false);
        ui->lineEdit_mailHostname->setEnabled(false);
        ui->checkBox_enableHostname->setEnabled(false);
        ui->checkBox_mailEnableAttach->setEnabled(false);
        ui->comboBox_snapshotInterval->setEnabled(false);
        ui->pushButton_test->setEnabled(false);
    } else {
        ui->lineEdit_mailUserName->setEnabled(true);
        ui->lineEdit_mailPassword->setEnabled(true);
        ui->lineEdit_smtpServer->setEnabled(true);
        ui->lineEdit_smtpPort->setEnabled(true);
        ui->lineEdit_senderAddr->setEnabled(true);
        ui->lineEdit_receiverName->setEnabled(true);
        ui->lineEdit_receiverName2->setEnabled(true);
        ui->lineEdit_receiverName3->setEnabled(true);
        ui->checkBox_enableTls->setEnabled(true);
        ui->checkBox_enableSsl->setEnabled(true);
        ui->lineEdit_mailHostname->setEnabled(true);
        ui->checkBox_enableHostname->setEnabled(true);
        ui->checkBox_mailEnableAttach->setEnabled(true);
        ui->comboBox_snapshotInterval->setEnabled(true);
        ui->pushButton_test->setEnabled(true);
        int urlEnable = (int)ui->checkBox_enableHostname->isChecked();
        ui->lineEdit_mailHostname->setEnabled(urlEnable);
    }
}

void NetworkPageEmail::on_comboBox_mailEnable_activated(int index)
{
    on_comboBox_mailEnable_indexSet(index);
}

void NetworkPageEmail::on_checkBox_enableHostname_toggled(bool checked)
{
    if (ui->comboBox_mailEnable->currentData().toInt() == 1) {
        ui->lineEdit_mailHostname->setEnabled(checked);
    } else {
        ui->lineEdit_mailHostname->setEnabled(false);
    }
}
