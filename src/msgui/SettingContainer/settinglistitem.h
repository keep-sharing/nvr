#ifndef SETTINGLISTITEM_H
#define SETTINGLISTITEM_H

#include <QPushButton>

struct SettingItemInfo
{
    QString name;
    int id = -1;
    int permission = -1;
    int mode = 0;
    bool visible = true;

    SettingItemInfo()
    {

    }
    SettingItemInfo(const QString &item_name, int item_id, int item_mode, int item_permission = -1, bool item_visible = true)
    {
        name = item_name;
        id = item_id;
        mode = item_mode;
        permission = item_permission;
        visible = item_visible;
    }

    QList<SettingItemInfo> subItems;
};
QDebug operator<<(QDebug dbg, const SettingItemInfo &info);

class SettingListItem : public QPushButton
{
    Q_OBJECT

public:
    explicit SettingListItem(const SettingItemInfo &info, QWidget *parent = nullptr);

    SettingItemInfo itemInfo() const;
    void updateText(const QString &text);

    void clearUnderMouse();

    bool isOpen() const;
    bool isItemMain() const;
    bool isItemSub() const;

    bool isItemVisible() const;

    void appendSubItem(SettingListItem *item);
    void showSubItems();
    void hideSubItems();
    QList<SettingListItem *> subItems();

signals:

public slots:

private:
    SettingItemInfo m_info;
    bool m_isOpen = false;
    QList<SettingListItem *> m_subItems;
};

#endif // SETTINGLISTITEM_H
