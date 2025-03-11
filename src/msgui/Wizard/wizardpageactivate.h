#ifndef WIZARDPAGEACTIVATE_H
#define WIZARDPAGEACTIVATE_H

#include "abstractwizardpage.h"
#include <QWidget>

extern "C" {
#include "msdb.h"
}

namespace Ui {
class WizardPageActivate;
}

class WizardPageActivate : public AbstractWizardPage {
    Q_OBJECT

public:
    explicit WizardPageActivate(QWidget *parent = nullptr);
    ~WizardPageActivate();

    void initializeData() override;
    void saveSetting() override;

    virtual void previousPage() override;
    virtual void nextPage() override;
    virtual void skipWizard() override;

    void processMessage(MessageReceive *message) override;

private slots:
    void onLanguageChanged();

    void on_comboBox_language_activated(int index);

private:
    Ui::WizardPageActivate *ui;

    db_user m_adminUser;
};

#endif // WIZARDPAGEACTIVATE_H
