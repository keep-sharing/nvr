#include "testanprplay.h"
#include "ui_testanprplay.h"

TestAnprPlay::TestAnprPlay(QWidget *parent) :
    BaseShadowDialog(parent),
    ui(new Ui::TestAnprPlay)
{
    ui->setupUi(this);
    setWindowModality(Qt::NonModal);
    setTitleWidget(ui->label_title);
}

TestAnprPlay::~TestAnprPlay()
{
    delete ui;
}

void TestAnprPlay::setPlayRect(const QRect &rc)
{
    ui->label_rect->setText(QString("(%1, %2, %3 x %4)").arg(rc.x()).arg(rc.y()).arg(rc.width()).arg(rc.height()));
}

QRect TestAnprPlay::playRect() const
{
    return QRect(ui->spinBox_x->value(), ui->spinBox_y->value(), ui->spinBox_width->value(), ui->spinBox_height->value());
}
