#include "NetworkPagePPPoE.h"
#include "ui_NetworkPagePPPoE.h"
#include "centralmessage.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include <QDebug>

NetworkPagePPPoe::NetworkPagePPPoe(QWidget *parent)
    : AbstractNetworkPage(parent)
    , ui(new Ui::NetworkPagePPPoe)
{
    ui->setupUi(this);
    initNetPPPoETab();

    ui->lineEdit_username->setCheckMode(MyLineEdit::EmptyCheck);
    ui->lineEdit_password->setCheckMode(MyLineEdit::EmptyCheck);
    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

NetworkPagePPPoe::~NetworkPagePPPoe()
{
    delete ui;
}

void NetworkPagePPPoe::initializeData()
{
    gotoNetPPPoETab();
}

void NetworkPagePPPoe::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void NetworkPagePPPoe::initNetPPPoETab()
{
    ui->comboBox_pppoe->clear();
    ui->comboBox_pppoe->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBox_pppoe->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);

    ui->lineEdit_dynamicIp->setDisabled(true);
}

void NetworkPagePPPoe::gotoNetPPPoETab()
{
    //showWait();
    memset(&m_dbPppoe, 0x0, sizeof(struct pppoe));
    read_pppoe(SQLITE_FILE_NAME, &m_dbPppoe);

    ui->comboBox_pppoe->setCurrentIndex(m_dbPppoe.enable);
    ui->lineEdit_username->setText(QString(m_dbPppoe.username));
    ui->lineEdit_password->setText(QString(m_dbPppoe.password));
    ui->lineEdit_confirm->setText(QString(m_dbPppoe.password));
    on_comboBox_pppoe_activated(m_dbPppoe.enable);
    onGetDynamicIp();
}

void NetworkPagePPPoe::onLanguageChanged()
{
    const struct device_info &sys_info = qMsNvr->deviceInfo();
    if (sys_info.max_lan == 1 || qMsNvr->isPoe())
        ui->label_pppoe->setText(GET_TEXT("SYSTEMNETWORK/71006", "PPPoE"));
    else
        ui->label_pppoe->setText(GET_TEXT("SYSTEMNETWORK/71014", "LAN1 PPPoE"));

    ui->label_dynamicIp->setText(GET_TEXT("SYSTEMNETWORK/71024", "Dynamic IP"));
    ui->label_username->setText(GET_TEXT("COMMON/1007", "User Name"));
    ui->label_password->setText(GET_TEXT("COMMON/1008", "Password"));
    ui->label_confirm->setText(GET_TEXT("USER/74055", "Confirm Password"));
    ui->label_note->setText(GET_TEXT("SYSTEMNETWORK/71025", "Note: If both UPnP and PPPoE are enabled, only PPPoE will take effect. "));
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->comboBox_pppoe->setItemText(0, GET_TEXT("COMMON/1018", "Disable"));
    ui->comboBox_pppoe->setItemText(1, GET_TEXT("COMMON/1009", "Enable"));
}

bool NetworkPagePPPoe::isValidatorPPPoEInput()
{
    if (ui->comboBox_pppoe->currentIndex() == 0) {
        return true;
    }
    //
    QString userName = ui->lineEdit_username->text();
    bool valid = ui->lineEdit_username->checkValid();
    valid = ui->lineEdit_password->checkValid() && valid;

    if (userName.contains(" ")) {
        ui->lineEdit_username->setCustomValid(false, GET_TEXT("MYLINETIP/112012", "Invalid character: space."));
        valid = false;
    }
    //
    QString strPsw = ui->lineEdit_password->text();

    if (strPsw.contains(" ")) {
        ui->lineEdit_password->setCustomValid(false, GET_TEXT("MYLINETIP/112012", "Invalid character: space."));
        valid = false;
    }

    QString strConfirm = ui->lineEdit_confirm->text();
    if (!ui->lineEdit_password->text().isEmpty() && strConfirm != strPsw) {
        ui->lineEdit_confirm->setCustomValid(false, GET_TEXT("MYLINETIP/112008", "Passwords do not match. "));
        valid = false;
    }

    if (!valid) {
        return false;
    }

    return true;
}

void NetworkPagePPPoe::on_pushButton_apply_clicked()
{
    struct pppoe tmpPppoe;
    char confirm[64] = { 0 };
    int flag = 0;

    if (!isValidatorPPPoEInput())
        return;

    memset(&tmpPppoe, 0x0, sizeof(struct pppoe));
    tmpPppoe.enable = ui->comboBox_pppoe->currentIndex();
    if (tmpPppoe.enable == 0 && tmpPppoe.enable != m_dbPppoe.enable) {
        m_dbPppoe.enable = tmpPppoe.enable;
        flag = 1;
        write_pppoe(SQLITE_FILE_NAME, &m_dbPppoe);
    } else {
        snprintf(tmpPppoe.username, sizeof(tmpPppoe.username), "%s", ui->lineEdit_username->text().trimmed().toStdString().c_str());
        snprintf(tmpPppoe.password, sizeof(tmpPppoe.password), "%s", ui->lineEdit_password->text().trimmed().toStdString().c_str());
        snprintf(confirm, sizeof(confirm), "%s", ui->lineEdit_confirm->text().trimmed().toStdString().c_str());
        if (memcmp(&tmpPppoe, &m_dbPppoe, sizeof(struct pppoe))) {
            flag = 1;
            memcpy(&m_dbPppoe, &tmpPppoe, sizeof(struct pppoe));
            write_pppoe(SQLITE_FILE_NAME, &tmpPppoe);
        }
    }

    if (flag) {
        //showWait();
        struct req_set_sysconf req;
        memset(&req, 0, sizeof(struct req_set_sysconf));
        snprintf(req.arg, sizeof(req.arg), "%s", "pppoe");
        sendMessageOnly(REQUEST_FLAG_SET_NETWORK, (void *)&req, sizeof(struct req_set_sysconf));

        QTimer::singleShot(2500, this, SLOT(onGetDynamicIp()));
    }
}

void NetworkPagePPPoe::on_comboBox_pppoe_activated(int index)
{
    if (index) {
        ui->lineEdit_username->setDisabled(false);
        ui->lineEdit_password->setDisabled(false);
        ui->lineEdit_confirm->setDisabled(false);
#if 0	
		ui->label_dynamicIp->show();
		ui->lineEdit_dynamicIp->show();
		ui->label_username->show();
		ui->label_password->show();
		ui->label_confirm->show();
		ui->lineEdit_username->show();
		ui->lineEdit_password->show();
		ui->lineEdit_confirm->show();
#endif
    } else {
        ui->lineEdit_username->setDisabled(true);
        ui->lineEdit_password->setDisabled(true);
        ui->lineEdit_confirm->setDisabled(true);
#if 0	
		ui->label_dynamicIp->hide();
		ui->lineEdit_dynamicIp->hide();
		ui->label_username->hide();
		ui->label_password->hide();
		ui->label_confirm->hide();
		ui->lineEdit_username->hide();
		ui->lineEdit_password->hide();
		ui->lineEdit_confirm->hide();
#endif
    }
}

void NetworkPagePPPoe::onGetDynamicIp()
{
    char dynamicIP[64] = "0.0.0.0";
    net_get_ifaddr(DEVICE_NAME_PPPOE, dynamicIP, sizeof(dynamicIP));
    ui->lineEdit_dynamicIp->setText(QString(dynamicIP));

    //closeWait();
}

void NetworkPagePPPoe::on_pushButton_back_clicked()
{
    emit sig_back();
}
