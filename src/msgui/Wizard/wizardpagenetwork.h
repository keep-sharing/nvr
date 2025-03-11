#ifndef WIZARDPAGENETWORK_H
#define WIZARDPAGENETWORK_H

#include "abstractwizardpage.h"

namespace Ui {
class WizardPageNetwork;
}

class WizardPageNetwork : public AbstractWizardPage {
    Q_OBJECT

public:
    explicit WizardPageNetwork(QWidget *parent = nullptr);
    ~WizardPageNetwork();

    void initializeData() override;
    void saveSetting() override;

    virtual void previousPage() override;
    virtual void nextPage() override;
    virtual void skipWizard() override;

    void processMessage(MessageReceive *message) override;

private:
    int ON_RESPONSE_FLAG_TEST_IP_CONFLICT(MessageReceive *message);
    void ON_RESPONSE_FLAG_IP_CONFLICT_BY_DEV(MessageReceive *message);

    void initNetWizardPage();

    void initializePage();
    bool validatePage();

    void readFromProfileToDlg(int mode);
    void saveCurrentNICDlg(int id);
    void saveBond0Dlg();
    bool isValidatorInput();
    void storeAndApplay();
    int nextOrReboot(char *str, int device);

private slots:
    void onLanguageChanged();
    void workModeComboBoxChangedSlot(int index);
    void selectNIC_comboBoxChangedSlot(int index);
    void enableLAN_checkBoxSlot(bool state);
    void enableDHCP_checkBoxSlot(bool state);

private:
    Ui::WizardPageNetwork *ui;

    struct network *pNetworkDb;
    struct network *pNetworkDbOri;
    int currentNICid;
    int ipConflictCheckType; // 0: bond0 1:lan1 2:lan2
};

#endif // WIZARDPAGENETWORK_H
