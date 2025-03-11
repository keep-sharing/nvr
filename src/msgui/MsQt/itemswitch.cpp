#include "itemswitch.h"
#include "ui_itemswitch.h"

ItemSwitch::ItemSwitch(int row, int column, bool checked, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ItemSwitch),
    m_row(row),
    m_column(column)
{
    ui->setupUi(this);
    ui->toolButton->setChecked(checked);
}

ItemSwitch::~ItemSwitch()
{
    delete ui;
}

void ItemSwitch::setChecked(bool checked)
{
    ui->toolButton->setChecked(checked);
}

bool ItemSwitch::isChecked() const
{
    return ui->toolButton->isChecked();
}

void ItemSwitch::on_toolButton_clicked(bool checked)
{
    emit switchChanged(m_row, m_column, checked);
}
