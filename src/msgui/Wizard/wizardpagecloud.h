#ifndef WIZARDPAGECLOUD_H
#define WIZARDPAGECLOUD_H

#include "abstractwizardpage.h"

class QLabel;

namespace Ui {
class WizardPageCloud;
}

class WizardPageCloud : public AbstractWizardPage {
    Q_OBJECT

public:
    explicit WizardPageCloud(QWidget *parent = nullptr);
    ~WizardPageCloud();

    void initializeData() override;
    void saveSetting() override;

    void previousPage() override;
    void nextPage() override;

    void processMessage(MessageReceive *message) override;
    void setRegionVisible(bool visible);

protected:
    bool eventFilter(QObject *, QEvent *) override;

private:
    void ON_RESPONSE_FLAG_P2P_STATUS_INFO(MessageReceive *message);
    void ON_RESPONSE_FLAG_P2P_UNBIND_IOT_DEVICE(MessageReceive *message);
    void ON_RESPONSE_FLAG_ENABLE_P2P(MessageReceive *message);
    void ON_RESPONSE_FLAG_DISABLE_P2P(MessageReceive *message);

private slots:
    void onLanguageChanged();

    void on_comboBoxMilesightCloud_activated(int index);
    void on_pushButtonUnbindDevice_clicked();

    void on_comboBoxRegion_activated(int index);

  private:
    Ui::WizardPageCloud *ui;

    QLabel *m_labelStatus = nullptr;
};

#endif // WIZARDPAGECLOUD_H
