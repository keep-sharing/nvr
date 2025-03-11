#include "msschedulewidget.h"
#include "ui_msschedulewidget.h"
#include <QtDebug>
#include "MsLanguage.h"

MsScheduleWidget::MsScheduleWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MsScheduleWidget)
{
    ui->setupUi(this);

    m_schedultEdit = new MsScheduleEdit(this);
    connect(m_schedultEdit, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));

    onLanguageChanged();
}

MsScheduleWidget::~MsScheduleWidget()
{
    delete ui;
}

void MsScheduleWidget::setCurrentType(const int &type)
{
    ui->scheduleDraw->setCurrentType(type);
}

void MsScheduleWidget::setTypeColor(int type, QColor color)
{
    ui->scheduleDraw->setTypeColor(type, color);
}

void MsScheduleWidget::clearTypeColor()
{
    ui->scheduleDraw->clearTypeColor();
}

void MsScheduleWidget::addEditType(const QString &name, int type)
{
    m_schedultEdit->addType(name, type);
}

void MsScheduleWidget::clearEditType()
{
    m_schedultEdit->clearType();
}

void MsScheduleWidget::setSingleEditType(int type)
{
    m_schedultEdit->setSingleEditType(type);
}

void MsScheduleWidget::setHolidayVisible(bool visible)
{
    m_schedultEdit->setHolidayVisible(visible);
    ui->scheduleDraw->setScheduleMode(!visible);
}

void MsScheduleWidget::setCheckDayEnable(bool enable)
{
    ui->scheduleDraw->setCheckDayEnable(enable);
}

void MsScheduleWidget::setSchedule(schedule_day *schedule_day_array)
{
    ui->scheduleDraw->setSchedule(schedule_day_array);
}

void MsScheduleWidget::getSchedule(schedule_day *schedule_day_array)
{
    ui->scheduleDraw->getSchedule(schedule_day_array);
}

void MsScheduleWidget::onLanguageChanged()
{
    ui->pushButton_clearAll->setText(GET_TEXT("COMMON/1021", "Clear All"));
    ui->pushButton_selectAll->setText(GET_TEXT("COMMON/1022", "Select All"));
    ui->pushButton_edit->setText(GET_TEXT("COMMON/1019", "Edit"));
}

void MsScheduleWidget::onEditingFinished()
{
    ui->scheduleDraw->setScheduleArray(m_schedultEdit->scheduleArray());
}

void MsScheduleWidget::on_pushButton_clearAll_clicked()
{
    ui->scheduleDraw->clearCurrent();
}

void MsScheduleWidget::on_pushButton_selectAll_clicked()
{
    ui->scheduleDraw->selectAll();
}

void MsScheduleWidget::on_pushButton_edit_clicked()
{
    m_schedultEdit->setScheduleArray(ui->scheduleDraw->scheduleArray());
    m_schedultEdit->exec();
}
