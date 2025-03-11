#ifndef NETWORKPAGEDDNS_H
#define NETWORKPAGEDDNS_H

#include "AbstractNetworkPage.h"

namespace Ui {
class NetworkPageDDNS;
}

class NetworkPageDDNS : public AbstractNetworkPage {
    Q_OBJECT

public:
    explicit NetworkPageDDNS(QWidget *parent = nullptr);
    ~NetworkPageDDNS();

    void initializeData() override;

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_DDNS_STATUS(MessageReceive *message);

    void initNetDDNSTab();
    void gotoNetDDNStab();
    void freeNetDDNStab();
    void readDDNSConfig();
    void saveDDNSConfig();
    bool isValidatorDDNSInput();
    QString getMACStr(char *mac);

private slots:
    void onLanguageChanged() override;

    void slotDdnsEnableChanged(int index);
    void slotDdnsUseUPnP(int index);
    void slotDdnsDomainChanged(int index);

    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

private:
    Ui::NetworkPageDDNS *ui;

    struct ddns *pDDNS_Db;
    struct ddns *pDDNS_DbOri;
};

#endif // NETWORKPAGEDDNS_H
