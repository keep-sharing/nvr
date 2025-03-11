#ifndef PAGEDISKSTATUS_H
#define PAGEDISKSTATUS_H

#include "AbstractSettingPage.h"

namespace Ui {
class DiskStatus;
}

class PageDiskStatus : public AbstractSettingPage
{
    Q_OBJECT

public:
    explicit PageDiskStatus(QWidget *parent = nullptr);
    ~PageDiskStatus() override;

    void initializeData() override;

    bool canAutoLogout() override;

    void dealMessage(MessageReceive *message) override;

private slots:
    void onLanguageChanged() override;

    void onTabClicked(int index);

    void on_pushButton_refresh_clicked();
    void on_pushButton_back_clicked();

    void on_pushButtonSave_clicked();

  private:
    Ui::DiskStatus *ui;
};

#endif // PAGEDISKSTATUS_H
