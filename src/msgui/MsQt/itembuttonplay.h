#ifndef ITEMBUTTONPLAY_H
#define ITEMBUTTONPLAY_H

#include "itembuttonwidget.h"

namespace Ui {
class ItemButtonPlay;
}

class ItemButtonPlay : public ItemButtonWidget
{
    Q_OBJECT

public:
    explicit ItemButtonPlay(int row, int column, ItemButtonWidget::ButtonType type, QWidget *parent = nullptr);
    ~ItemButtonPlay();

private slots:
    void on_toolButton_clicked();

private:
    Ui::ItemButtonPlay *ui;
};

#endif // ITEMBUTTONPLAY_H
