#include "itembuttonedit.h"
#include "ui_itembuttonedit.h"
#include <QtDebug>

ItemButtonEdit::ItemButtonEdit(int row, int column, ItemButtonWidget::ButtonType type, QWidget *parent) :
    ItemButtonWidget(row, column, type, parent),
    ui(new Ui::ItemButtonEdit)
{
    ui->setupUi(this);
}

ItemButtonEdit::~ItemButtonEdit()
{
    delete ui;
}

void ItemButtonEdit::on_toolButton_clicked()
{
    emit clicked(m_row, m_column, m_type);
}
