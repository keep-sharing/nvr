#ifndef STORAGEMODE_H
#define STORAGEMODE_H

#include "AbstractSettingPage.h"

class AbstractSettingTab;

namespace Ui {
class StorageMode;
}

class StorageMode : public AbstractSettingPage
{
    Q_OBJECT

    enum SettingTab
    {
        TabNone,
        TabQuota,
        TabGroup
    };

public:
    explicit StorageMode(QWidget *parent = 0);
    ~StorageMode();

    void initializeData() override;

private slots:
    void onTabClicked(int index);

private:
    Ui::StorageMode *ui;
};

#endif // STORAGEMODE_H
