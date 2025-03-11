#ifndef NETWORKPAGEEMAIL_H
#define NETWORKPAGEEMAIL_H

#include "AbstractNetworkPage.h"

namespace Ui {
class NetworkPageEmail;
}

class NetworkPageEmail : public AbstractNetworkPage {
    Q_OBJECT

public:
    explicit NetworkPageEmail(QWidget *parent = nullptr);
    ~NetworkPageEmail();

    void initializeData() override;
    void processMessage(MessageReceive *message);

private:
    void ON_RESPONSE_FLAG_TEST_MAIL(MessageReceive *message);

    void initNetMailTab();
    void gotoNetMailTab();
    void freeNetMailTab();

    bool checkMail(const char *pszEmail);
    void readMailConfig();
    void saveMailConfig();
    bool isValidatorMailInput();

private slots:
    void onLanguageChanged();

    void slotEailEnableAttach(bool state);
    void on_checkBox_enableTls_clicked(bool checked);
    void on_checkBox_enableSsl_clicked(bool checked);
    void slotMailTestBtn();
    void slotMailApplyBtn();
    void slotMailBackBtn();

    void on_comboBox_mailEnable_indexSet(int index);
    void on_comboBox_mailEnable_activated(int index);

    void on_checkBox_enableHostname_toggled(bool checked);

private:
    Ui::NetworkPageEmail *ui;

    struct email *pEmailDb;
    struct email *pEmailDbOri;
    struct network_more *pMailNetMoreDb;
    struct network_more *pMailNetMoreDbOri;
    int _cur_receiver_index;

    struct snmp *NetSnmpPort = nullptr;
};

#endif // NETWORKPAGEEMAIL_H
