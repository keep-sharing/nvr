#ifndef PAGEDISKMANAGEMENT_H
#define PAGEDISKMANAGEMENT_H

#include "AbstractSettingPage.h"

namespace Ui {
class PageDiskManagement;
}

class PageDiskManagement : public AbstractSettingPage
{
    Q_OBJECT

public:
    explicit PageDiskManagement(QWidget *parent = nullptr);
    ~PageDiskManagement() override;
    void initializeData() override;

    void dealMessage(MessageReceive *message) override;
private slots:
    void onLanguageChanged() override;
    void onTabClicked(int index);

private:
    Ui::PageDiskManagement *ui;
};

#endif // PAGEDISKMANAGEMENT_H
