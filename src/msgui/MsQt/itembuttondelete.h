#ifndef ITEMBUTTONDELETE_H
#define ITEMBUTTONDELETE_H

#include "itembuttonwidget.h"

namespace Ui {
class ItemButtonDelete;
}

class ItemButtonDelete : public ItemButtonWidget
{
    Q_OBJECT

public:
    explicit ItemButtonDelete(int row, int column, ItemButtonWidget::ButtonType type, QWidget *parent = 0);
    ~ItemButtonDelete();

private slots:
    void on_toolButton_clicked();

private:
    Ui::ItemButtonDelete *ui;
};

#endif // ITEMBUTTONDELETE_H
