#include "settinglistitem.h"
#include "MyDebug.h"

QDebug operator<<(QDebug dbg, const SettingItemInfo &info)
{
    dbg.nospace() << qPrintable(QString("SettingItemInfo(name:%1,id:%2,permission:%3)").arg(info.name).arg(info.id).arg(info.permission));
    return dbg.space();
}

SettingListItem::SettingListItem(const SettingItemInfo &info, QWidget *parent) :
    QPushButton(parent)
{
    m_info = info;

    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    setCheckable(true);
    setText(m_info.name);
    setVisible(m_info.visible);
}

SettingItemInfo SettingListItem::itemInfo() const
{
    return m_info;
}

void SettingListItem::updateText(const QString &text)
{
    m_info.name = text;
    setText(text);
}

void SettingListItem::clearUnderMouse()
{
    setAttribute(Qt::WA_UnderMouse, false);
}

bool SettingListItem::isOpen() const
{
    return m_isOpen;
}

bool SettingListItem::isItemMain() const
{
    return !m_info.subItems.isEmpty();
}

bool SettingListItem::isItemSub() const
{
    return m_info.subItems.isEmpty();
}

bool SettingListItem::isItemVisible() const
{
    return m_info.visible;
}

void SettingListItem::appendSubItem(SettingListItem *item)
{
    m_subItems.append(item);
}

void SettingListItem::showSubItems()
{
    if (!isItemMain())
    {
        return;
    }
    for (int i = 0; i < m_subItems.size(); ++i)
    {
        SettingListItem *item = m_subItems.at(i);
        if (item->isItemVisible())
        {
            item->show();
        }
    }
    m_isOpen = true;
    update();
}

void SettingListItem::hideSubItems()
{
    if (!isItemMain())
    {
        return;
    }
    for (int i = 0; i < m_subItems.size(); ++i)
    {
        SettingListItem *item = m_subItems.at(i);
        item->hide();
    }
    m_isOpen = false;
    update();
}

QList<SettingListItem *> SettingListItem::subItems()
{
    return m_subItems;
}
