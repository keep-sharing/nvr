#ifndef SETTINGLISTWIDGET_H
#define SETTINGLISTWIDGET_H

#include <QWidget>
#include <QButtonGroup>
#include <QQueue>
#include "settinglistitem.h"

namespace Ui {
class SettingListWidget;
}

class SettingListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingListWidget(QWidget *parent = nullptr);
    ~SettingListWidget();

    void setItemList(const QList<SettingItemInfo> &infoList);
    void updateLanguage(const QList<SettingItemInfo> &infoList);

    //改变界面并触发信号
    void selectFirstItem();
    void selectItem(int index);
    void selectNextItem();
    void selectPreviousItem();
    bool isFirstItemSelected();
    bool isLastItemSelected();
    //只改变界面，不触发信号
    void editItem(int index);

    QString currentItemText() const;

    QList<QWidget *> test_itemButtonList();

signals:
    void itemClicked(const SettingItemInfo &item);

private slots:
    void onButtonClicked(int index);

private:
    Ui::SettingListWidget *ui;

    QButtonGroup *m_group = nullptr;

    QList<SettingListItem *> m_itemList;
    SettingListItem *m_currentSubItem = nullptr;
};

#endif // SETTINGLISTWIDGET_H
