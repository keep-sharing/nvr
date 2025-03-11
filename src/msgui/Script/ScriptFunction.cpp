#include "ScriptFunction.h"
#include "ui_ScriptFunction.h"

ScriptFunction::ScriptFunction(QWidget *parent) :
    BaseShadowDialog(parent),
    ui(new Ui::ScriptFunction)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
}

ScriptFunction::~ScriptFunction()
{
    delete ui;
}
