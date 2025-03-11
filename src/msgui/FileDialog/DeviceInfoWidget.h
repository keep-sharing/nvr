#ifndef DEVICEINFOWIDGET_H
#define DEVICEINFOWIDGET_H

#include <QProcess>
#include <QWidget>

extern "C" {
#include "msg.h"
}

namespace Ui {
class DeviceInfoWidget;
}

class DeviceInfoWidget : public QWidget {
    Q_OBJECT

public:
    explicit DeviceInfoWidget(const resp_usb_info &usb_info, QWidget *parent = nullptr);
    ~DeviceInfoWidget();

    resp_usb_info usbInfo() const;

    int port() const;
    QString deviceName() const;
    void setDeviceName(const QString &name);
    QString devicePath() const;
    void setDevicePath(const QString &path);
    void setIsProtected(bool protect);
    bool isProtected() const;
    void setIsFormated(bool format);
    bool isFormated() const;
    void setFreeBytes(qint64 bytes);
    qint64 freeBytes() const;

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    Ui::DeviceInfoWidget *ui;

    resp_usb_info m_usb_info;

    bool m_isProtected = false;
    bool m_isFormated = true;
    QString m_deviceName;
    QString m_devicePath;
    qint64 m_freeBytes = 0;

    QProcess *m_process = nullptr;
};

#endif // DEVICEINFOWIDGET_H
