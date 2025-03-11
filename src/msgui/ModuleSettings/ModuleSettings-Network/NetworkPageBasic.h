#ifndef NETWORKPAGEBASIC_H
#define NETWORKPAGEBASIC_H

#include "AbstractNetworkPage.h"

class RouterEdit;

namespace Ui {
class NetworkPageBasic;
}

class NetworkPageBasic : public AbstractNetworkPage {
    Q_OBJECT

public:
    explicit NetworkPageBasic(QWidget *parent = nullptr);
    ~NetworkPageBasic();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_TEST_IP_CONFLICT(MessageReceive *message);

    void initNetBasicTab();
    void gotoNetBasicTab();
    void freeNetBasicTab();

    void readBasicConfig(int mode);
    void saveBasicConfig();

    void writeBasicConfig(int mode);
    int isHotspareWorking(int mode);

    bool isValidatorInput();
    void getNetBasicInfo(void *data);
    int dealConfirmLoadBalance(int mode);

private slots:
    void onLanguageChanged();

    void onComboBoxWorkingModeChanged(int index);
    void slotBond0Eth0Enable(bool state);
    void slotEth1Enable(bool state);
    void slotBond0Eth0Type(int state);
    void slotEth1Type(int state);
    void slotBondPrimaryComboBoxChanged(int index);

    void slotEth0IPv6ModeChanged(int index);
    void slotEth1IPv6ModeChanged(int index);
    void slotShowEth0Details();
    void slotShowEth1Details();

    void onPushButtonApplyClicked();
    void onPushButtonBackClicked();

private:
    Ui::NetworkPageBasic *ui;

    struct network *pNetworkDb = nullptr;
    struct network *pNetworkDbOri = nullptr;
    struct network *pNetworkDbStatic = nullptr;
    int _ip_conflict_type; // 0: bond0 1:lan1 2:lan2
    RouterEdit *pRouterAdvDialog;
    int _ipConflict;

    int failoverMode = 0;
};

#endif // NETWORKPAGEBASIC_H
