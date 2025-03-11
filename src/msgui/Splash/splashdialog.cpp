#include "splashdialog.h"
#include "ui_splashdialog.h"
#include <QPainter>

SplashDialog::SplashDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SplashDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    m_background = QPixmap("/opt/app/bin/logo.jpg");
}

SplashDialog::~SplashDialog()
{
    delete ui;
}

void SplashDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.drawPixmap(rect(), m_background);
}
