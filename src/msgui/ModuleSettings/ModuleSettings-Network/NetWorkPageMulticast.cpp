#include "NetWorkPageMulticast.h"
#include "ui_NetWorkPageMulticast.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "PushEventEdit.h"
#include "centralmessage.h"
NetWorkPageMulticast::NetWorkPageMulticast(QWidget *parent)
    : AbstractNetworkPage(parent)
    , ui(new Ui::NetWorkPageMulticast)
{
    ui->setupUi(this);
    ui->comboBoxEnable->clear();
    ui->comboBoxEnable->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBoxEnable->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);

    ui->lineEditIP->setCheckMode(MyLineEdit::IPv4Check);

    onLanguageChanged();
}

NetWorkPageMulticast::~NetWorkPageMulticast()
{
    delete ui;
}

void NetWorkPageMulticast::initializeData()
{
    DbMulticast multicast;
    memset(&multicast, 0, sizeof(DbMulticast));
    read_multicast(SQLITE_FILE_NAME, &multicast);
    ui->comboBoxEnable->setCurrentIndexFromData(multicast.enable);
    on_comboBoxEnable_activated(ui->comboBoxEnable->currentIndex());
    ui->lineEditIP->setText(multicast.ip);
}

void NetWorkPageMulticast::processMessage(MessageReceive *message)
{
    switch (message->header.type) {
    case RESPONSE_FLAG_SET_NETWORK_MULTICAST:
        ON_RESPONSE_FLAG_SET_NETWORK_MULTICAST(message);
        break;
    }
}

void NetWorkPageMulticast::ON_RESPONSE_FLAG_SET_NETWORK_MULTICAST(MessageReceive *message)
{
    Q_UNUSED(message);
    //closeWait();
}

QString NetWorkPageMulticast::ipAdjust(QString str)
{
    int i, val;
    QString pStr;
    QString ret = "";
    QStringList strList;
    strList = str.split(".");
    for (i = 0; i < 4; i++) {
        pStr = strList[i];
        val = pStr.toInt();
        ret += QString("%1").arg(val);
        if (i < 3)
            ret += QString(".");
    }
    return ret;
}

void NetWorkPageMulticast::onLanguageChanged()
{
    ui->labelEnable->setText(GET_TEXT("NETWORKSTATUS/144000", "Multicast"));
    ui->labelIP->setText(GET_TEXT("NETWORKSTATUS/144001", "IP Address"));
    ui->labelNote->setText(GET_TEXT("NETWORKSTATUS/144002", "Note: Valid IP address range: 224.0.0.0~239.255.255.255."));
    ui->pushButtonApply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
}

void NetWorkPageMulticast::on_comboBoxEnable_activated(int index)
{
    if (index == 1) {
        ui->lineEditIP->setEnabled(true);
    } else {
        ui->lineEditIP->setEnabled(false);
    }
}

void NetWorkPageMulticast::on_pushButtonApply_clicked()
{
    DbMulticast multicast;
    memset(&multicast, 0, sizeof(DbMulticast));
    read_multicast(SQLITE_FILE_NAME, &multicast);
    if (ui->comboBoxEnable->currentIntData()) {
        bool valid = ui->lineEditIP->checkValid();
        if (valid) {
            QString strIp = ui->lineEditIP->text();
            strIp = ipAdjust(strIp);
            quint32 startAddress = QHostAddress("224.0.0.0").toIPv4Address();
            quint32 endAddress = QHostAddress("239.255.255.255").toIPv4Address();
            quint32 ipAddress = QHostAddress(strIp).toIPv4Address();
            if (ipAddress < startAddress || ipAddress > endAddress) {
                ui->lineEditIP->setCustomValid(false, GET_TEXT("MYLINETIP/112001", "Invalid."));
                valid = false;
            }
        }
        if (!valid) {
            return;
        }
    }
    multicast.enable = ui->comboBoxEnable->currentIntData();
    snprintf(multicast.ip, sizeof(multicast.ip), "%s", ui->lineEditIP->text().toStdString().c_str());
    write_multicast(SQLITE_FILE_NAME, &multicast);
    sendMessage(REQUEST_FLAG_SET_NETWORK_MULTICAST, &multicast, sizeof(struct DbMulticast));
    //showWait();
}

void NetWorkPageMulticast::on_pushButtonBack_clicked()
{
    emit sig_back();
}
