#include "TestWizard.h"
#include "ui_TestWizard.h"
#include "AbstractTestItem.h"
#include "TestHardwareData.h"
#include <QTimer>

TestWizard::TestWizard(QWidget *parent) :
    BaseShadowDialog(parent),
    ui(new Ui::TestWizard)
{
    ui->setupUi(this);

    setTitleWidget(ui->widgetTitle);
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
    setWindowModality(Qt::NonModal);

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    m_timer->setInterval(1000);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

TestWizard::~TestWizard()
{
    delete ui;
}

void TestWizard::startTest(AbstractTestItem *item)
{
    m_currentItem = item;
    ui->labelTitle->setText("TEST " + item->moduleName());
    ui->labelMessage->clear();
}

void TestWizard::showMessage(const QString &text)
{
    ui->labelMessage->setText(text);
}

void TestWizard::onTimeout()
{
    if (TestHardwareData::instance()->isItemRunning()) {
        m_timer->start();
        return;
    }
    ui->pushButtonPass->setEnabled(true);
    ui->pushButtonNotPass->setEnabled(true);
    emit testNext();
}

void TestWizard::on_pushButtonPass_clicked()
{
    m_currentItem->setPassed(true);
    QMetaObject::invokeMethod(TestHardwareData::instance(), "testItemStop", Qt::DirectConnection);

    ui->pushButtonPass->setEnabled(false);
    ui->pushButtonNotPass->setEnabled(false);
    m_timer->start();
}

void TestWizard::on_pushButtonNotPass_clicked()
{
    m_currentItem->setPassed(false);
    QMetaObject::invokeMethod(TestHardwareData::instance(), "testItemStop", Qt::DirectConnection);

    ui->pushButtonPass->setEnabled(false);
    ui->pushButtonNotPass->setEnabled(false);
    m_timer->start();
}

void TestWizard::on_pushButtonCancel_clicked()
{
    QMetaObject::invokeMethod(TestHardwareData::instance(), "testItemStop", Qt::DirectConnection);
    ui->pushButtonPass->setEnabled(true);
    ui->pushButtonNotPass->setEnabled(true);

    emit testCancel();
}
