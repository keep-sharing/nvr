#ifndef MYPUSHBUTTONWITHCOMBOBOX_H
#define MYPUSHBUTTONWITHCOMBOBOX_H

#include <QLabel>
#include "MyListWidget.h"
#include <mypushbutton.h>

class MyPushButtonWithCombobox : public MyPushButton {
    Q_OBJECT
public:
    explicit MyPushButtonWithCombobox(QWidget *parent = nullptr);
    void setCurrentIndex(int index);
    void addItem(QString text, int data);
    void clear();
    int currentInt();

  signals:
    void itemClicked();
public slots:
    void onButtonClicked(bool clicked);
    void onItemClicked(const QString &text);

private:
    MyListWidget *m_listWidget = nullptr;
    QLabel *m_textLabel = nullptr;
};

#endif // MYPUSHBUTTONWITHCOMBOBOX_H
