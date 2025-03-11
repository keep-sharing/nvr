#include "mycombobox.h"

MyComboBox::MyComboBox(QWidget *parent) :
    ComboBox(parent)
{
    connect(this, SIGNAL(activated(int)), this, SLOT(onActivated(int)));
}

void MyComboBox::setCurrentIndex(int index)
{
    ComboBox::setCurrentIndex(index);

    emit currentIndexSet(index);
}

void MyComboBox::setCurrentIndexFromData(const QVariant &data, int role)
{
    int index = findData(data, role);
    setCurrentIndex(index);
}

void MyComboBox::onActivated(int index)
{
    emit currentIndexSet(index);
}
