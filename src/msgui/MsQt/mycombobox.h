#ifndef MYCOMBOBOX_H
#define MYCOMBOBOX_H

#include "combobox.h"

class MyComboBox : public ComboBox
{
    Q_OBJECT
public:
    explicit MyComboBox(QWidget *parent = nullptr);

    void setCurrentIndex(int index);
    void setCurrentIndexFromData(const QVariant &data, int role = Qt::UserRole);

signals:
    void currentIndexSet(int index);

private slots:
    void onActivated(int index);
};

#endif // MYCOMBOBOX_H
