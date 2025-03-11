#include "timeeditwidget.h"
#include "ui_timeeditwidget.h"

TimeEditWidget::TimeEditWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TimeEditWidget)
{
    ui->setupUi(this);

    connect(ui->spinBox_beginHour, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
    connect(ui->spinBox_beginMinute, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
    connect(ui->spinBox_endHour, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
    connect(ui->spinBox_endMinute, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
}

TimeEditWidget::~TimeEditWidget()
{
    delete ui;
}

bool TimeEditWidget::hasFocus() const
{
    if (ui->spinBox_beginHour->hasFocus() || ui->spinBox_beginMinute->hasFocus() || ui->spinBox_endHour->hasFocus() || ui->spinBox_endMinute->hasFocus())
    {
        return true;
    }
    return false;
}

int TimeEditWidget::beginMinute() const
{
    return ui->spinBox_beginHour->value() * 60 + ui->spinBox_beginMinute->value();
}

void TimeEditWidget::setBeginMinute(int minute)
{
	int hour = minute / 60;
	int min = minute % 60;

    ui->spinBox_beginHour->setValue(hour);
    ui->spinBox_beginMinute->setValue(min);
}

int TimeEditWidget::endMinute() const
{
    return ui->spinBox_endHour->value() * 60 + ui->spinBox_endMinute->value();
}

void TimeEditWidget::setEndMinute(int minute)
{
	int hour = minute / 60;
	int min = minute % 60;

    ui->spinBox_endHour->setValue(hour);
    ui->spinBox_endMinute->setValue(min);
}

void TimeEditWidget::on_spinBox_endHour_valueChanged(int arg1)
{
	if (arg1 == 24)
	{
		ui->spinBox_endMinute->setDisabled(true);
		ui->spinBox_endMinute->setValue(0);
	}
	else
	{
		ui->spinBox_endMinute->setDisabled(false);
	}
}

void TimeEditWidget::on_spinBox_beginHour_valueChanged(int arg1)
{
	if (arg1 == 24)
	{
		ui->spinBox_beginMinute->setDisabled(true);
		ui->spinBox_beginMinute->setValue(0);
	}
	else
	{
		ui->spinBox_beginMinute->setDisabled(false);
	}

}
