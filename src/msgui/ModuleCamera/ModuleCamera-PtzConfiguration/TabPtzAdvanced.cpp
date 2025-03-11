#include "TabPtzAdvanced.h"
#include "ui_TabPtzAdvanced.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "channelcopydialog.h"
#include "MsDevice.h"
#include "MsLanguage.h"

TabPtzAdvanced::TabPtzAdvanced(QWidget *parent)
    : PtzBasePage(parent)
    , ui(new Ui::PtzAdvancedPage)
{
    ui->setupUi(this);

    ui->comboBox_channel->clear();
    for (int i = 0; i < qMsNvr->maxChannel(); ++i) {
        ui->comboBox_channel->addItem(QString::number(i + 1), i);
    }

    ui->comboBox_address->clear();
    for (int i = 0; i < 255; ++i) {
        ui->comboBox_address->addItem(QString::number(i + 1), i + 1);
    }

    ui->comboBox_connectionType->clear();
    ui->comboBox_connectionType->addItem("ONVIF", 0);
    if (qMsNvr->hasRs485()) {
        ui->comboBox_connectionType->addItem("RS485", 1);
    }

    ui->comboBox_baudRate->clear();
    ui->comboBox_baudRate->addItem(QString("%1").arg(1200), 1200);
    ui->comboBox_baudRate->addItem(QString("%1").arg(1800), 1800);
    ui->comboBox_baudRate->addItem(QString("%1").arg(2400), 2400);
    ui->comboBox_baudRate->addItem(QString("%1").arg(4800), 4800);
    ui->comboBox_baudRate->addItem(QString("%1").arg(9600), 9600);
    ui->comboBox_baudRate->addItem(QString("%1").arg(19200), 19200);
    ui->comboBox_baudRate->addItem(QString("%1").arg(38400), 38400);
    ui->comboBox_baudRate->addItem(QString("%1").arg(57600), 57600);
    ui->comboBox_baudRate->addItem(QString("%1").arg(115200), 115200);

    ui->comboBox_dataBit->clear();
    ui->comboBox_dataBit->addItem("5", 5);
    ui->comboBox_dataBit->addItem("6", 6);
    ui->comboBox_dataBit->addItem("7", 7);
    ui->comboBox_dataBit->addItem("8", 8);

    ui->comboBox_stopBit->clear();
    ui->comboBox_stopBit->addItem("1", 1);
    ui->comboBox_stopBit->addItem("2", 2);

    ui->comboBox_checksumBit->clear();
    ui->comboBox_checksumBit->addItem("None", 0);
    ui->comboBox_checksumBit->addItem("Odd", 1);
    ui->comboBox_checksumBit->addItem("Even", 2);

    ui->comboBox_protocol->clear();
    ui->comboBox_protocol->addItem("1602", 0);
    ui->comboBox_protocol->addItem("A01", 1);
    ui->comboBox_protocol->addItem("AB_D", 2);
    ui->comboBox_protocol->addItem("AB_P", 3);
    ui->comboBox_protocol->addItem("ADV", 4);
    ui->comboBox_protocol->addItem("HIKVISION", 5);
    ui->comboBox_protocol->addItem("LILIN", 6);
    ui->comboBox_protocol->addItem("PANASONIC_CS850", 7);
    ui->comboBox_protocol->addItem("PELCO_D", 8);
    ui->comboBox_protocol->addItem("PELCO_P", 9);
    ui->comboBox_protocol->addItem("SAMSUNG_E", 10);
    ui->comboBox_protocol->addItem("SAMSUNG_T", 11);
    ui->comboBox_protocol->addItem("SONY_D70", 12);
    ui->comboBox_protocol->addItem("YAAN", 13);

    onLanguageChanged();
}

TabPtzAdvanced::~TabPtzAdvanced()
{
    delete ui;
}

void TabPtzAdvanced::initializeData()
{
    //
    m_ptzPortMap.clear();
    m_ptzPortMapSource.clear();
    struct ptz_port ptz_port_array[MAX_CAMERA];
    memset(ptz_port_array, 0, sizeof(struct ptz_port) * MAX_CAMERA);
    int count = 0;
    read_ptz_ports(SQLITE_FILE_NAME, ptz_port_array, &count);
    for (int i = 0; i < count; ++i) {
        const struct ptz_port &port = ptz_port_array[i];
        m_ptzPortMap.insert(port.id, port);
    }
    m_ptzPortMapSource = m_ptzPortMap;

    ui->comboBox_channel->setCurrentIndexFromData(currentChannel());
    on_comboBox_channel_activated(ui->comboBox_channel->currentIndex());
}

void TabPtzAdvanced::copyData()
{
    if (m_copyList.isEmpty()) {

    } else {
        const int channel = m_copyList.takeFirst();
        if (channel != m_advancedChannel) {
            struct ptz_port &port = m_ptzPortMap[channel];
            struct ptz_port &portCurrent = m_ptzPortMap[m_advancedChannel];
            memcpy(&port, &portCurrent, sizeof(struct ptz_port));
            port.id = channel;
            if (isAdvancedDataChanged(channel)) {
                write_ptz_port(SQLITE_FILE_NAME, &port);
                sendMessageOnly(REQUEST_FLAG_PTZ_SET_SERIAL_PORT, (void *)&port.id, sizeof(int));
            }
        }
    }
}

bool TabPtzAdvanced::isAdvancedDataChanged(int channel)
{
    if (m_ptzPortMap.contains(channel) && m_ptzPortMapSource.contains(channel)) {
        struct ptz_port port1 = m_ptzPortMap.value(channel);
        struct ptz_port port2 = m_ptzPortMapSource.value(channel);
        if (memcpy(&port1, &port2, sizeof(struct ptz_port)) != nullptr) {
            return true;
        }
    }
    return false;
}

void TabPtzAdvanced::onLanguageChanged()
{
    ui->label_channel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));
    ui->label_connectionType->setText(GET_TEXT("PTZCONFIG/36041", "Connection Type"));
    ui->label_baudRate->setText(GET_TEXT("PTZCONFIG/36042", "Baud Rate"));
    ui->label_dataBit->setText(GET_TEXT("PTZCONFIG/36043", "Data Bit"));
    ui->label_stopBit->setText(GET_TEXT("PTZCONFIG/36044", "Stop Bit"));
    ui->label_checksumBit->setText(GET_TEXT("PTZCONFIG/36045", "Checksum Bit"));
    ui->label_protocol->setText(GET_TEXT("CHANNELMANAGE/30014", "Protocol"));
    ui->label_address->setText(GET_TEXT("PTZCONFIG/36047", "Address"));
    ui->pushButton_copy->setText(GET_TEXT("COMMON/1005", "Copy"));
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
}

void TabPtzAdvanced::on_comboBox_channel_activated(int index)
{
    m_advancedChannel = ui->comboBox_channel->itemData(index).toInt();
    if (m_ptzPortMap.contains(m_advancedChannel)) {
        const struct ptz_port &port = m_ptzPortMap.value(m_advancedChannel);
        ui->comboBox_connectionType->setCurrentIndexFromData(port.connect_type);
        ui->comboBox_baudRate->setCurrentIndexFromData(port.baudrate);
        ui->comboBox_dataBit->setCurrentIndexFromData(port.data_bit);
        ui->comboBox_stopBit->setCurrentIndexFromData(port.stop_bit);
        ui->comboBox_checksumBit->setCurrentIndexFromData(port.parity_type);
        ui->comboBox_protocol->setCurrentIndexFromData(port.protocol);
        ui->comboBox_address->setCurrentIndexFromData(port.address);

        on_comboBox_connectionType_activated(ui->comboBox_connectionType->currentIndex());
    } else {
        qMsWarning() << "invalid channel:" << m_advancedChannel;
    }
}

void TabPtzAdvanced::on_comboBox_connectionType_activated(int index)
{
    bool rs485 = (index == 1);

    if (m_ptzPortMap.contains(m_advancedChannel)) {
        struct ptz_port &port = m_ptzPortMap[m_advancedChannel];
        port.connect_type = ui->comboBox_connectionType->itemData(index).toInt();
    }

    ui->comboBox_baudRate->setEnabled(rs485);
    ui->comboBox_dataBit->setEnabled(rs485);
    ui->comboBox_stopBit->setEnabled(rs485);
    ui->comboBox_checksumBit->setEnabled(rs485);
    ui->comboBox_protocol->setEnabled(rs485);
    ui->comboBox_address->setEnabled(rs485);
}

void TabPtzAdvanced::on_comboBox_baudRate_activated(int index)
{
    if (m_ptzPortMap.contains(m_advancedChannel)) {
        struct ptz_port &port = m_ptzPortMap[m_advancedChannel];
        port.baudrate = ui->comboBox_baudRate->itemData(index).toInt();
    }
}

void TabPtzAdvanced::on_comboBox_dataBit_activated(int index)
{
    if (m_ptzPortMap.contains(m_advancedChannel)) {
        struct ptz_port &port = m_ptzPortMap[m_advancedChannel];
        port.data_bit = ui->comboBox_dataBit->itemData(index).toInt();
    }
}

void TabPtzAdvanced::on_comboBox_stopBit_activated(int index)
{
    if (m_ptzPortMap.contains(m_advancedChannel)) {
        struct ptz_port &port = m_ptzPortMap[m_advancedChannel];
        port.stop_bit = ui->comboBox_stopBit->itemData(index).toInt();
    }
}

void TabPtzAdvanced::on_comboBox_checksumBit_activated(int index)
{
    if (m_ptzPortMap.contains(m_advancedChannel)) {
        struct ptz_port &port = m_ptzPortMap[m_advancedChannel];
        port.parity_type = ui->comboBox_checksumBit->itemData(index).toInt();
    }
}

void TabPtzAdvanced::on_comboBox_protocol_activated(int index)
{
    if (m_ptzPortMap.contains(m_advancedChannel)) {
        struct ptz_port &port = m_ptzPortMap[m_advancedChannel];
        port.protocol = ui->comboBox_protocol->itemData(index).toInt();
    }
}

void TabPtzAdvanced::on_comboBox_address_activated(int index)
{
    if (m_ptzPortMap.contains(m_advancedChannel)) {
        struct ptz_port &port = m_ptzPortMap[m_advancedChannel];
        port.address = ui->comboBox_address->itemData(index).toInt();
    }
}

void TabPtzAdvanced::on_pushButton_copy_clicked()
{
    m_copyList.clear();

    ChannelCopyDialog copy(this);
    copy.setCurrentChannel(m_advancedChannel);
    int result = copy.exec();
    if (result == ChannelCopyDialog::Accepted) {
        m_copyList = copy.checkedList();
        QTimer::singleShot(0, this, SLOT(on_pushButton_apply_clicked()));
    }
}

void TabPtzAdvanced::on_pushButton_apply_clicked()
{
    if (isAdvancedDataChanged(m_advancedChannel)) {
        struct ptz_port port = m_ptzPortMap.value(m_advancedChannel);
        write_ptz_port(SQLITE_FILE_NAME, &port);
        sendMessageOnly(REQUEST_FLAG_PTZ_SET_SERIAL_PORT, (void *)&port.id, sizeof(int));
    }

    if (!m_copyList.isEmpty()) {
        //这个消息没有返回，所以这样做
        //showWait();
        while (!m_copyList.isEmpty()) {
            copyData();
            qApp->processEvents();
        }
        //closeWait();
    }

    m_ptzPortMapSource = m_ptzPortMap;
}

void TabPtzAdvanced::on_pushButton_back_clicked()
{
    back();
}
