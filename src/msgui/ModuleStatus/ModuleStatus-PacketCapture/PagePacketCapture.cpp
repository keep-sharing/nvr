#include "PagePacketCapture.h"
#include "ui_PagePacketCapture.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "MyFileSystemDialog.h"
#include "myqt.h"
#include "PacketCaptureData.h"
#include <QDir>

PagePacketCapture::PagePacketCapture(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::PacketCapture)
{
    ui->setupUi(this);

    struct network netInfo;
    memset(&netInfo, 0, sizeof(struct network));
    read_network(SQLITE_FILE_NAME, &netInfo);

    ui->comboBoxNic->clear();

    const struct device_info &sys_info = qMsNvr->deviceInfo();
    switch (netInfo.mode) {
    case NETMODE_MULTI:
        if (sys_info.max_lan == 1) {
            ui->comboBoxNic->addItem("eth0", "eth0");
        } else {
            ui->comboBoxNic->addItem("eth0", "eth0");
            ui->comboBoxNic->addItem("eth1", "eth1");
        }
        break;
    case NETMODE_LOADBALANCE:
        ui->comboBoxNic->addItem("Bond0", "bond0");
        break;
    case NETMODE_BACKUP:
        ui->comboBoxNic->addItem("Bond0", "bond0");
        break;
    }

    QRegExp rx("\\d+|^$");
    QRegExpValidator *validator = new QRegExpValidator(rx, this);
    ui->lineEditPort->setValidator(validator);

    const device_info &info = qMsNvr->deviceInfo();
    if (info.max_lan == 1) {
        ui->labelNic->hide();
        ui->comboBoxNic->hide();
    }

    PacketCaptureData *capture = PacketCaptureData::instance();
    if (capture) {
        connect(capture, SIGNAL(started()), this, SLOT(onStarted()));
        connect(capture, SIGNAL(finished()), this, SLOT(onFinished()));
    }

    ui->lineEditIP->setCheckMode(MyLineEdit::IPv4CanEmptyCheck);
    ui->lineEditPort->setCheckMode(MyLineEdit::RangeCanEmptyCheck, 1, 65535);
    ui->lineEditExportDirectory->setCheckMode(MyLineEdit::EmptyCheck);

    onLanguageChanged();
}

PagePacketCapture::~PagePacketCapture()
{
    delete ui;
}

void PagePacketCapture::initializeData()
{
    updateFormData();
    updateFormEnable();
}

void PagePacketCapture::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void PagePacketCapture::onLanguageChanged()
{
    ui->labelIp->setText(GET_TEXT("PACKETCAPTURE/109001", "IP"));
    ui->labelPort->setText(GET_TEXT("PACKETCAPTURE/109002", "Port"));
    ui->labelNic->setText(GET_TEXT("PACKETCAPTURE/109007", "NIC"));
    ui->labelExportDirectory->setText(GET_TEXT("PACKETCAPTURE/109003", "Export Directory"));
    ui->pushButtonBrowse->setText(GET_TEXT("CAMERAMAINTENANCE/38004", "Browse"));
    ui->pushButtonStart->setText(GET_TEXT("PACKETCAPTURE/109008", "Start"));
    ui->pushButtonEnd->setText(GET_TEXT("PACKETCAPTURE/109009", "End"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
}

void PagePacketCapture::onStarted()
{
    m_eventLoop.exit();
}

void PagePacketCapture::onFinished()
{
    //MsWaitting::closeGlobalWait();

    PacketCaptureData *capture = PacketCaptureData::instance();
    if (capture) {
        capture->stopCapture();
        capture->stopThread();
        delete capture;
    }

    updateFormEnable();
}

void PagePacketCapture::on_pushButtonBrowse_clicked()
{
    ui->pushButtonBrowse->clearUnderMouse();

    QString path = MyFileSystemDialog::instance()->getOpenDirectory();
    if (path.isEmpty()) {
        return;
    }
    ui->lineEditExportDirectory->setText(path);
    m_diskPort = MyFileSystemDialog::instance()->currentDevicePort();
}

void PagePacketCapture::on_pushButtonStart_clicked()
{
    ui->pushButtonStart->clearUnderMouse();
    ui->pushButtonStart->clearFocus();

    QString ip = ui->lineEditIP->text();
    bool valid = ui->lineEditIP->checkValid();
    valid = ui->lineEditPort->checkValid() && valid;
    valid = ui->lineEditExportDirectory->checkValid() && valid;
    if (!valid) {
        return;
    }

    QString port = ui->lineEditPort->text();
    QString nic = ui->comboBoxNic->currentData().toString();
    QString path = ui->lineEditExportDirectory->text();

    //MsWaitting::showGlobalWait();

    PacketCaptureData *capture = PacketCaptureData::instance();
    if (!capture) {
        capture = new PacketCaptureData();
        connect(capture, SIGNAL(started()), this, SLOT(onStarted()));
        connect(capture, SIGNAL(finished()), this, SLOT(onFinished()));
    }
    capture->setIp(ip);
    capture->setPort(port);
    capture->setNic(nic);
    capture->setPath(path);
    capture->startCapture(m_diskPort);
    //m_eventLoop.exec();

    //MsWaitting::closeGlobalWait();

    //updateFormEnable();
}

void PagePacketCapture::on_pushButtonEnd_clicked()
{
    ui->pushButtonEnd->clearUnderMouse();
    ui->pushButtonEnd->clearFocus();

    PacketCaptureData *capture = PacketCaptureData::instance();
    if (capture) {
        //MsWaitting::showGlobalWait();
        capture->stopCapture();
    }
}

void PagePacketCapture::on_pushButtonBack_clicked()
{
    back();
}

void PagePacketCapture::updateFormData()
{
    PacketCaptureData *capture = PacketCaptureData::instance();

    if (capture && capture->state() == PacketCaptureData::StateWorking) {
        ui->comboBoxNic->setCurrentIndexFromData(capture->nic());
        if (ui->comboBoxNic->currentIndex() < 0) {
            ui->comboBoxNic->setCurrentIndex(0);
        }
        ui->lineEditIP->setText(capture->ip());
        ui->lineEditPort->setText(capture->port());
        ui->lineEditExportDirectory->setText(capture->path());
    } else {
        ui->comboBoxNic->setCurrentIndex(0);
        ui->lineEditIP->clear();
        ui->lineEditPort->clear();
        ui->lineEditExportDirectory->clear();
    }
}

void PagePacketCapture::updateFormEnable()
{
    PacketCaptureData *capture = PacketCaptureData::instance();

    if (capture && capture->state() == PacketCaptureData::StateWorking) {
        ui->lineEditIP->setEnabled(false);
        ui->lineEditPort->setEnabled(false);
        ui->comboBoxNic->setEnabled(false);
        ui->pushButtonBrowse->setEnabled(false);
        ui->pushButtonStart->setEnabled(false);
        ui->pushButtonEnd->setEnabled(true);
    } else {
        ui->lineEditIP->setEnabled(true);
        ui->lineEditPort->setEnabled(true);
        ui->comboBoxNic->setEnabled(true);
        ui->pushButtonBrowse->setEnabled(true);
        ui->pushButtonStart->setEnabled(true);
        ui->pushButtonEnd->setEnabled(false);
    }
}
