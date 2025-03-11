#include "itembuttonlock.h"
#include "ui_itembuttonlock.h"

ItemButtonLock::ItemButtonLock(int row, int column, ItemButtonWidget::ButtonType type, QWidget *parent) :
    ItemButtonWidget(row, column, type, parent),
    ui(new Ui::ItemButtonLock)
{
    ui->setupUi(this);
}

ItemButtonLock::~ItemButtonLock()
{
    delete ui;
}

void ItemButtonLock::on_toolButton_clicked()
{
    emit clicked(m_row, m_column, m_type);
}
