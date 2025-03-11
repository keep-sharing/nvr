#include "AccessFilter.h"
#include "ui_AccessFilter.h"
#include "EditAccessFilter.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"

extern "C" {
#include "msdb.h"
}

AccessFilter::AccessFilter(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::AccessFilter)
{
    ui->setupUi(this);

    ui->comboBox_filterEnable->clear();
    ui->comboBox_filterEnable->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBox_filterEnable->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);
    ui->comboBox_filterType->clear();
    ui->comboBox_filterType->addItem(GET_TEXT("ACCESSFILTER/81002", "Deny"), 0);
    ui->comboBox_filterType->addItem(GET_TEXT("ACCESSFILTER/81003", "Allow"), 1);

    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("PTZCONFIG/36047", "Address");
    headerList << GET_TEXT("COMMON/1019", "Edit");
    headerList << GET_TEXT("CHANNELMANAGE/30023", "Delete");

    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setColumnCount(headerList.size());
    //相关信号和槽
    connect(ui->tableView, SIGNAL(itemClicked(int, int)), this, SLOT(onTableItemClicked(int, int)));
    //用代理显示图片
    ui->tableView->setItemDelegateForColumn(ColumnEdit, new ItemButtonDelegate(QPixmap(":/common/common/edit.png"), this));
    ui->tableView->setItemDelegateForColumn(ColumnDelete, new ItemButtonDelegate(QPixmap(":/common/common/delete.png"), this));
    //排序
    ui->tableView->setSortableForColumn(ColumnCheck, false);
    ui->tableView->setSortableForColumn(ColumnAddress, false);
    ui->tableView->setSortableForColumn(ColumnEdit, false);
    ui->tableView->setSortableForColumn(ColumnDelete, false);

    //列宽
    ui->tableView->setColumnWidth(ColumnCheck, 50);
    ui->tableView->setColumnWidth(ColumnAddress, 1000);
    ui->tableView->setColumnWidth(ColumnEdit, 220);
    ui->tableView->setColumnWidth(ColumnDelete, 220);

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

AccessFilter::~AccessFilter()
{
    delete ui;
}

void AccessFilter::gotoAccessFilterPage()
{
    int enable = get_param_int(SQLITE_FILE_NAME, PARAM_ACCESS_FILTER_ENABLE, 0);
    ui->comboBox_filterEnable->setCurrentIndex(enable);
    int type = get_param_int(SQLITE_FILE_NAME, PARAM_ACCESS_FILTER_TYPE, 1);
    ui->comboBox_filterType->setCurrentIndex(type);

    struct access_list info[MAX_LIMIT_ADDR];
    int cnt = 0;
    read_access_filter(SQLITE_FILE_NAME, info, &cnt);
    m_filterList.clear();

    int i;
    for (i = 0; i < cnt; ++i) {
        if (info[i].type >= ADDRESS_TYPE_MAC && info[i].type < ADDRESS_TYPE_MAX)
            m_filterList.append(info[i]);
    }

    ui->tableView->clearContent();
    int row = 0;
    for (i = 0; i < m_filterList.size(); ++i) {
        ui->tableView->setItemData(row, ColumnCheck, false, ItemCheckedRole);
        ui->tableView->setItemText(row, ColumnAddress, m_filterList.at(i).address);
        row++;
    }
    ui->tableView->setRowCount(row);

    qMsDebug() << QString("enable:[%1], type:[%2], cnt:[%3]").arg(enable).arg(type).arg(cnt);
}

void AccessFilter::initializeData()
{
    gotoAccessFilterPage();
}

void AccessFilter::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void AccessFilter::onLanguageChanged()
{
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("PTZCONFIG/36047", "Address");
    headerList << GET_TEXT("COMMON/1019", "Edit");
    headerList << GET_TEXT("CHANNELMANAGE/30023", "Delete");

    ui->tableView->setHorizontalHeaderLabels(headerList);
    //
    ui->label_accessFilter->setText(GET_TEXT("ACCESSFILTER/81000", "Access Filter"));
    ui->label_filterType->setText(GET_TEXT("ACCESSFILTER/81001", "Filter Type"));

    ui->comboBox_filterEnable->setItemText(0, GET_TEXT("COMMON/1018", "Disable"));
    ui->comboBox_filterEnable->setItemText(1, GET_TEXT("COMMON/1009", "Enable"));

    ui->comboBox_filterType->setItemText(0, GET_TEXT("ACCESSFILTER/81003", "Deny"));
    ui->comboBox_filterType->setItemText(1, GET_TEXT("ACCESSFILTER/81002", "Allow"));

    ui->pushButton_add->setText(GET_TEXT("COMMON/1000", "Add"));
    ui->pushButton_delete->setText(GET_TEXT("CHANNELMANAGE/30023", "Delete"));
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
}

#if 0
void AccessFilter::on_comboBox_filterType_activated(int index)
{
    ui->tableView->clearContent();
    int row = 0;
    for(int i = 0; i < m_filterList.size(); ++i)
    {
        MsDebug() << QString("index:[%1], m_filterList.at(%2).filter_type:[%3]address:[%4]").arg(index).arg(i).arg(m_filterList.at(i).filter_type).arg(m_filterList.at(i).address);
        if(index == m_filterList.at(i).filter_type)
        {
            ui->tableView->setItemData(row, ColumnCheck, false, ItemCheckedRole);
            ui->tableView->setItemText(row, ColumnAddress, m_filterList.at(i).address);
            row++;
        }
    }
    ui->tableView->setRowCount(row);
}
#endif

void AccessFilter::deleteRow(int row)
{
    QString str = ui->tableView->itemText(row, ColumnAddress);
    //    int type = ui->comboBox_filterType->currentIndex();
    ui->tableView->removeRow(row);
    for (int i = row; i < m_filterList.length(); i++) {
        if (str == m_filterList.at(i).address) {
            m_filterList.removeAt(i);
        }
    }
}

void AccessFilter::onTableItemClicked(int row, int column)
{
    //    MsDebug() << QString("row:[%1], column:[%2], str:[%3]").arg(row).arg(column).arg(str);

    if (column == ColumnEdit) {
        QString address = ui->tableView->itemText(row, ColumnAddress);
        int i;
        for (i = row; i < m_filterList.length(); i++) {
            if (address == m_filterList.at(i).address) {
                break;
            }
        }
        ADDRESS_TYPE address_type = ADDRESS_TYPE_MAX;
        EditAccessFilter editAddress(this);
        EditAccessFilter::instance()->initializeData(i, m_filterList); //edit address
        editAddress.exec();

        address = EditAccessFilter::instance()->getAddress();
        if (address.contains(":")) {
            address_type = ADDRESS_TYPE_MAC;
        } else if (address.contains("-")) {
            address_type = ADDRESS_TYPE_IP_RANGE;
        } else if (address.contains(".")) {
            address_type = ADDRESS_TYPE_IP_SINGLE;
        } else {
            address_type = ADDRESS_TYPE_MAX;
        }
        qMsDebug() << QString("before: address_type:[%1], address:[%2];    AddressType:[%3], Address:[%4]").arg(address_type).arg(address).arg(m_filterList.at(i).type).arg(m_filterList.at(i).address);
        if (address_type != ADDRESS_TYPE_MAX) {
            access_list tmp;
            tmp.type = address_type;
            snprintf(tmp.address, sizeof(tmp.address), "%s", address.toStdString().c_str());
            m_filterList.replace(i, tmp);
            ui->tableView->setItemText(row, ColumnAddress, address);
        }
        qMsDebug() << QString("after: AddressType:[%1], Address:[%2]").arg(m_filterList.at(i).type).arg(m_filterList.at(i).address);
    } else if (column == ColumnDelete) {
        deleteRow(row);
    }
}

void AccessFilter::on_pushButton_add_clicked()
{
    qMsDebug() << QString("on_pushButton_add_clicked  length:[%1]  row:[%2]").arg(m_filterList.length()).arg(ui->tableView->rowCount());
    if (m_filterList.length() >= MAX_LIMIT_ADDR) {
        ShowMessageBox(QString("max list:[%1]").arg(MAX_LIMIT_ADDR));
        return;
    }

    ADDRESS_TYPE address_type = ADDRESS_TYPE_MAX;
    EditAccessFilter addAddress(this);
    EditAccessFilter::instance()->initializeData(m_filterList.length(), m_filterList); //add address
    addAddress.exec();

    QString address = EditAccessFilter::instance()->getAddress();
    if (address.contains(":")) {
        address_type = ADDRESS_TYPE_MAC;
    } else if (address.contains("-")) {
        address_type = ADDRESS_TYPE_IP_RANGE;
    } else if (address.contains(".")) {
        address_type = ADDRESS_TYPE_IP_SINGLE;
    } else {
        address_type = ADDRESS_TYPE_MAX;
    }
    if (address_type != ADDRESS_TYPE_MAX) {
        access_list tmp;
        tmp.type = address_type;
        snprintf(tmp.address, sizeof(tmp.address), "%s", address.toStdString().c_str());
        m_filterList.append(tmp);
        int row = ui->tableView->rowCount();
        ui->tableView->setItemData(row, ColumnCheck, false, ItemCheckedRole);
        ui->tableView->setItemText(row, ColumnAddress, address);
        row++;
        ui->tableView->setRowCount(row);
        qMsDebug() << QString("address:[%1], row:[%2]").arg(address).arg(row);
    }
    ui->pushButton_add->setAttribute(Qt::WA_UnderMouse, false);
    ui->pushButton_add->clearFocus();
}

void AccessFilter::on_pushButton_delete_clicked()
{
    //    MsDebug() << QString("on_pushButton_delete_clicked  length:[%1]").arg(m_filterList.length());
    for (int row = ui->tableView->rowCount(); row >= 0; --row) //选择视图表的一行
    {
        if (ui->tableView->isItemChecked(row)) {
            deleteRow(row);
        }
    }
    ui->tableView->setHeaderChecked(false);
}

void AccessFilter::on_pushButton_apply_clicked()
{
    int enable = ui->comboBox_filterEnable->currentIndex();
    set_param_int(SQLITE_FILE_NAME, PARAM_ACCESS_FILTER_ENABLE, enable);
    int type = ui->comboBox_filterType->currentIndex();
    set_param_int(SQLITE_FILE_NAME, PARAM_ACCESS_FILTER_TYPE, type);

    struct access_list info[MAX_LIMIT_ADDR];
    memset(info, 0, sizeof(info));
    int cnt = m_filterList.length();
    for (int i = 0; i < cnt; ++i) {
        memcpy(&info[i], &m_filterList.at(i), sizeof(struct access_list));
    }
    write_access_filter(SQLITE_FILE_NAME, info, cnt);
    sendMessageOnly(REQUEST_FLAG_SET_ACCESS_FILTER, nullptr, 0);
    qMsDebug() << QString("on_pushButton_apply_clicked  length:[%1], info[0].address:[%2]").arg(cnt).arg(info[0].address);
}

void AccessFilter::on_pushButton_back_clicked()
{
    emit sig_back();
}
