#ifndef WIZARDPAGETIME_H
#define WIZARDPAGETIME_H

#include "abstractwizardpage.h"

extern "C" {
#include "msdb.h"
}

namespace Ui {
class WizardPageTime;
}

class WizardPageTime : public AbstractWizardPage {
    Q_OBJECT

public:
    explicit WizardPageTime(QWidget *parent = nullptr);
    ~WizardPageTime();

    void initializeData() override;
    void saveSetting() override;

    virtual void previousPage() override;
    virtual void nextPage() override;
    virtual void skipWizard() override;

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_SET_SYSTIME(MessageReceive *message);

private slots:
    void onLanguageChanged();

    void on_checkBox_synchronize_clicked(bool checked);
    void on_checkBox_manually_clicked(bool checked);

private:
    Ui::WizardPageTime *ui;

    struct time m_dbTime;
};

#endif // WIZARDPAGETIME_H
