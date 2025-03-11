#include "BottomBar.h"
#include "ui_BottomBar.h"
#include <QPainter>

BottomBar::BottomBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BottomBar)
{
    ui->setupUi(this);
}

BottomBar::~BottomBar()
{
    delete ui;
}

void BottomBar::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 40));
    painter.drawRect(rect());
}

void BottomBar::on_toolButton_1_clicked()
{
    emit layoutChanged(1, 1);
}

void BottomBar::on_toolButton_4_clicked()
{
    emit layoutChanged(2, 2);
}

void BottomBar::on_toolButton_9_clicked()
{
    emit layoutChanged(3, 3);
}

void BottomBar::on_toolButton_12_clicked()
{
    emit layoutChanged(3, 4);
}

void BottomBar::on_toolButton_16_clicked()
{
    emit layoutChanged(4, 4);
}

void BottomBar::on_toolButton_25_clicked()
{
    emit layoutChanged(5, 5);
}

void BottomBar::on_toolButton_36_clicked()
{
    emit layoutChanged(6, 6);
}

void BottomBar::on_pushButtonCameraManagement_clicked()
{
    emit cameraManagement();
}

