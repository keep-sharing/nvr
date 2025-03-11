#include "presetbuttongroup.h"
#include "ui_presetbuttongroup.h"
#include "ptzitemdelegate.h"

PresetButtonGroup::PresetButtonGroup(int row, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PresetButtonGroup),
    m_row(row)
{
    ui->setupUi(this);
}

PresetButtonGroup::~PresetButtonGroup()
{
    delete ui;
}

void PresetButtonGroup::setButtonState(int state)
{
    switch (state)
    {
    case PtzItemDelegate::ItemDisable:
        ui->toolButton_save->setEnabled(true);
        ui->toolButton_delete->setEnabled(false);
        ui->toolButton_play->setEnabled(false);
        break;
    case PtzItemDelegate::ItemEnable:
        ui->toolButton_save->setEnabled(true);
        ui->toolButton_delete->setEnabled(true);
        ui->toolButton_play->setEnabled(true);
        break;
    case PtzItemDelegate::ItemValidDefault:
        ui->toolButton_save->setEnabled(false);
        ui->toolButton_delete->setEnabled(false);
        ui->toolButton_play->setEnabled(true);
        break;
    case PtzItemDelegate::ItemInvalidDefault:
        ui->toolButton_save->setEnabled(false);
        ui->toolButton_delete->setEnabled(false);
        ui->toolButton_play->setEnabled(false);
        break;
    default:
        break;
    }
}

void PresetButtonGroup::on_toolButton_play_clicked()
{
    emit buttonClicked(m_row, 0);
}

void PresetButtonGroup::on_toolButton_save_clicked()
{
    emit buttonClicked(m_row, 1);
}

void PresetButtonGroup::on_toolButton_delete_clicked()
{
    emit buttonClicked(m_row, 2);
}
