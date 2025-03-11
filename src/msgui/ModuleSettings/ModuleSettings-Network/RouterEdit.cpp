#include "RouterEdit.h"
#include "ui_RouterEdit.h"
#include "MsLanguage.h"
#include <QDebug>
#include <QFile>

RouterEdit::RouterEdit(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::RouterEdit)
{
    ui->setupUi(this);

    QFile file(":/style/style/settingstyle.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
      setStyleSheet(file.readAll());
    }
    file.close();

    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("CAMERASEARCH/32002", "No.");
    headerList << GET_TEXT("SYSTEMNETWORK/71162", "IPv6");
    headerList << GET_TEXT("SYSTEMNETWORK/71163", "Note");

    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setSortingEnabled(false);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->tableView->setColumnCount(headerList.size());
    ui->tableView->hideColumn(0);
    ui->tableView->setHeaderCheckable(false);
    //ui->tableView->setRowCount(1);
    ui->tableView->setColumnWidth(1, 80);
    ui->tableView->setColumnWidth(2, 250);

    ui->label_title->setText(GET_TEXT("SYSTEMNETWORK/71155", "Router Advertisement"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
}

RouterEdit::~RouterEdit()
{
    delete ui;
}

void RouterEdit::setNetinfo(network *info)
{
    _netinfo = *info;
}

void RouterEdit::updateInfo(int index)
{
    char tmpaddr[64] = { 0 };
    ui->label_title->setText(GET_TEXT("SYSTEMNETWORK/71155", "Router Advertisement"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));

    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("CAMERASEARCH/32002", "No.");
    headerList << GET_TEXT("SYSTEMNETWORK/71162", "IPv6");
    headerList << GET_TEXT("SYSTEMNETWORK/71163", "Note");
    ui->tableView->setHorizontalHeaderLabels(headerList);

    if (index == 0) //eth0
        snprintf(tmpaddr, sizeof(tmpaddr), "%s", _netinfo.lan1_ip6_address);
    else if (index == 1) //eth1
        snprintf(tmpaddr, sizeof(tmpaddr), "%s", _netinfo.lan2_ip6_address);
    else if (index == 2) //bond0
        snprintf(tmpaddr, sizeof(tmpaddr), "%s", _netinfo.bond0_ip6_address);

    if (tmpaddr[0] != '\0') {
        ui->tableView->setItemText(0, 1, "1");
        ui->tableView->setItemText(0, 2, QString("%1").arg(tmpaddr));
        ui->tableView->setItemText(0, 3, "");
    } else {
        ui->tableView->setItemText(0, 1, "");
        ui->tableView->setItemText(0, 2, "");
        ui->tableView->setItemText(0, 3, "");
    }
}

void RouterEdit::on_pushButton_back_clicked()
{
    close();
}
