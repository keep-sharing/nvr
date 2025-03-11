#ifndef WIZARDPAGEQUESTION_H
#define WIZARDPAGEQUESTION_H

#include "abstractwizardpage.h"

#include <QWidget>

namespace Ui {
class WizardPageQuestion;
}

class WizardPageQuestion : public AbstractWizardPage {
    Q_OBJECT

public:
    explicit WizardPageQuestion(QWidget *parent = nullptr);
    ~WizardPageQuestion();

    void initializeData() override;
    void saveSetting() override;

    virtual void previousPage();
    virtual void nextPage();
    virtual void skipWizard();

    void processMessage(MessageReceive *message) override;

private:
    void saveQuestion();

private slots:
    void onLanguageChanged();

private:
    Ui::WizardPageQuestion *ui;
};

#endif // WIZARDPAGEQUESTION_H
