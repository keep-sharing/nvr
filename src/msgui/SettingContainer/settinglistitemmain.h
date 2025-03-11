#ifndef SETTINGLISTITEMMAIN_H
#define SETTINGLISTITEMMAIN_H

#include "settinglistitem.h"

class SettingListItemMain : public SettingListItem
{
    Q_OBJECT
public:
    explicit SettingListItemMain(const SettingItemInfo &info, QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *event) override;

signals:

public slots:

private:

};

#endif // SETTINGLISTITEMMAIN_H
