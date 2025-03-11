#include "addmanualmode.h"
#include "ui_addmanualmode.h"
#include "MsLanguage.h"

AddManualMode::AddManualMode(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddManualMode)
{
    ui->setupUi(this);
	slotTranslateUi();
}

AddManualMode::~AddManualMode()
{
    delete ui;
}

void AddManualMode::slotTranslateUi()
{
	ui->label_title->setText(GET_TEXT("IMAGE/37353","Manual Mode Add"));
	ui->label_exposureTime->setText(GET_TEXT("IMAGE/37210","Exposure Time"));
	ui->label_gainLevel->setText(GET_TEXT("IMAGE/37211","Gain Level"));
	ui->pushButton_ok->setText(GET_TEXT("COMMON/1001","OK"));
	ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004","Cancel"));
}

