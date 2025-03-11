#ifndef SETTINGLISTITEMSUB_H
#define SETTINGLISTITEMSUB_H

#include "settinglistitem.h"

class SettingListItemSub : public SettingListItem
{
    Q_OBJECT
public:
    explicit SettingListItemSub(const SettingItemInfo &info, QWidget *parent = 0);

signals:

public slots:

};

#endif // SETTINGLISTITEMSUB_H
