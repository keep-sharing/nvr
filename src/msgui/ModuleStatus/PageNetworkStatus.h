#ifndef PAGENETWORKSTATUS_H
#define PAGENETWORKSTATUS_H

#include "AbstractSettingPage.h"

namespace Ui {
class NetworkStatus;
}

class PageNetworkStatus : public AbstractSettingPage
{
    Q_OBJECT

public:
    explicit PageNetworkStatus(QWidget *parent = 0);
    ~PageNetworkStatus();

    void initializeData() override;

    bool canAutoLogout() override;

    void dealMessage(MessageReceive *message) override;

private slots:
    void onLanguageChanged();
    void onTabClicked(int index);

private:
    Ui::NetworkStatus *ui;
};

#endif // PAGENETWORKSTATUS_H
