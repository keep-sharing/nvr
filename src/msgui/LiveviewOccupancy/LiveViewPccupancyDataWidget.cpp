#include "LiveViewPccupancyDataWidget.h"
#include "ui_LiveViewPccupancyDataWidget.h"

LiveViewPccupancyDataWidget::LiveViewPccupancyDataWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LiveViewPccupancyDataWidget)
{
    ui->setupUi(this);
}

LiveViewPccupancyDataWidget::~LiveViewPccupancyDataWidget()
{
    delete ui;
}

void LiveViewPccupancyDataWidget::setIcon(QString text)
{
    ui->labelIcon->setPixmap(QPixmap(text));
}

void LiveViewPccupancyDataWidget::setItemText(QString text)
{
    ui->labelText->setText(text);
}

void LiveViewPccupancyDataWidget::setValue(QString text)
{
    ui->labelValue->setText(text);
}

void LiveViewPccupancyDataWidget::setIconVisible(bool isVisible)
{
    ui->labelIcon->setVisible(isVisible);
}

void LiveViewPccupancyDataWidget::setTextSizeGreat()
{
    setIconVisible(false);
    ui->labelText->setStyleSheet("QLabel{padding-left: 15px;color: #FFFFFF;font: 1500 75pt;}");
    ui->labelValue->setStyleSheet("QLabel{color: #FFFFFF;font: 1500 75pt;}");
}

void LiveViewPccupancyDataWidget::setTextSizeSmall()
{
    setIconVisible(true);
    ui->labelText->setStyleSheet("QLabel{padding-left: 15px;color: #FFFFFF;font: 48pt;}");
    ui->labelValue->setStyleSheet("QLabel{color: #FFFFFF;font: 48pt;}");
}
