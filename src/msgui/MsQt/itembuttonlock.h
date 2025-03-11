#ifndef ITEMBUTTONLOCK_H
#define ITEMBUTTONLOCK_H

#include "itembuttonwidget.h"

namespace Ui {
class ItemButtonLock;
}

class ItemButtonLock : public ItemButtonWidget
{
    Q_OBJECT

public:
    explicit ItemButtonLock(int row, int column, ItemButtonWidget::ButtonType type, QWidget *parent = nullptr);
    ~ItemButtonLock();

private slots:
    void on_toolButton_clicked();

private:
    Ui::ItemButtonLock *ui;
};

#endif // ITEMBUTTONLOCK_H
