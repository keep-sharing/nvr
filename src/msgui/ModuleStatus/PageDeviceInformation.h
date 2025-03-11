#ifndef PAGEDEVICEINFORMATION_H
#define PAGEDEVICEINFORMATION_H

#include "AbstractSettingPage.h"

namespace Ui {
class DeviceInformation;
}

extern "C" {
#include "msdb.h"
}
class PageDeviceInformation : public AbstractSettingPage {
    Q_OBJECT

public:
    explicit PageDeviceInformation(QWidget *parent = 0);
    ~PageDeviceInformation();

    static int get_uptime(char *uptime, int size);

    void initializeData() override;

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_SYSINFO(MessageReceive *message);

private slots:
    void on_pushButton_back_clicked();

private:
    Ui::DeviceInformation *ui;
};

#endif // PAGEDEVICEINFORMATION_H
