#include "patrolbuttongroup.h"
#include "ui_patrolbuttongroup.h"
#include "ptzitemdelegate.h"

PatrolButtonGroup::PatrolButtonGroup(int row, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PatrolButtonGroup),
    m_row(row)
{
    ui->setupUi(this);
}

PatrolButtonGroup::~PatrolButtonGroup()
{
    delete ui;
}

void PatrolButtonGroup::setButtonState(int state)
{
    switch (state)
    {
    case PtzItemDelegate::ItemDisable:
        ui->toolButton_play->setEnabled(false);
        ui->toolButton_setting->setEnabled(true);
        ui->toolButton_delete->setEnabled(false);
        break;
    case PtzItemDelegate::ItemEnable:
        setPlayButtonState("play");
        ui->toolButton_play->setEnabled(true);
        ui->toolButton_setting->setEnabled(true);
        ui->toolButton_delete->setEnabled(true);
        break;
    case PtzItemDelegate::ItemPatrolPlaying:
        setPlayButtonState("stop");
        ui->toolButton_play->setEnabled(true);
        ui->toolButton_setting->setEnabled(true);
        ui->toolButton_delete->setEnabled(true);
        break;
    default:
        break;
    }
}

void PatrolButtonGroup::setPlayButtonState(const QString &state)
{
    ui->toolButton_play->setProperty("state", state);
    ui->toolButton_play->style()->unpolish(ui->toolButton_play);
    ui->toolButton_play->style()->polish(ui->toolButton_play);
}

void PatrolButtonGroup::on_toolButton_play_clicked()
{
    const QString &state = ui->toolButton_play->property("state").toString();
    if (state == "play")
    {
        setPlayButtonState("stop");
    }
    else if (state == "stop")
    {
        setPlayButtonState("play");
    }
    emit buttonClicked(m_row, 0);
}

void PatrolButtonGroup::on_toolButton_setting_clicked()
{
    emit buttonClicked(m_row, 1);
}

void PatrolButtonGroup::on_toolButton_delete_clicked()
{
    emit buttonClicked(m_row, 2);
}
