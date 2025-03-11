#include "pagecontrol.h"
#include "ui_pagecontrol.h"

PageControl::PageControl(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageControl)
{
    ui->setupUi(this);
}

PageControl::~PageControl()
{
    delete ui;
}
