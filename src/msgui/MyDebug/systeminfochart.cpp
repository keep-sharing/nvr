#include "systeminfochart.h"
#include "ui_systeminfochart.h"
#include <QPainter>

SystemInfoChart::SystemInfoChart(QWidget *parent) :
    BaseShadowDialog(parent),
    ui(new Ui::SystemInfoChart)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint | Qt::WindowStaysOnTopHint);
    setWindowModality(Qt::NonModal);
    setTitleWidget(ui->label_title);
}

SystemInfoChart::~SystemInfoChart()
{
    delete ui;
}

void SystemInfoChart::appendValue(int index, int value)
{
    ui->timeChart->appendValue(index, value);
}

void SystemInfoChart::paintEvent(QPaintEvent *event)
{
    BaseShadowDialog::paintEvent(event);
}

void SystemInfoChart::on_toolButton_close_clicked()
{
    close();
}
