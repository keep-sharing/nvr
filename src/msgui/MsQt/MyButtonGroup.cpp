#include "MyButtonGroup.h"
#include <QAbstractButton>
#include "MyDebug.h"

MyButtonGroup::MyButtonGroup(QObject *parent)
    : QButtonGroup(parent)
{
}

void MyButtonGroup::setCurrentButton(QAbstractButton *btn)
{
    editCurrentButton(btn);
    emit buttonClicked(id(btn));
}

void MyButtonGroup::editCurrentButton(QAbstractButton *btn)
{
    if (btn) {
        btn->setChecked(true);
    }
}

void MyButtonGroup::setCurrentId(int id)
{
    editCurrentId(id);
    emit buttonClicked(id);
}

void MyButtonGroup::editCurrentId(int id)
{
    QAbstractButton *btn = button(id);
    if (btn) {
        btn->setChecked(true);
    }
}
