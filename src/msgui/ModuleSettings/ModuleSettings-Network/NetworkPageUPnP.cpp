#include "NetworkPageUPnP.h"
#include "ui_NetworkPageUPnP.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "UPnPEdit.h"
#include "centralmessage.h"
#include <QtDebug>

extern "C" {
#include "msdb.h"
}

NetworkPageUPnp::NetworkPageUPnp(QWidget *parent)
    : AbstractNetworkPage(parent)
    , ui(new Ui::NetworkPageUPnp)
{
    ui->setupUi(this);

    initNetUPnPTab();

    onLanguageChanged();
}

NetworkPageUPnp::~NetworkPageUPnp()
{
    freeNetUPnPtab();
    delete ui;
}

void NetworkPageUPnp::initializeData()
{
    gotoNetUPnPtab();
}

void NetworkPageUPnp::initNetUPnPTab()
{
    ui->comboBox_enableUpnp->clear();
    ui->comboBox_enableUpnp->addItem("Disable", false);
    ui->comboBox_enableUpnp->addItem("Enable", true);

    ui->comboBox_typeUpnp->clear();
    ui->comboBox_typeUpnp->addItem("Auto", 0);
    ui->comboBox_typeUpnp->addItem("Manual", 1);

    ui->tableView->setHeaderCheckable(false);
    ui->tableView->setRowCount(2);
    ui->tableView->setItemDelegateForColumn(1, new ItemButtonDelegate(QPixmap(":/common/common/edit.png"), this));
    connect(ui->tableView, SIGNAL(itemClicked(int, int)), this, SLOT(onItemClicked(int, int)));
    connect(ui->tableView, SIGNAL(itemDoubleClicked(int, int)), this, SLOT(onItemDoubleClicked(int, int)));

    connect(ui->comboBox_enableUpnp, SIGNAL(currentIndexChanged(int)), this, SLOT(slotEnableUPnP(int)));
    connect(ui->comboBox_typeUpnp, SIGNAL(currentIndexChanged(int)), this, SLOT(slotTypeUPnP(int)));

    pUPnP_Db = NULL;
    pUPnP_DbOri = NULL;
    pNetPortDb = NULL;
    pUPnPEidtDlg = NULL;
    refreshTimer = NULL;
}

void NetworkPageUPnp::gotoNetUPnPtab()
{
    if (pUPnP_Db == NULL) {
        pUPnP_Db = new struct upnp;
    }
    if (pUPnP_DbOri == NULL) {
        pUPnP_DbOri = new struct upnp;
    }
    if (pNetPortDb == NULL) {
        pNetPortDb = new struct network_more;
    }
    memset(pUPnP_DbOri, 0, sizeof(struct upnp));
    memset(pUPnP_DbOri, 0, sizeof(struct upnp));
    memset(pNetPortDb, 0, sizeof(struct network_more));

    read_upnp(SQLITE_FILE_NAME, pUPnP_Db);
    memcpy(pUPnP_DbOri, pUPnP_Db, sizeof(struct upnp));

    read_port(SQLITE_FILE_NAME, pNetPortDb);
    readUPnPConfig();

    if (refreshTimer == NULL) {
        refreshTimer = new QTimer(this);
        connect(refreshTimer, SIGNAL(timeout()), this, SLOT(slotRefreshUPnPTable()));
    }

    if (refreshTimer) {
        refreshTimer->start(4000);
    }
}

void NetworkPageUPnp::freeNetUPnPtab()
{
    if (pUPnP_Db) {
        delete pUPnP_Db;
        pUPnP_Db = nullptr;
    }
    if (pUPnP_DbOri) {
        delete pUPnP_DbOri;
        pUPnP_DbOri = nullptr;
    }
    if (pUPnP_DbOri) {
        delete pUPnP_DbOri;
        pUPnP_DbOri = nullptr;
    }
    if (pUPnPEidtDlg) {
        delete pUPnPEidtDlg;
        pUPnPEidtDlg = nullptr;
    }
    if (refreshTimer) {
        refreshTimer->stop();
        refreshTimer->deleteLater();
        refreshTimer = NULL;
    }
}

void NetworkPageUPnp::readUPnPConfig()
{
    ui->comboBox_enableUpnp->setCurrentIndex(ui->comboBox_enableUpnp->findData(pUPnP_Db->enable));
    ui->comboBox_typeUpnp->setCurrentIndex(ui->comboBox_typeUpnp->findData(pUPnP_Db->type));

    ui->tableView->setItemText(0, 0, "HTTP");
    ui->tableView->setItemText(0, 2, QString("%1").arg(pUPnP_Db->http_port));
    ui->tableView->setItemText(0, 3, QString("%1").arg(pNetPortDb->http_port));
    if (pUPnP_Db->http_status == 1) {
        ui->tableView->setItemText(0, 4, GET_TEXT("SYSTEMNETWORK/71147", "Valid"));
    } else {
        ui->tableView->setItemText(0, 4, GET_TEXT("SYSTEMNETWORK/71148", "Invalid"));
    }
    ui->tableView->setItemText(1, 0, "RTSP");
    ui->tableView->setItemText(1, 2, QString("%1").arg(pUPnP_Db->rtsp_port));
    ui->tableView->setItemText(1, 3, QString("%1").arg(pNetPortDb->rtsp_port));
    if (pUPnP_Db->rtsp_status == 1) {
        ui->tableView->setItemText(1, 4, GET_TEXT("SYSTEMNETWORK/71147", "Valid"));
    } else {
        ui->tableView->setItemText(1, 4, GET_TEXT("SYSTEMNETWORK/71148", "Invalid"));
    }
    slotEnableUPnP(pUPnP_Db->enable);
}

bool NetworkPageUPnp::saveUPnPConfig()
{
    if (ui->comboBox_enableUpnp->currentIndex() == 1 && ui->comboBox_typeUpnp->currentIndex() == 1 && pUPnP_Db->rtsp_port == pUPnP_Db->http_port) {
        ShowMessageBox(GET_TEXT("SYSTEMNETWORK/71107", "Port can not be the same."));
        return false;
    }
    pUPnP_Db->enable = ui->comboBox_enableUpnp->currentIndex();
    pUPnP_Db->type = ui->comboBox_typeUpnp->currentIndex();

    if (memcmp(pUPnP_Db, pUPnP_DbOri, sizeof(struct upnp)) != 0) {
        memcpy(pUPnP_DbOri, pUPnP_Db, sizeof(struct upnp));
        sendMessageOnly(REQUEST_FLAG_SET_UPNP, (void *)pUPnP_Db, sizeof(struct upnp));
    }
    return true;
}

void NetworkPageUPnp::resizeEvent(QResizeEvent *event)
{
    int tableWidth = ui->tableView->width();
    int unitWidth = tableWidth / ui->tableView->columnCount();
    for (int column = 0; column < ui->tableView->columnCount(); ++column) {
        ui->tableView->setColumnWidth(column, unitWidth);
    }
    QWidget::resizeEvent(event);
}

void NetworkPageUPnp::onLanguageChanged()
{
    QStringList headerList;
    headerList << GET_TEXT("SYSTEMNETWORK/71144", "Port Type");
    headerList << GET_TEXT("COMMON/1019", "Edit");
    headerList << GET_TEXT("SYSTEMNETWORK/71145", "External Port");
    headerList << GET_TEXT("SYSTEMNETWORK/71146", "Internal Port");
    headerList << GET_TEXT("MENU/10006", "Status");
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setSortingEnabled(false);
    ui->tableView->setColumnCount(headerList.size());
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->label_enableUpnp->setText(GET_TEXT("SYSTEMNETWORK/71142", "UPnP"));
    ui->label_typeUpnp->setText(GET_TEXT("SYSTEMNETWORK/71143", "Forwarding Type"));
    ui->comboBox_enableUpnp->setItemText(0, GET_TEXT("COMMON/1018", "Disable"));
    ui->comboBox_enableUpnp->setItemText(1, GET_TEXT("COMMON/1009", "Enable"));
    ui->comboBox_typeUpnp->setItemText(0, GET_TEXT("COMMON/1014", "Auto"));
    ui->comboBox_typeUpnp->setItemText(1, GET_TEXT("COMMON/1055", "Manual"));
}

void NetworkPageUPnp::slotRefreshUPnPTable()
{
    struct upnp UPnP;
    read_upnp(SQLITE_FILE_NAME, &UPnP);
    read_port(SQLITE_FILE_NAME, pNetPortDb);
    if (UPnP.enable != pUPnP_Db->enable || UPnP.type != pUPnP_Db->type) {
        memcpy(pUPnP_Db, &UPnP, sizeof(struct upnp));
        memcpy(pUPnP_DbOri, &UPnP, sizeof(struct upnp));
        readUPnPConfig();
    } else {
        if (UPnP.enable && ui->comboBox_typeUpnp->currentIndex() == 0) {
            pUPnP_Db->http_port = UPnP.http_port;
            pUPnP_DbOri->http_port = UPnP.http_port;
            pUPnP_Db->rtsp_port = UPnP.rtsp_port;
            pUPnP_DbOri->rtsp_port = UPnP.rtsp_port;

            ui->tableView->setItemText(0, 2, QString("%1").arg(pUPnP_Db->http_port));
            ui->tableView->setItemText(1, 2, QString("%1").arg(pUPnP_Db->rtsp_port));
        }

        pUPnP_Db->http_status = UPnP.http_status;
        pUPnP_DbOri->http_status = UPnP.http_status;
        pUPnP_Db->rtsp_status = UPnP.rtsp_status;
        pUPnP_DbOri->rtsp_status = UPnP.rtsp_status;

        if (pUPnP_Db->http_status == 1) {
            ui->tableView->setItemText(0, 4, GET_TEXT("SYSTEMNETWORK/71147", "Valid"));
        } else {
            ui->tableView->setItemText(0, 4, GET_TEXT("SYSTEMNETWORK/71148", "Invalid"));
        }
        if (pUPnP_Db->rtsp_status == 1) {
            ui->tableView->setItemText(1, 4, GET_TEXT("SYSTEMNETWORK/71147", "Valid"));
        } else {
            ui->tableView->setItemText(1, 4, GET_TEXT("SYSTEMNETWORK/71148", "Invalid"));
        }
        ui->tableView->setItemText(0, 3, QString("%1").arg(pNetPortDb->http_port));
        ui->tableView->setItemText(1, 3, QString("%1").arg(pNetPortDb->rtsp_port));
    }
}

void NetworkPageUPnp::onItemClicked(int row, int column)
{
    Q_UNUSED(column)

    if (!ui->tableView->isRowEnable(row)
        || column != ColumnEdit) {
        return;
    }

    QString strType;
    int port = 0;
    if (row == 0) {
        strType = "HTTP";
        port = ui->tableView->itemText(row, 2).toInt();

    } else if (row == 1) {
        strType = "RTSP";
        port = ui->tableView->itemText(row, 2).toInt();
    }

    UPnPEdit edit(this);
    edit.initializeData(strType, port);
    int result = edit.exec();
    if (result == QDialog::Accepted) {
        int port = edit.externalPort();
        ui->tableView->setItemIntValue(row, 2, port);
        if (row == 0) {
            pUPnP_Db->http_port = port;
        } else if (row == 1) {
            pUPnP_Db->rtsp_port = port;
        }
    }
}

void NetworkPageUPnp::slotEnableUPnP(int index)
{
    bool enable = ui->comboBox_enableUpnp->itemData(index).toBool();
    ui->comboBox_typeUpnp->setEnabled(enable);
    if (enable) {
        slotTypeUPnP(ui->comboBox_typeUpnp->currentIndex());
    } else {
        slotTypeUPnP(0);
    }
}

void NetworkPageUPnp::slotTypeUPnP(int index)
{
    bool manual = index == 1;
    for (int row = 0; row < ui->tableView->rowCount(); row++) {
        for (int column = 0; column < ui->tableView->columnCount(); column++) {
            ui->tableView->setItemEnable(row, column, manual);
        }
    }
    ui->tableView->clearSelection();
}

void NetworkPageUPnp::on_pushButton_apply_clicked()
{
    if (saveUPnPConfig()) {
        if (refreshTimer) {
            refreshTimer->start();
        }
        //showWait();
        QEventLoop eventloop;
        QTimer::singleShot(4000, &eventloop, SLOT(quit()));
        eventloop.exec();
        //closeWait();
        ShowMessageBox(GET_TEXT("SYSTEMNETWORK/71112", "Save successfully!"));
    }
}

void NetworkPageUPnp::on_pushButton_back_clicked()
{
    emit sig_back();
}

void NetworkPageUPnp::onItemDoubleClicked(int row, int column)
{
    Q_UNUSED(row);
    Q_UNUSED(column);

    if (column == ColumnEdit) {
        return;
    }
    if (ui->comboBox_typeUpnp->currentIndex() != 1
        || ui->comboBox_enableUpnp->currentIndex() != 1) {
        return;
    }

    QString strType;
    int port = 0;
    if (row == 0) {
        strType = "HTTP";
        port = ui->tableView->itemText(row, 2).toInt();

    } else if (row == 1) {
        strType = "RTSP";
        port = ui->tableView->itemText(row, 2).toInt();
    }

    UPnPEdit edit(this);
    edit.initializeData(strType, port);
    int result = edit.exec();
    if (result == QDialog::Accepted) {
        int port = edit.externalPort();
        ui->tableView->setItemIntValue(row, 2, port);
        if (row == 0) {
            pUPnP_Db->http_port = port;
        } else if (row == 1) {
            pUPnP_Db->rtsp_port = port;
        }
    }
}
