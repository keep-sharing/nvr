#ifndef WIZARDPAGECAMERA_H
#define WIZARDPAGECAMERA_H

#include "abstractwizardpage.h"

namespace Ui {
class WizardPageCamera;
}

class WizardPageCamera : public AbstractWizardPage
{
    Q_OBJECT

public:
    explicit WizardPageCamera(QWidget *parent = nullptr);
    ~WizardPageCamera();

    void initializeData() override;
    void saveSetting() override;

    virtual void previousPage() override;
    virtual void nextPage() override;
    virtual void skipWizard() override;

    void processMessage(MessageReceive *message) override;

private:
    Ui::WizardPageCamera *ui;
};

#endif // WIZARDPAGECAMERA_H
