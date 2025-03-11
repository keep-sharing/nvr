#include "FisheyeBar.h"
#include "ui_FisheyeBar.h"

FisheyeBar::FisheyeBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FisheyeBar)
{
    ui->setupUi(this);
}

FisheyeBar::~FisheyeBar()
{
    delete ui;
}

void FisheyeBar::on_toolButton_ptz_clicked()
{
    emit buttonClicked(Mode_PTZ);
}

void FisheyeBar::on_toolButton_close_clicked()
{
    emit buttonClicked(Mode_Close);
}
