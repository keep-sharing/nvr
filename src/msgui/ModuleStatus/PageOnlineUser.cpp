#include "PageOnlineUser.h"
#include "ui_PageOnlineUser.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "centralmessage.h"

extern "C" {
#include "msdb.h"
}

PageOnlineUser::PageOnlineUser(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::OnlineUser)
{
    ui->setupUi(this);

    //表头
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("ONLINEUSER/74101", "No.");
    headerList << GET_TEXT("ONLINEUSER/74102", "User Name");
    headerList << GET_TEXT("ONLINEUSER/74103", "User Level");
    headerList << GET_TEXT("ONLINEUSER/74104", "IP Address");
    headerList << GET_TEXT("ONLINEUSER/74105", "User Login Time");
    headerList << GET_TEXT("ONLINEUSER/74107", "Add to Access Filter");
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setColumnCount(headerList.size());
    ui->tableView->hideColumn(ColumnCheck);
    //列宽
    ui->tableView->setColumnWidth(ColumnIndex, 130);
    ui->tableView->setColumnWidth(ColumnUserName, 250);
    ui->tableView->setColumnWidth(ColumnUserLevel, 250);
    ui->tableView->setColumnWidth(ColumnIP, 250);
    ui->tableView->setColumnWidth(ColumnUserLoginTime, 390);

    ui->pushButton_refresh->setText(GET_TEXT("ONLINEUSER/74106", "Refresh"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
    //用代理显示图片
    if (qMsNvr->isSlaveMode()) {
        ui->tableView->setItemDelegateForColumn(ColumnAddToAccessFilter, new ItemButtonDelegate(QPixmap(":/onlineuser/onlineuser/add_gray_disable.png"), this));
    } else {
        ui->tableView->setItemDelegateForColumn(ColumnAddToAccessFilter, new ItemButtonDelegate(QPixmap(":/onlineuser/onlineuser/add_gray.png"), this));
    }

    connect(ui->tableView, SIGNAL(itemClicked(int, int)), this, SLOT(onTableClicked(int, int)));
}

PageOnlineUser::~PageOnlineUser()
{
    delete ui;
}

void PageOnlineUser::initializeData()
{
    ui->tableView->clearSort();
    on_pushButton_refresh_clicked();
}

void PageOnlineUser::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_ONLINE_USER:
        ON_RESPONSE_FLAG_GET_ONLINE_USER(message);
        break;
    }
}

void PageOnlineUser::ON_RESPONSE_FLAG_GET_ONLINE_USER(MessageReceive *message)
{
    ms_online_user_info *user_info_array = static_cast<ms_online_user_info *>(message->data);

    ui->tableView->clearContent();
    if (user_info_array) {
        int count = message->header.size / sizeof(ms_online_user_info);
        ui->tableView->setRowCount(count);
        for (int i = 0; i < count; ++i) {
            const ms_online_user_info &user_info = user_info_array[i];
            ui->tableView->setItemText(i, ColumnIndex, QString("%1").arg(i + 1));
            ui->tableView->setItemText(i, ColumnUserName, QString(user_info.userName));
            QString strLevel;
            switch (user_info.userLevel) {
            case USERLEVEL_ADMIN:
                strLevel = "Admin";
                break;
            case USERLEVEL_OPERATOR:
                strLevel = "Operator";
                break;
            case USERLEVEL_USER:
                strLevel = "Viewer";
                break;
            case USERLEVEL_LOCAL:
                strLevel = "Local";
                break;
            default:
                break;
            }
            ui->tableView->setItemText(i, ColumnUserLevel, strLevel);
            ui->tableView->setItemText(i, ColumnIP, QString(user_info.ipaddr));
            ui->tableView->setItemText(i, ColumnUserLoginTime, QString(user_info.loginTime));
        }
    }
    ui->tableView->reorder();
    //MsWaitting::closeGlobalWait();
}

void PageOnlineUser::on_pushButton_refresh_clicked()
{
    sendMessage(REQUEST_FLAG_GET_ONLINE_USER, nullptr, 0);
    //MsWaitting::showGlobalWait();
}

void PageOnlineUser::on_pushButton_back_clicked()
{
    back();
}
/**
* @description      点击按钮后检测IP地址是否已经存在,若存在弹窗报错,若不存在添加进Access Filter
* @date                 2021.03.01
* @author:              Kirin
*/
void PageOnlineUser::onTableClicked(int row, int column)
{
    if (ColumnAddToAccessFilter == column && !qMsNvr->isSlaveMode()) {
        QString address = ui->tableView->itemText(row, ColumnIP);
        struct access_list info[MAX_LIMIT_ADDR];
        int cnt = 0;
        read_access_filter(SQLITE_FILE_NAME, info, &cnt);

        for (int i = 0; i < cnt; ++i) {
            if (QString(info[i].address) == address) {
                ShowMessageBox(GET_TEXT("ACCESSFILTER/81009", "The same address has already existed, cannot add it twice."));
                return;
            }
        }

        QList<access_list> filterList;
        for (int i = 0; i < cnt; ++i) {
            if (info[i].type >= ADDRESS_TYPE_MAC && info[i].type < ADDRESS_TYPE_MAX) {
                filterList.append(info[i]);
            }
        }
        access_list tmp;
        tmp.type = ADDRESS_TYPE_IP_SINGLE;
        snprintf(tmp.address, sizeof(tmp.address), "%s", address.toStdString().c_str());
        filterList.append(tmp);
        cnt = filterList.size();
        if (cnt >= MAX_LIMIT_ADDR) {
            ShowMessageBox(QString("max list:[%1]").arg(MAX_LIMIT_ADDR));
            return;
        }
        memset(&info, 0, sizeof(struct access_list) * MAX_LIMIT_ADDR);
        for (int i = 0; i < cnt; ++i) {
            memcpy(&info[i], &filterList.at(i), sizeof(struct access_list));
        }
        write_access_filter(SQLITE_FILE_NAME, info, cnt);
        sendMessageOnly(REQUEST_FLAG_SET_ACCESS_FILTER, nullptr, 0);
        ShowMessageBox(GET_TEXT("CAMERAMAINTENANCE/38013", "Add successfully."));
    }
}
