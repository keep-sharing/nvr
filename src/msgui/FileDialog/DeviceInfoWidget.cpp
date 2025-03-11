#include "DeviceInfoWidget.h"
#include "ui_DeviceInfoWidget.h"
#include <QProcess>
#include <QtDebug>

DeviceInfoWidget::DeviceInfoWidget(const resp_usb_info &usb_info, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeviceInfoWidget)
{
    ui->setupUi(this);

    memcpy(&m_usb_info, &usb_info, sizeof(resp_usb_info));

//    m_process = new QProcess(this);
//    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onProcessFinished(int,QProcess::ExitStatus)));
}

DeviceInfoWidget::~DeviceInfoWidget()
{
    delete ui;
}

resp_usb_info DeviceInfoWidget::usbInfo() const
{
    return m_usb_info;
}

int DeviceInfoWidget::port() const
{
    return m_usb_info.port;
}

QString DeviceInfoWidget::deviceName() const
{
    return m_deviceName;
}

void DeviceInfoWidget::setDeviceName(const QString &name)
{
    m_deviceName = name.trimmed();
    ui->label_name->setText(m_deviceName);
}

QString DeviceInfoWidget::devicePath() const
{
    return m_devicePath;
}

void DeviceInfoWidget::setDevicePath(const QString &path)
{
    ui->progressBar->setValue(0);
    ui->label_capacity->setText(QString("Free: --, Total:--"));
    //
    m_devicePath = path.trimmed();

    //m_process->start(QString("df -h %1").arg(path));

    qDebug() << QString("DeviceInfoWidget::setDevicePath, %1").arg(path);
    qDebug() << QString("DeviceInfoWidget::setDevicePath, get usb info begin.");
    char info[256] = {0};
    FILE *fp = ms_vpopen(QString("df -h %1").arg(path).toStdString().c_str(), "r");
    if (!fp)
    {
        qDebug() << QString("DeviceInfoWidget::setDevicePath, get usb info error.");
        return;
    }
    fread(info, sizeof(char), sizeof(info), fp);
    ms_vpclose(fp);
    qDebug() << QString("DeviceInfoWidget::setDevicePath, get usb info end.");

    QStringList strList = QString(info).split(" ", QString::SkipEmptyParts);
    if (strList.size() == 12)
    {
        QString strTotal = strList.at(7);
        QString strFree = strList.at(9);
        QString strPercent = strList.at(10);
        strPercent.chop(1);
        ui->progressBar->setValue(strPercent.toInt());
        ui->label_capacity->setText(QString("Free: %1, Total: %2").arg(strFree).arg(strTotal));
    }
}

void DeviceInfoWidget::setIsProtected(bool protect)
{
    m_isProtected = protect;
}

bool DeviceInfoWidget::isProtected() const
{
    return m_isProtected;
}

void DeviceInfoWidget::setIsFormated(bool format)
{
    m_isFormated = format;
}

bool DeviceInfoWidget::isFormated() const
{
    return m_isFormated;
}

void DeviceInfoWidget::setFreeBytes(qint64 bytes)
{
    m_freeBytes = bytes;
}

qint64 DeviceInfoWidget::freeBytes() const
{
    return m_freeBytes;
}

void DeviceInfoWidget::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(exitStatus)

    QString outText = m_process->readAllStandardOutput();
    QStringList strList = outText.split(" ", QString::SkipEmptyParts);
    if (strList.size() == 12)
    {
        QString strTotal = strList.at(7);
        QString strFree = strList.at(9);
        QString strPercent = strList.at(10);
        strPercent.chop(1);
        ui->progressBar->setValue(strPercent.toInt());
        ui->label_capacity->setText(QString("Free: %1, Total: %2").arg(strFree).arg(strTotal));
    }
}
