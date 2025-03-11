#include "patternbuttongroup.h"
#include "ui_patternbuttongroup.h"
#include "ptzitemdelegate.h"

PatternButtonGroup::PatternButtonGroup(int row, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PatternButtonGroup),
    m_row(row)
{
    ui->setupUi(this);
}

PatternButtonGroup::~PatternButtonGroup()
{
    delete ui;
}

void PatternButtonGroup::setButtonState(int state)
{
    switch (state)
    {
    case PtzItemDelegate::ItemDisable:
        ui->toolButton_play->setEnabled(false);
        ui->toolButton_record->setEnabled(true);
        ui->toolButton_delete->setEnabled(false);
        break;
    case PtzItemDelegate::ItemEnable:
        setPlayButtonState("play");
        ui->toolButton_play->setEnabled(true);
        ui->toolButton_record->setEnabled(true);
        ui->toolButton_delete->setEnabled(true);
        break;
    case PtzItemDelegate::ItemPatternPlaying:
        setPlayButtonState("stop");
        ui->toolButton_play->setEnabled(true);
        ui->toolButton_record->setEnabled(true);
        ui->toolButton_delete->setEnabled(true);
        break;
    case PtzItemDelegate::ItemPatternRecording:
        ui->toolButton_play->setEnabled(false);
        ui->toolButton_record->setEnabled(true);
        ui->toolButton_delete->setEnabled(false);
        break;
    default:
        break;
    }
}

void PatternButtonGroup::setPlayButtonState(const QString &state)
{
    ui->toolButton_play->setProperty("state", state);
    ui->toolButton_play->style()->unpolish(ui->toolButton_play);
    ui->toolButton_play->style()->polish(ui->toolButton_play);
}

void PatternButtonGroup::on_toolButton_play_clicked()
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

void PatternButtonGroup::on_toolButton_record_clicked()
{
    emit buttonClicked(m_row, 1);
}

void PatternButtonGroup::on_toolButton_delete_clicked()
{
    emit buttonClicked(m_row, 2);
}
