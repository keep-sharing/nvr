#include "itembuttoncheckbox.h"
#include "ui_itembuttoncheckbox.h"

ItemButtonCheckBox::ItemButtonCheckBox(int row, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ItemButtonCheckBox),
    m_row(row)
{
    ui->setupUi(this);
}

ItemButtonCheckBox::~ItemButtonCheckBox()
{
    delete ui;
}

void ItemButtonCheckBox::setChecked(bool checked)
{
    ui->checkBox->setChecked(checked);
}

void ItemButtonCheckBox::on_checkBox_clicked(bool checked)
{
    emit checkBoxClicked(m_row, checked);
}
