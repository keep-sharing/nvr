#ifndef NETWORKPAGECLOUD_H
#define NETWORKPAGECLOUD_H

#include "AbstractNetworkPage.h"

namespace Ui {
class NetworkPageCloud;
}

class NetworkPageCloud : public AbstractNetworkPage {
    Q_OBJECT

public:
    explicit NetworkPageCloud(QWidget *parent = nullptr);
    ~NetworkPageCloud();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_P2P_STATUS_INFO(MessageReceive *message);
    void ON_RESPONSE_FLAG_P2P_UNBIND_IOT_DEVICE(MessageReceive *message);
    void ON_RESPONSE_FLAG_ENABLE_P2P(MessageReceive *message);
    void ON_RESPONSE_FLAG_DISABLE_P2P(MessageReceive *message);

private slots:
    void onLanguageChanged() override;

    void on_comboBoxMilesightCloud_indexSet(int index);
    void on_toolButtonMilesightCloudStatus_clicked();
    void on_pushButtonUnbindDevice_clicked();
    void on_pushButtonApply_clicked();
    void on_pushButtonBack_clicked();

private:
    Ui::NetworkPageCloud *ui;

};

#endif // NETWORKPAGECLOUD_H
