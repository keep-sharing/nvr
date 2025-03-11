#ifndef NETWORKPAGEMORE_H
#define NETWORKPAGEMORE_H

#include "AbstractNetworkPage.h"
#include <QMap>

extern "C" {
#include "msdb.h"
}

namespace Ui {
class NetworkPageMore;
}

class NetworkPageMore : public AbstractNetworkPage {
    Q_OBJECT

public:
    explicit NetworkPageMore(QWidget *parent = nullptr);
    ~NetworkPageMore();

    void initNetMoreTab();
    void gotoNetMoreTab();
    void freeNetMoreTab();

    void readMoreConfig();
    void saveMoreConfig();
    bool isValidatorMoreInput();

    bool saveSetting();

    void initializeData() override;

private:
    bool checkPort(QString type, int default_port, int now, int min, int max);
    int is_port_use(int port);
    int is_port_check(int port);
    bool isBrowserRestrictions(int port);

private slots:
    void onLanguageChanged() override;

    void slotSshEnableChanged(int index);

    void on_pushButton_pushType_clicked();

    void on_pushButton_apply_clicked();
    void on_pushButton_back_clicked();

private:
    Ui::NetworkPageMore *ui;

    network_more *pNetMoreDb = nullptr;
    network_more *pNetMoreDbOri = nullptr;
    network_more *pNetMoreDbOther = nullptr;
    snmp *NetSnmpPort = nullptr;
    struct email *pEmailPort = nullptr;
    int NVRAlarmInType;
    QMap<int, int> IPCPushType;

    PushMsgNvrEvent m_pushMsgNvrEvtDb;
};

#endif // NETWORKPAGEMORE_H
