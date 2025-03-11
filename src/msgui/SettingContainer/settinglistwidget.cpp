#include "settinglistwidget.h"
#include "ui_settinglistwidget.h"
#include <QtDebug>
#include "settinglistitemmain.h"
#include "settinglistitemsub.h"
#include "MyDebug.h"

SettingListWidget::SettingListWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingListWidget)
{
    ui->setupUi(this);

    m_group = new QButtonGroup(this);
    connect(m_group, SIGNAL(buttonClicked(int)), this, SLOT(onButtonClicked(int)));
}

SettingListWidget::~SettingListWidget()
{
    delete ui;
}

void SettingListWidget::setItemList(const QList<SettingItemInfo> &infoList)
{
    //
    for (int i = 0; i < m_itemList.size(); ++i)
    {
        SettingListItem *item = m_itemList.at(i);
        ui->verticalLayout->removeWidget(item);
        m_group->removeButton(item);
        delete item;
        item = nullptr;
    }
    m_itemList.clear();
    m_currentSubItem = nullptr;
    //
    int index = 0;
    for (int i = 0; i < infoList.size(); ++i)
    {
        const SettingItemInfo &infoMain = infoList.at(i);
        SettingListItemMain *itemMain = new SettingListItemMain(infoMain, this);
        ui->verticalLayout->addWidget(itemMain);
        m_group->addButton(itemMain, index);
        index++;
        m_itemList.append(itemMain);

        for (int i = 0; i < infoMain.subItems.size(); ++i)
        {
            const SettingItemInfo &infoSub = infoMain.subItems.at(i);
            SettingListItemSub *itemSub = new SettingListItemSub(infoSub, this);
            ui->verticalLayout->addWidget(itemSub);
            m_group->addButton(itemSub, index);
            index++;
            m_itemList.append(itemSub);
            //
            itemMain->appendSubItem(itemSub);
        }
    }
}

void SettingListWidget::updateLanguage(const QList<SettingItemInfo> &infoList)
{
    for (int i = 0; i < m_itemList.size(); ++i)
    {
        SettingListItem *item = m_itemList.at(i);
        SettingItemInfo info = infoList.at(i);
        item->updateText(info.name);
    }
}

void SettingListWidget::selectFirstItem()
{
    for (int i = 0; i < m_itemList.size(); ++i)
    {
        SettingListItem *item = m_itemList.at(i);
        if (item->isItemVisible())
        {
            item->setChecked(true);
            onButtonClicked(i);
            //
            if (item->isItemMain())
            {
                SettingListItem *subItem = m_itemList.at(i + 1);
                if (subItem->isItemVisible())
                {
                    subItem->setChecked(true);
                    onButtonClicked(i + 1);
                    m_currentSubItem = subItem;
                }
                else
                {
                    continue;
                }
            }
            //
            break;
        }
    }
}

void SettingListWidget::selectItem(int index)
{
    SettingListItem *item = m_itemList.at(index);
    if (item->isVisible())
    {
        item->setChecked(true);
        onButtonClicked(index);
    }
}

void SettingListWidget::selectNextItem()
{
    qDebug() << "====SettingListWidget::selectNextItem====";
    int checkedIndex = m_group->checkedId();
    qDebug() << "----checked index:" << checkedIndex;
    bool vaildItem = false;
    do
    {
        if (checkedIndex < 0)
        {
            checkedIndex = 0;
        }
        else if (checkedIndex < m_itemList.size() - 1)
        {
            checkedIndex += 1;
        }
        else
        {
            checkedIndex = 0;
        }
        SettingListItem *item = m_itemList.at(checkedIndex);
        if (item->isVisible())
        {
            vaildItem = true;
        }
    } while (!vaildItem);

    qDebug() << "----next index:" << checkedIndex;
    selectItem(checkedIndex);
}

void SettingListWidget::selectPreviousItem()
{
    qDebug() << "====SettingListWidget::selectPreviousItem====";
    int checkedIndex = m_group->checkedId();
    qDebug() << "----checked index:" << checkedIndex;
    bool vaildItem = false;
    do
    {
        if (checkedIndex < 0)
        {
            checkedIndex = 0;
        }
        else if (checkedIndex == 0)
        {
            checkedIndex = m_itemList.size() - 1;
        }
        else
        {
            checkedIndex -= 1;
        }
        SettingListItem *item = m_itemList.at(checkedIndex);
        if (item->isVisible())
        {
            vaildItem = true;
        }
    } while (!vaildItem);

    qDebug() << "----previous index:" << checkedIndex;
    selectItem(checkedIndex);
}

bool SettingListWidget::isFirstItemSelected()
{
    bool result = false;
    for (int i = 0; i < m_itemList.size(); ++i)
    {
        SettingListItem *item = m_itemList.at(i);
        if (item->isVisible())
        {
            result = item->isChecked();
            break;
        }
    }
    return result;
}

bool SettingListWidget::isLastItemSelected()
{
    bool result = false;
    for (int i = m_itemList.size() - 1; i >= 0; --i)
    {
        SettingListItem *item = m_itemList.at(i);
        if (item->isVisible())
        {
            result = item->isChecked();
            break;
        }
    }
    return result;
}

void SettingListWidget::editItem(int index)
{
    if (index < 0 || index > m_itemList.size() - 1)
    {
        return;
    }
    SettingListItem *item = m_itemList.at(index);
    if (item->isVisible())
    {
        item->setChecked(true);
    }
}

QString SettingListWidget::currentItemText() const
{
    QString text;
    QAbstractButton *button = m_group->checkedButton();
    if (button)
    {
        text = button->text();
    }
    return text;
}

QList<QWidget *> SettingListWidget::test_itemButtonList()
{
    QList<QWidget *> list;
    for (int i = 0; i < m_itemList.size(); ++i)
    {
        list.append(m_itemList.at(i));
    }
    return list;
}

void SettingListWidget::onButtonClicked(int index)
{
    SettingListItem *item = m_itemList.at(index);
    item->clearUnderMouse();
    if (item->isItemMain())
    {
        if (item->isOpen())
        {
            item->hideSubItems();
        }
        else
        {
            item->showSubItems();
            QList<SettingListItem *> subItems = item->subItems();
            for (int i = 0; i < subItems.size(); ++i)
            {
                SettingListItem *subItem = subItems.at(i);
                if (subItem == m_currentSubItem)
                {
                    subItem->setChecked(true);
                }
            }
            //
            for (int i = 0; i < m_itemList.size(); ++i)
            {
                SettingListItem *itemMain = m_itemList.at(i);
                if (itemMain != item)
                {
                    itemMain->hideSubItems();
                }
            }
        }
    }
    else
    {
        m_currentSubItem = item;
        emit itemClicked(item->itemInfo());
    }
}
