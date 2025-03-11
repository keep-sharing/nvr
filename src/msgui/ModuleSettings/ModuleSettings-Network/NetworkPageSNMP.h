#ifndef NETWORKPAGESNMP_H
#define NETWORKPAGESNMP_H

#include "AbstractNetworkPage.h"

extern "C" {
#include "msdb.h"
}

namespace Ui {
class NetworkPageSNMP;
}

class NetworkPageSNMP : public AbstractNetworkPage {
    Q_OBJECT

public:
    explicit NetworkPageSNMP(QWidget *parent = nullptr);
    ~NetworkPageSNMP();

    void initNetSnmpTab();
    void gotoNetSnmpTab();
    void freeNetSnmpTab();

    bool saveSetting();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private:
    void snmpSetV1V2cEnable();
    void snmpSetV3Enable();
    void snmpSetSecurity();
    bool isValidatorSnmpInput();
    void saveSnmpConfig();
    void readSnmpConfig();
    void writeSnmpConfig();

    void setReadSnmpVaild();
    void setWriteSnmpValid();

    int is_port_use(int port);
    int is_port_check(int port);

private slots:
    void onLanguageChanged() override;

    void slotSnmpv1Changed(int index);
    void slotSnmpv2cChanged(int index);
    void slotsnmpv3Changed(int index);
    void slotRdLevelOfSecurityChanged(int index);
    void slotWrLevelOfSecurityChanged(int index);

    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

private:
    Ui::NetworkPageSNMP *ui;

    snmp NetSnmpDb;
    snmp NetSnmpDbOri;

    network_more netMore;
    snmp NetSnmpDbOther;

    struct email *pEmailPort = nullptr;
};

#endif // NETWORKPAGESNMP_H
