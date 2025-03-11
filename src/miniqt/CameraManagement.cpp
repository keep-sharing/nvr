#include "CameraManagement.h"
#include "ui_CameraManagement.h"

#include "CameraInfoManagement.h"
#include "CameraInfo.h"
#include <QtDebug>
#include <QTextStream>
#include <QFile>
#include <QTimer>

#define SETTING_PATH "/mnt/mtd/settings.ini"
#define CAMERALIST_PATH "/mnt/mtd/CameraList.txt"

CameraManagement::CameraManagement(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CameraManagement)
{
    ui->setupUi(this);

    m_settings = new QSettings(SETTING_PATH, QSettings::IniFormat, this);

    ui->lineEditIP->setText(m_settings->value("IP").toString());
    ui->lineEditUser->setText(m_settings->value("User").toString());
    ui->lineEditPassword->setText(m_settings->value("Password").toString());
    ui->comboBoxProtocal->setCurrentIndex(ui->comboBoxProtocal->findText(m_settings->value("Protocal").toString()));
    ui->comboBoxCodec->setCurrentIndex(ui->comboBoxProtocal->findText(m_settings->value("Codec").toString()));
    ui->comboBoxFrameSize->setCurrentIndex(ui->comboBoxFrameSize->findText(m_settings->value("FrameSize").toString()));

    m_cameraModel = new CameraModel(this);
    connect(&CameraInfoManagement::instance(), SIGNAL(dataChanged(int)), m_cameraModel, SLOT(updateModel(int)));

    ui->comboBoxProtocal->clear();
    ui->comboBoxProtocal->addItem("UDP", TRANSPROTOCOL_UDP);
    ui->comboBoxProtocal->addItem("TCP", TRANSPROTOCOL_TCP);

    ui->comboBoxCodec->clear();
    ui->comboBoxCodec->addItem("H264", ENC_TYPE_H264);
    ui->comboBoxCodec->addItem("H265", ENC_TYPE_H265);

    ui->comboBoxFrameSize->clear();
    ui->comboBoxFrameSize->addItem("704*576");
    ui->comboBoxFrameSize->addItem("1280*720");
    ui->comboBoxFrameSize->addItem("1280*960");
    ui->comboBoxFrameSize->addItem("1920*1080");
    ui->comboBoxFrameSize->addItem("2048*1536");
    ui->comboBoxFrameSize->addItem("2592*1520");
    ui->comboBoxFrameSize->addItem("2592*1944");
    ui->comboBoxFrameSize->addItem("3072*2048");
    ui->comboBoxFrameSize->addItem("3840*2160");
    ui->comboBoxFrameSize->addItem("4000*3000");

    ui->tableView->setFocus();
    ui->tableView->setModel(m_cameraModel);

    ui->tableView->setColumnWidth(CameraModel::ColumnIP, 150);
    ui->tableView->setColumnWidth(CameraModel::ColumnState, 200);
    ui->tableView->setColumnWidth(CameraModel::ColumnRTSP, 200);

    connect(ui->tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(onTableViewClicked(QModelIndex)));

    QTimer::singleShot(1000, this, SLOT(readCameraList()));
}

CameraManagement::~CameraManagement()
{
    delete ui;
}

void CameraManagement::showEvent(QShowEvent *event)
{
    m_cameraModel->resetModel();

    QDialog::showEvent(event);
}

void CameraManagement::saveCameraList()
{
    QFile file(CAMERALIST_PATH);
    if (!file.open(QFile::WriteOnly)) {
        return;
    }
    QTextStream stream(&file);
    for (int i = 0; i < 64; ++i) {
        auto *cameraInfo = CameraInfo::fromChannel(i);
        if (cameraInfo->isValid()) {
            stream << cameraInfo->channel() << ",";
            stream << cameraInfo->ip() << ",";
            stream << cameraInfo->user() << ",";
            stream << cameraInfo->password() << ",";
            stream << cameraInfo->protocolString() << ",";
            stream << cameraInfo->codecString() << ",";
            stream << cameraInfo->frameSize();
            stream << "\r\n";
        }
    }
}

void CameraManagement::readCameraList()
{
    QFile file(CAMERALIST_PATH);
    if (!file.open(QFile::ReadOnly)) {
        return;
    }
    QTextStream stream(&file);
    QString line;
    do {
        line = stream.readLine();
        QStringList list = line.split(",");
        if (list.size() == 7) {
            qDebug() << list;
            int channel = list.at(0).toInt();
            QString ip = list.at(1);
            QString user = list.at(2);
            QString password = list.at(3);
            QString protocol = list.at(4);
            QString codec = list.at(5);
            QString frameSize = list.at(6);
            CameraInfo *info = CameraInfo::fromChannel(channel);
            info->setIp(ip);
            info->setUser(user);
            info->setPassword(password);
            if (protocol == "UDP") {
                info->setProtocol(TRANSPROTOCOL_UDP);
            } else {
                info->setProtocol(TRANSPROTOCOL_TCP);
            }
            if (codec == "H264") {
                info->setCodec(ENC_TYPE_H264);
            } else {
                info->setCodec(ENC_TYPE_H265);
            }
            info->setFrameSize(frameSize);
            info->createRtspClient();
            info->createRtspServer();
            m_channelMap.insert(channel, 1);
        }
    } while (!line.isNull());

    m_cameraModel->resetModel();
}

void CameraManagement::on_pushButtonAdd_clicked()
{
    if (ui->lineEditIP->text().isEmpty()) {
        return;
    }
    if (ui->lineEditUser->text().isEmpty()) {
        return;
    }
    if (ui->lineEditPassword->text().isEmpty()) {
        return;
    }
    if (ui->comboBoxProtocal->currentIndex() < 0) {
        return;
    }
    if (ui->comboBoxCodec->currentIndex() < 0) {
        return;
    }
    if (ui->comboBoxFrameSize->currentIndex() < 0) {
        return;
    }

    m_settings->setValue("IP", ui->lineEditIP->text());
    m_settings->setValue("User", ui->lineEditUser->text());
    m_settings->setValue("Password", ui->lineEditPassword->text());
    m_settings->setValue("Protocal", ui->comboBoxProtocal->currentText());
    m_settings->setValue("Codec", ui->comboBoxCodec->currentText());
    m_settings->setValue("FrameSize", ui->comboBoxFrameSize->currentText());

    int channel = -1;
    for (int i = 0; i < 64; ++i) {
        if (!m_channelMap.contains(i) || m_channelMap.value(i) != 1) {
            m_channelMap.insert(i, 1);
            channel = i;
            break;
        }
    }
    if (channel < 0 || channel > 63) {
        return;
    }

    qDebug() << "Add channel:" << channel;

    CameraInfo *info = CameraInfo::fromChannel(channel);
    info->setIp(ui->lineEditIP->text());
    info->setUser(ui->lineEditUser->text());
    info->setPassword(ui->lineEditPassword->text());
    info->setProtocol(ui->comboBoxProtocal->itemData(ui->comboBoxProtocal->currentIndex()).toInt());
    info->setCodec(ui->comboBoxCodec->itemData(ui->comboBoxCodec->currentIndex()).toInt());
    info->setFrameSize(ui->comboBoxFrameSize->currentText());
    info->createRtspClient();
    info->createRtspServer();

    m_cameraModel->resetModel();
    saveCameraList();
}

void CameraManagement::onTableViewClicked(const QModelIndex &index)
{
    qDebug() << "Table clicked:" << index;
    if (!index.isValid()) {
        return;
    }
    int channel = m_cameraModel->itemText(index.row(), CameraModel::ColumnChannel).toInt() - 1;
    qDebug() << "Remove channel:" << channel;
    if (channel < 0 || channel > 63) {
        return;
    }

    if (index.column() == CameraModel::ColumnDelete) {
        CameraInfo *info = CameraInfo::fromChannel(channel);
        info->deleteRtspClient();
        info->deleteRtspServer();
        info->clear();

        m_channelMap.insert(channel, 0);

        m_cameraModel->resetModel();
        saveCameraList();
    }
}

