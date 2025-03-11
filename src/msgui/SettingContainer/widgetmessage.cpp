#include "widgetmessage.h"
#include "ui_widgetmessage.h"

WidgetMessage::WidgetMessage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetMessage)
{
    ui->setupUi(this);
}

WidgetMessage::~WidgetMessage()
{
    delete ui;
}

void WidgetMessage::showMessage(const QString &text)
{
    ui->labelMessage->setText(text);
    show();
}

void WidgetMessage::hideMessage()
{
    ui->labelMessage->clear();
    hide();
}
