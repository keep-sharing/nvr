#ifndef ITEMBUTTONEDIT_H
#define ITEMBUTTONEDIT_H

#include "itembuttonwidget.h"

namespace Ui {
class ItemButtonEdit;
}

class ItemButtonEdit : public ItemButtonWidget
{
    Q_OBJECT

public:
    explicit ItemButtonEdit(int row, int column, ItemButtonWidget::ButtonType type, QWidget *parent = 0);
    ~ItemButtonEdit();

private slots:
    void on_toolButton_clicked();

private:
    Ui::ItemButtonEdit *ui;
};

#endif // ITEMBUTTONEDIT_H
