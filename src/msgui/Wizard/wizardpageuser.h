#ifndef WIZARDPAGEUSER_H
#define WIZARDPAGEUSER_H

#include "abstractwizardpage.h"

namespace Ui {
class WizardPageUser;
}

class WizardPageUser : public AbstractWizardPage {
    Q_OBJECT

public:
    explicit WizardPageUser(QWidget *parent = nullptr);
    ~WizardPageUser();

    void initializeData() override;
    void saveSetting() override;

    virtual void previousPage() override;
    virtual void nextPage() override;
    virtual void skipWizard() override;

    void processMessage(MessageReceive *message) override;

private slots:
    void onLanguageChanged();

    void on_checkBox_newPassword_clicked(bool checked);
    void on_comboBox_language_activated(int index);


private:
    Ui::WizardPageUser *ui;
};

#endif // WIZARDPAGEUSER_H
