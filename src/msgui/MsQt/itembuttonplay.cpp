#include "itembuttonplay.h"
#include "ui_itembuttonplay.h"

ItemButtonPlay::ItemButtonPlay(int row, int column, ItemButtonWidget::ButtonType type, QWidget *parent) :
    ItemButtonWidget(row, column, type, parent),
    ui(new Ui::ItemButtonPlay)
{
    ui->setupUi(this);
}

ItemButtonPlay::~ItemButtonPlay()
{
    delete ui;
}

void ItemButtonPlay::on_toolButton_clicked()
{
    emit clicked(m_row, m_column, m_type);
}
