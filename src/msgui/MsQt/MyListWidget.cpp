#include "MyListWidget.h"
#include "ui_MyListWidget.h"

MyListWidget::MyListWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MyListWidget)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    ui->listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->listWidget->setStyleSheet("QListWidget{outline: none;border: 1px solid #949494;background-color: #FFFFFF;} QListWidget::item{border: none;padding-left: 20px;height: 30px;color: #4A4A4A;} QListWidget::item:selected{color: #FFFFFF;background-color: #0AA9E3;} QListWidget::item:hover{color: #FFFFFF;background-color: #0AA9E3;}");
}

MyListWidget::~MyListWidget()
{
    delete ui;
}

void MyListWidget::setCurrentIndex(int index)
{
    ui->listWidget->setCurrentRow(index);
    on_listWidget_itemClicked(ui->listWidget->currentItem());
}

void MyListWidget::addItem(QString text, int index)
{
    QListWidgetItem *item = new QListWidgetItem(text);
    item->setData(Qt::UserRole, index);
    ui->listWidget->addItem(item);

    if (ui->listWidget->count() <= 10) {
        setMaximumHeight(ui->listWidget->count() * 30);
        setMinimumHeight(ui->listWidget->count() * 30);
    } else {
        setMaximumHeight(300);
        setMinimumHeight(300);
    }
}

void MyListWidget::clear()
{
    ui->listWidget->clear();
}

int MyListWidget::currentInt()
{
    return ui->listWidget->currentItem()->data(Qt::UserRole).toInt();
}

void MyListWidget::hideEvent(QHideEvent *)
{
    close();
}

void MyListWidget::on_listWidget_itemClicked(QListWidgetItem *item)
{
    close();
    emit itemClicked(item->text());
}
