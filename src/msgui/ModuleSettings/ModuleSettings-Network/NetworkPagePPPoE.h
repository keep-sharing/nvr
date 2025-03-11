#ifndef NETWORKPAGEPPPOE_H
#define NETWORKPAGEPPPOE_H

#include "AbstractNetworkPage.h"

extern "C" {
#include "msdb.h"
}

namespace Ui {
class NetworkPagePPPoe;
}

class NetworkPagePPPoe : public AbstractNetworkPage {
    Q_OBJECT

public:
    explicit NetworkPagePPPoe(QWidget *parent = nullptr);
    ~NetworkPagePPPoe();

    void initializeData() override;
    void processMessage(MessageReceive *message);

private:
    void initNetPPPoETab();
    void gotoNetPPPoETab();
    bool isValidatorPPPoEInput();

private slots:
    void onLanguageChanged();
    void on_pushButton_apply_clicked();
    void on_comboBox_pppoe_activated(int index);
    void onGetDynamicIp();
    void on_pushButton_back_clicked();

private:
    Ui::NetworkPagePPPoe *ui;
    struct pppoe m_dbPppoe;
};

#endif // NETWORKPAGEPPPOE_H
