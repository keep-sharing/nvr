#include "itembuttondelete.h"
#include "ui_itembuttondelete.h"

ItemButtonDelete::ItemButtonDelete(int row, int column, ItemButtonWidget::ButtonType type, QWidget *parent) :
    ItemButtonWidget(row, column, type, parent),
    ui(new Ui::ItemButtonDelete)
{
    ui->setupUi(this);
}

ItemButtonDelete::~ItemButtonDelete()
{
    delete ui;
}

void ItemButtonDelete::on_toolButton_clicked()
{
    emit clicked(m_row, m_column, m_type);
}
