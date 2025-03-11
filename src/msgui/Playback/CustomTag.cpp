#include "CustomTag.h"
#include "ui_CustomTag.h"
#include <QFile>
#include <QtDebug>
#include "MessageBox.h"
#include "MsLanguage.h"
#include "MsGlobal.h"

CustomTag::CustomTag(QWidget *parent) :
    BaseShadowDialog(parent),
    ui(new Ui::CustomTag)
{
    ui->setupUi(this);

    QFile file(":/style/style/settingstyle.qss");
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        setStyleSheet(file.readAll());
    }
    else
    {
        qWarning() << QString("PlaybackFileManagement load style failed.");
    }
    file.close();

    ui->lineEdit_name->setMaxLength(32);
    ui->lineEdit_name->setCheckMode(MyLineEdit::DDNSCheck);
}

CustomTag::~CustomTag()
{
    delete ui;
}

void CustomTag::addTag(const QString &title)
{
    ui->label_title->setText(title);
    ui->pushButton_add->setText(GET_TEXT("COMMON/1000", "Add"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

void CustomTag::editTag(const QString &name)
{
    ui->label_title->setText(GET_TEXT("PLAYBACK/80083","Tag Edit"));
    ui->lineEdit_name->setText(name);
    ui->pushButton_add->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
}

QString CustomTag::tagName()
{
    return ui->lineEdit_name->text();
}

void CustomTag::on_pushButton_add_clicked()
{
    if (!ui->lineEdit_name->checkValid()) {
        return;
    }

    accept();
}

void CustomTag::on_pushButton_cancel_clicked()
{
    reject();
}
