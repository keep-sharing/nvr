#include "TabSmartStatus.h"
#include "ui_TabSmartStatus.h"
#include "DiskSmartTest.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "centralmessage.h"
#include <QtDebug>

TabSmartStatus *TabSmartStatus::s_smartStatus = nullptr;

TabSmartStatus::TabSmartStatus(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::PageSmartStatus)
{
    ui->setupUi(this);

    s_smartStatus = this;

    QStringList headerList;
    headerList << GET_TEXT("DISKMANAGE/72022", "ID");
    headerList << GET_TEXT("DISKMANAGE/72023", "Attribute Name");
    headerList << GET_TEXT("DISKMANAGE/72024", "Value");
    headerList << GET_TEXT("DISKMANAGE/72025", "Worst");
    headerList << GET_TEXT("DISKMANAGE/72026", "Thresh");
    headerList << GET_TEXT("DISKMANAGE/72027", "Raw Value");
    headerList << GET_TEXT("MENU/10006", "Status");
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setHeaderCheckable(false);
    ui->tableView->setColumnCount(headerList.size());
    ui->tableView->setSortType(ColumnID, SortFilterProxyModel::SortInt);
    ui->tableView->setSortType(ColumnValue, SortFilterProxyModel::SortInt);
    ui->tableView->setSortType(ColumnWorst, SortFilterProxyModel::SortInt);
    ui->tableView->setSortType(ColumnThreshold, SortFilterProxyModel::SortInt);
    ui->tableView->setSortType(ColumnType, SortFilterProxyModel::SortInt);
    ui->tableView->setSortType(ColumnStatus, SortFilterProxyModel::SortInt);
    ui->comboBox_testType->clear();
    ui->comboBox_testType->addItem(GET_TEXT("DISKMANAGE/72052", "FAST"), 0);
    ui->comboBox_testType->addItem(GET_TEXT("DISKMANAGE/72053", "FULL"), 1);

    m_waitting = new MsWaitting(this);

    //
    connect(&gDiskSmartTest, SIGNAL(progressChanged(int, int)), this, SLOT(onProgressChanged(int, int)));

    onLanguageChanged();
}

TabSmartStatus::~TabSmartStatus()
{
    s_smartStatus = nullptr;
    delete ui;
}

TabSmartStatus *TabSmartStatus::instance()
{
    return s_smartStatus;
}

void TabSmartStatus::initializeData(QList<int> portList)
{
    //
    ui->tableView->clearSort();
    ui->comboBox_port->clear();
    for (int i = 0; i < portList.size(); ++i) {
        int port = portList.at(i);
        ui->comboBox_port->addItem(QString::number(port), port);
    }
    //
    int portIndex = ui->comboBox_port->currentIndex();
    if (gDiskSmartTest.testPort() != -1) {
        int port = gDiskSmartTest.testPort();
        if (gDiskSmartTest.isTesting()) {
            ui->widget_info->setEnabled(false);
        } else {
            ui->widget_info->setEnabled(true);
        }
        ui->lineEdit_testProcess->setText(QString("%1%").arg(gDiskSmartTest.testProgress()));
        portIndex = ui->comboBox_port->findData(port);
    } else {
        ui->widget_info->setEnabled(true);
    }
    ui->comboBox_port->setCurrentIndex(portIndex);
    on_comboBox_port_activated(portIndex);
}

void TabSmartStatus::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_SMART_ATTR:
        ON_RESPONSE_FLAG_GET_SMART_ATTR(message);
        break;
    case RESPONSE_FLAG_RET_SMART_TEST_START:
        ON_RESPONSE_FLAG_RET_SMART_TEST_START(message);
        break;
    }
}

void TabSmartStatus::ON_RESPONSE_FLAG_GET_SMART_ATTR(MessageReceive *message)
{
    qDebug() << QString("PageSmartStatus::ON_RESPONSE_FLAG_GET_SMART_ATTR");
    //m_waitting->//closeWait();

    struct resp_get_smart_attr *smart_attr = (struct resp_get_smart_attr *)(message->data);
    if (!smart_attr) {
        ui->tableView->clearContent();
        return;
    }

    int attr_size = smart_attr->attrsize;
    void *info_data = ms_malloc(attr_size);
    if (!info_data) {
        qWarning() << QString("PageSmartStatus::ON_RESPONSE_FLAG_GET_SMART_ATTR, ms_malloc failed.");
        return;
    }
    memcpy(info_data, smart_attr + (message->header.size - attr_size) / sizeof(struct resp_get_smart_attr), attr_size);

    struct smart_info *smartInfo = (struct smart_info *)info_data;
    int info_size = attr_size / sizeof(struct smart_info);
    if (smartInfo == NULL) {
        if (info_data) {
            ms_free(info_data);
            info_data = NULL;
        }
        ui->tableView->clearContent();
        return;
    }
    ui->tableView->setRowCount(info_size);
    for (int row = 0; row < info_size; row++) {
        const smart_info &info = smartInfo[row];
        ui->tableView->setItemText(row, ColumnID, QString("%1").arg(info.id, 2, 16, QLatin1Char('0')).toUpper());
        ui->tableView->setItemData(row, ColumnID, info.id, SortIntRole);
        ui->tableView->setItemText(row, ColumnName, info.name);
        ui->tableView->setItemIntValue(row, ColumnValue, info.val);
        ui->tableView->setItemIntValue(row, ColumnWorst, info.worst);
        ui->tableView->setItemIntValue(row, ColumnThreshold, info.thresh);
        ui->tableView->setItemIntValue(row, ColumnType, QString("%1").arg(info.raw_val).toInt());
        switch (info.status) {
        case STATUS_GOOD:
            ui->tableView->setItemText(row, ColumnStatus, GET_TEXT("COMMON/1001", "OK"));
            break;
        case STATUS_WARN:
            ui->tableView->setItemText(row, ColumnStatus, GET_TEXT("DISKMANAGE/72062", "Warning"));
            ui->tableView->setRowColor(row, QColor(153, 102, 0));
            break;
        case STATUS_BAD:
            ui->tableView->setItemText(row, ColumnStatus, GET_TEXT("DISKMANAGE/72063", "Failure"));
            ui->tableView->setRowColor(row, QColor(255, 0, 0));
            break;
        case STATUS_FAIL:
            ui->tableView->setItemText(row, ColumnStatus, GET_TEXT("DISKMANAGE/72042", "Unknow Failure"));
            break;
        default:
            ui->tableView->setItemText(row, ColumnStatus, GET_TEXT("DISKMANAGE/72042", "Unknow Failure"));
            break;
        }
        ui->tableView->setItemData(row, ColumnStatus, info.status, SortIntRole);
    }
    ui->tableView->reorder();
    if (info_data) {
        ms_free(info_data);
        info_data = NULL;
    }
    //
    const smart_status &smartStatus = smart_attr->status;
    ui->lineEdit_temperature->setText(QString("%1").arg(smartStatus.temperature));
    ui->lineEdit_uptime->setText(QString("%1").arg(smartStatus.uptime));
    QString strStatus;
    switch (smartStatus.status) {
    case STATUS_GOOD:
        strStatus = GET_TEXT("DISKMANAGE/72039", "In good condition");
        break;
    case STATUS_WARN:
        strStatus = GET_TEXT("DISKMANAGE/72040", "Unstable or damaged sectors");
        break;
    case STATUS_BAD:
        strStatus = GET_TEXT("DISKMANAGE/72041", "Hard disk errors");
        break;
    case STATUS_FAIL:
        strStatus = GET_TEXT("DISKMANAGE/72042", "Unknown Failure");
        break;
    }
    ui->lineEdit_allEvaluation->setText(strStatus);
    if (QString(smartStatus.result).contains("PASSED")) {
        ui->lineEdit_safeEvaluation->setText(GET_TEXT("DISKMANAGE/72037", "PASSED"));
    } else {
        ui->lineEdit_safeEvaluation->setText(GET_TEXT("DISKMANAGE/72038", "FAILED"));
    }
}

void TabSmartStatus::ON_RESPONSE_FLAG_RET_SMART_TEST_START(MessageReceive *message)
{
    if (!message->data) {
        qWarning() << "PageSmartStatus::ON_RESPONSE_FLAG_RET_SMART_TEST_START, data is null.";
        return;
    }
    qDebug() << QString("PageSmartStatus::ON_RESPONSE_FLAG_RET_SMART_TEST_START");
    int ret = (*(int *)(message->data));
    if (ret == 0) {
        gDiskSmartTest.startGetProcess();
    } else {
        gDiskSmartTest.clearTestPort();
        ui->widget_info->setEnabled(true);
        ShowMessageBox(GET_TEXT("DISKMANAGE/72043", "S.M.A.R.T Test Start Failed."));
    }
}

void TabSmartStatus::resizeEvent(QResizeEvent *event)
{
    int tableWidth = ui->tableView->width();
    int columnWidth = (tableWidth - 50) / ui->tableView->columnCount();
    for (int i = 0; i < ui->tableView->columnCount(); ++i) {
        ui->tableView->setColumnWidth(i, columnWidth);
    }
    QWidget::resizeEvent(event);
}

void TabSmartStatus::onLanguageChanged()
{
    ui->label_port->setText(GET_TEXT("DISKSTATUS/66000", "Port"));
    ui->label_testProcess->setText(GET_TEXT("DISKSTATUS/66001", "Test Process"));
    ui->label_testType->setText(GET_TEXT("DISKSTATUS/66002", "Test Type"));
    ui->label_temperature->setText(GET_TEXT("DISKMANAGE/72032", "Temperature(℃)"));
    ui->label_uptime->setText(GET_TEXT("DISKSTATUS/66003", "Uptime"));
    ui->label_selfEvaluation->setText(GET_TEXT("DISKSTATUS/66004", "Self-evaluation"));
    ui->label_allEvaluation->setText(GET_TEXT("DISKSTATUS/66005", "All-evaluation"));
    ui->label_test->setText(GET_TEXT("DISKSTATUS/66006", "S.M.A.R.T Test"));
    ui->pushButton_test->setText(GET_TEXT("DISKSTATUS/66007", "Test"));
}

void TabSmartStatus::onProgressChanged(int port, int progress)
{
    Q_UNUSED(port)

    if (!isVisible()) {
        return;
    }

    ui->lineEdit_testProcess->setText(QString("%1%").arg(progress));
    if (progress >= 100) {
        on_comboBox_port_activated(ui->comboBox_port->currentIndex());
        ui->widget_info->setEnabled(true);
    }
}

void TabSmartStatus::on_comboBox_port_activated(int index)
{
    ui->tableView->clearContent();
    ui->lineEdit_temperature->clear();
    ui->lineEdit_uptime->clear();
    ui->lineEdit_allEvaluation->clear();
    ui->lineEdit_safeEvaluation->clear();

    if (index < 0) {
        return;
    }

    int port = ui->comboBox_port->itemData(index).toInt();
    if (gDiskSmartTest.testPort() != port) {
        ui->lineEdit_testProcess->clear();
    }
    sendMessage(REQUEST_FLAG_GET_SMART_ATTR, &port, sizeof(int));
    //m_waitting->//showWait();
}

void TabSmartStatus::on_pushButton_test_clicked()
{
    struct req_smart_test_start smartTest;
    memset(&smartTest, 0, sizeof(req_smart_test_start));
    smartTest.port = ui->comboBox_port->currentData().toInt();
    smartTest.type = ui->comboBox_testType->currentData().toInt();

    qDebug() << QString("PageSmartStatus::on_pushButton_test_clicked, REQUEST_FLAG_SMART_TEST_START, port: %1").arg(smartTest.port);
    sendMessage(REQUEST_FLAG_SMART_TEST_START, (void *)&smartTest, sizeof(smartTest));
    ui->widget_info->setEnabled(false);

    //
    gDiskSmartTest.setTestPort(smartTest.port);
}
