#ifndef WIZARDPAGEP2P_H
#define WIZARDPAGEP2P_H

#include "abstractwizardpage.h"

extern "C" {
#include "msdb.h"
}

namespace Ui {
class WizardPageP2P;
}

class WizardPageP2P : public AbstractWizardPage {
    Q_OBJECT

public:
    explicit WizardPageP2P(QWidget *parent = 0);
    ~WizardPageP2P();

    void initializeData() override;
    void saveSetting() override;

    virtual void previousPage() override;
    virtual void nextPage() override;
    virtual void skipWizard() override;

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_P2P_STATUS_INFO(MessageReceive *message);
    void ON_RESPONSE_FLAG_ENABLE_P2P(MessageReceive *message);

    void set_P2P_Visible(bool enable);

private slots:
    void onLanguageChanged();

    void on_comboBox_p2p_service_activated(int index);

private:
    Ui::WizardPageP2P *ui;

    p2p_info m_dbP2P;
};

#endif // WIZARDPAGEP2P_H
