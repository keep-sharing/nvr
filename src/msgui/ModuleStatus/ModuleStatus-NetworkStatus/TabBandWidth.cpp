#include "TabBandWidth.h"
#include "ui_TabBandWidth.h"
#include "centralmessage.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include <QTimer>
#include <QtDebug>

extern "C" {
#include "msdb.h"
}

TabBandWidth::TabBandWidth(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::TabBandWidth)
{
    ui->setupUi(this);

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    m_timer->setInterval(2000);

    ui->chart_lan1Receive->setLineColor(QColor(247, 190, 129));
    ui->chart_lan2Receive->setLineColor(QColor(247, 190, 129));
    ui->chart_lan1Send->setLineColor(QColor(55, 178, 92));
    ui->chart_lan2Send->setLineColor(QColor(55, 178, 92));

    onLanguageChanged();
}

TabBandWidth::~TabBandWidth()
{
    delete ui;
}

void TabBandWidth::initializeData()
{
    const device_info &sys_info = qMsNvr->deviceInfo();

    struct network db_network;
    memset(&db_network, 0, sizeof(struct network));
    read_network(SQLITE_FILE_NAME, &db_network);
    if (db_network.mode == NETMODE_MULTI) {
        if (sys_info.max_lan == 1 || qMsNvr->isPoe()) {
            ui->groupBox_lan2->hide();
        }
    } else {
        ui->groupBox_lan2->hide();
    }
}

void TabBandWidth::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_NETWORK_BANDWIDTH:
        ON_RESPONSE_FLAG_GET_NETWORK_BANDWIDTH(message);
        break;
    case RESPONSE_FLAG_GET_NETWORK_SPEED:
        ON_RESPONSE_FLAG_GET_NETWORK_SPEED(message);
        break;
    }
}

void TabBandWidth::ON_RESPONSE_FLAG_GET_NETWORK_BANDWIDTH(MessageReceive *message)
{
    if (message->isNull()) {
        return;
    }
    struct resp_network_bandwidth *info = (struct resp_network_bandwidth *)message->data;

    QString strFree = QString("%1 Mbps").arg(QLocale().toString(qreal(info->free) / 1024, 'f', 2));
    ui->lineEdit_bandwidthFree->setText(strFree);
    QString strUsed = QString("%1 Mbps").arg(QLocale().toString(qreal(info->used) / 1024, 'f', 2));
    ui->lineEdit_bandwidthUsed->setText(strUsed);
}

void TabBandWidth::ON_RESPONSE_FLAG_GET_NETWORK_SPEED(MessageReceive *message)
{
    if (message->isNull()) {
        qWarning() << QString("ON_RESPONSE_FLAG_GET_NETWORK_INFO, data is null.");
        return;
    }
    struct resp_network_speed *network_speed_list = (struct resp_network_speed *)message->data;
    int count = message->size() / sizeof(struct resp_network_speed);
    for (int i = 0; i < count; ++i) {
        const resp_network_speed &network_speed = network_speed_list[i];
        if (network_speed.netif_id == NETIF_LAN0 || network_speed.netif_id == NETIF_BOND0) {
            int rx_speed = network_speed.rx_speed;
            int tx_speed = network_speed.tx_speed;
            ui->label_lan1Receive->setText(QString("%1: %2").arg(GET_TEXT("NETWORKSTATUS/61021", "Receive Rate")).arg(NetworkChart::valueString(rx_speed)));
            ui->label_lan1Send->setText(QString("%1: %2").arg(GET_TEXT("NETWORKSTATUS/61022", "Send Rate")).arg(NetworkChart::valueString(tx_speed)));
            ui->chart_lan1Receive->appendValue(rx_speed);
            ui->chart_lan1Send->appendValue(tx_speed);
        }
        if (network_speed.netif_id == NETIF_LAN1) {
            int rx_speed = network_speed.rx_speed;
            int tx_speed = network_speed.tx_speed;
            ui->label_lan2Receive->setText(QString("%1: %2").arg(GET_TEXT("NETWORKSTATUS/61021", "Receive Rate")).arg(NetworkChart::valueString(rx_speed)));
            ui->label_lan2Send->setText(QString("%1: %2").arg(GET_TEXT("NETWORKSTATUS/61022", "Send Rate")).arg(NetworkChart::valueString(tx_speed)));
            ui->chart_lan2Receive->appendValue(rx_speed);
            ui->chart_lan2Send->appendValue(tx_speed);
        }
    }
}

void TabBandWidth::showEvent(QShowEvent *event)
{
    ui->chart_lan1Receive->clear();
    ui->chart_lan1Send->clear();
    ui->chart_lan2Receive->clear();
    ui->chart_lan2Send->clear();
    m_requestCount = 30;
    m_timer->start();
    QWidget::showEvent(event);
}

void TabBandWidth::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

void TabBandWidth::onLanguageChanged()
{
    ui->groupBox_bandwidth->setTitle(GET_TEXT("NETWORKSTATUS/61019", "Receive Bandwidth"));
    ui->label_free->setText(GET_TEXT("NETWORKSTATUS/61020", "Free"));
    ui->label_used->setText(GET_TEXT("NETWORKSTATUS/61023", "Used"));
    ui->groupBox_lan1->setTitle(GET_TEXT("CAMERASEARCH/32022", "LAN1"));
    ui->groupBox_lan2->setTitle(GET_TEXT("CAMERASEARCH/32023", "LAN2"));
    ui->label_lan1Receive->setText(GET_TEXT("NETWORKSTATUS/61021", "Receive Rate"));
    ui->label_lan2Receive->setText(GET_TEXT("NETWORKSTATUS/61021", "Receive Rate"));
    ui->label_lan1Send->setText(GET_TEXT("NETWORKSTATUS/61022", "Send Rate"));
    ui->label_lan2Send->setText(GET_TEXT("NETWORKSTATUS/61022", "Send Rate"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
}

void TabBandWidth::onTimer()
{
    m_requestCount++;
    if (m_requestCount > 30) {
        sendMessage(REQUEST_FLAG_GET_NETWORK_BANDWIDTH, (void *)NULL, 0);
        m_requestCount = 0;
    }

    //
    sendMessage(REQUEST_FLAG_GET_NETWORK_SPEED, (void *)NULL, 0);
}

void TabBandWidth::on_pushButtonBack_clicked()
{
    emit sig_back();
}
