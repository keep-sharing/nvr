#ifndef MSWIZARD_H
#define MSWIZARD_H

#include "abstractwizardpage.h"
#include <QButtonGroup>
#include <QMap>
#include <QWidget>

extern "C" {
#include "msdb.h"
}

namespace Ui {
class MsWizard;
}

class MsWizard : public QWidget {
    Q_OBJECT

public:
    explicit MsWizard(QWidget *parent = nullptr);
    ~MsWizard();

    static MsWizard *instance();

    void showWizard();
    void finishWizard();
    void skipWizard();
    void next();

    bool isActivatePage();

    void setWizardMode(const WizardMode &mode);
    WizardMode wizardMode() const;
    void showWizardPage(const WizardType &type);

    void dealMessage(MessageReceive *message);

protected:
    void paintEvent(QPaintEvent *) override;
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;

private:
signals:
    void sig_finished();

private slots:
    void onLanguageChanged();

    void onPushButtonBackClicked();
    void onPushButtonNextClicked();
    void onPushButtonSkipClicked();

    void on_checkBox_enable_clicked(bool checked);

    void on_pushButton_test_clearPassword_clicked();
    void on_pushButton_test_clearQuestion_clicked();

private:
    Ui::MsWizard *ui;
    static MsWizard *s_msWizard;

    db_user m_adminUser;
    WizardMode m_mode = ModeActivate;

    WizardType m_currentItemType = Wizard_Activate;
    QMap<WizardType, AbstractWizardPage *> m_pageMap;

    QButtonGroup *m_buttonGroup;
    QButtonGroup *m_activateButtonGroup;
};

#endif // MSWIZARD_H
