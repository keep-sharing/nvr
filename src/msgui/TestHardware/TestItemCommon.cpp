#include "TestItemCommon.h"
#include "ui_TestItemCommon.h"

TestItemCommon::TestItemCommon(QWidget *parent) :
    AbstractTestItem(parent),
    ui(new Ui::TestItemCommon)
{
    ui->setupUi(this);

    ui->toolButtonStart->hide();
}

TestItemCommon::~TestItemCommon()
{
    delete ui;
}

void TestItemCommon::setModuleName(const QString &text)
{
    m_moduleName = text;
    ui->checkBox->setText("TEST " + text);
}

void TestItemCommon::startTest()
{
    on_toolButtonStart_clicked();
}

void TestItemCommon::setChecked(bool newChecked)
{
    ui->checkBox->setChecked(newChecked);
    AbstractTestItem::setChecked(newChecked);
}

void TestItemCommon::on_checkBox_clicked(bool checked)
{
    ui->toolButtonStart->setEnabled(checked);
    AbstractTestItem::setChecked(checked);
}

void TestItemCommon::on_toolButtonStart_clicked()
{
    ui->toolButtonStart->setAttribute(Qt::WA_UnderMouse, false);
    emit itemStarted(this);
    QMetaObject::invokeMethod(TestHardwareData::instance(), "testItemStart", Qt::QueuedConnection, Q_ARG(QString, m_moduleName));
}
