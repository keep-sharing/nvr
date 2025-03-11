#ifndef WIZARDPAGEPATTERN_H
#define WIZARDPAGEPATTERN_H

#include "abstractwizardpage.h"

extern "C" {
#include "msdb.h"
}

namespace Ui {
class WizardPagePattern;
}

class WizardPagePattern : public AbstractWizardPage {
    Q_OBJECT

public:
    explicit WizardPagePattern(QWidget *parent = 0);
    ~WizardPagePattern();

    void initializeData() override;
    void saveSetting() override;

    virtual void previousPage() override;
    virtual void nextPage() override;
    virtual void skipWizard() override;

    void processMessage(MessageReceive *message) override;

private slots:
    void onDrawFinished(QString text);
    void onDrawStart();
    void on_comboBox_patternEnable_activated(int index);

private:
    Ui::WizardPagePattern *ui;
    QString m_pattern_text;
    db_user m_adminUser;
    int step = 1;
};

#endif // WIZARDPAGEPATTERN_H
