#ifndef DEVICELISTWIDGET_H
#define DEVICELISTWIDGET_H

#include <QListWidget>

class DeviceInfoWidget;

class DeviceListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit DeviceListWidget(QWidget *parent = nullptr);

    void addDeviceWidget(DeviceInfoWidget *widget);

signals:

public slots:
};

#endif // DEVICELISTWIDGET_H
