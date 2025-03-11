#ifndef ABSTRACTWIZARDPAGE_H
#define ABSTRACTWIZARDPAGE_H

#include "MsWidget.h"

class MessageReceive;
class MsWaitting;

enum WizardType {
    Wizard_Activate,
    wizard_Pattern,
    Wizard_Question,
    Wizard_User,
    Wizard_Time,
    Wizard_NetWork,
    Wizard_Disk,
    Wizard_Camera,
    Wizard_P2P,
    Wizard_Record,
    Wizard_None
};

enum WizardMode {
    ModeActivate,
    ModeNormal
};

class AbstractWizardPage : public MsWidget {
    Q_OBJECT
public:
    explicit AbstractWizardPage(QWidget *parent = nullptr);

    virtual void initializeData() = 0;
    virtual void saveSetting() = 0;

    virtual void previousPage();
    virtual void nextPage();
    virtual void skipWizard();

protected:
    void setWizardMode(const WizardMode &mode);
    WizardMode wizardMode() const;
    void showWizardPage(const WizardType &type);
    void finishWizard();

signals:

public slots:

protected:
    MsWaitting *m_waitting = nullptr;
};

#endif // ABSTRACTWIZARDPAGE_H
