#ifndef WIZARDPAGERECORD_H
#define WIZARDPAGERECORD_H

#include "abstractwizardpage.h"

namespace Ui {
class WizardPageRecord;
}

class WizardPageRecord : public AbstractWizardPage {
    Q_OBJECT

public:
    explicit WizardPageRecord(QWidget *parent = nullptr);
    ~WizardPageRecord();

    void initializeData() override;
    void saveSetting() override;

    virtual void previousPage() override;
    virtual void nextPage() override;
    virtual void skipWizard() override;

    void processMessage(MessageReceive *message) override;

private slots:
    void onLanguageChanged();

private:
    Ui::WizardPageRecord *ui;
};

#endif // WIZARDPAGERECORD_H
