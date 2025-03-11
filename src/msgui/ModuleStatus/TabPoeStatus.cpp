#include "TabPoeStatus.h"
#include "ui_TabPoeStatus.h"
#include "centralmessage.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"

TabPoeStatus::TabPoeStatus(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::PagePoeStatus)
{
    ui->setupUi(this);

    QStringList headerList;
    headerList << GET_TEXT("CAMERASTATUS/62018", "PoE Port");
    headerList << GET_TEXT("COMMON/1033", "IP Address");
    headerList << GET_TEXT("CAMERASTATUS/62019", "Current Power Consumption");
    headerList << GET_TEXT("MENU/10006", "Status");
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setHeaderCheckable(false);
    ui->tableView->setColumnCount(headerList.size());
    ui->tableView->setSortType(ColumnPort, SortFilterProxyModel::SortInt);
    ui->tableView->setSortType(ColumnIP, SortFilterProxyModel::SortIP);

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimerout()));
    m_timer->setInterval(5000);

    m_waitting = new MsWaitting(this);

    onLanguageChanged();
}

TabPoeStatus::~TabPoeStatus()
{
    delete ui;
}

void TabPoeStatus::initializeData()
{
    m_poeCount = qMsNvr->poeCount();
    switch (m_poeCount) {
    case 4:
        m_totalPower = 40;
        break;
    case 8:
        m_totalPower = 120;
        break;
    case 16:
    case 24:
        m_totalPower = 200;
        break;
    default:
        m_totalPower = 0;
        break;
    }
    QString strTotalPower = QString::number(m_totalPower, 'f', 2);
    ui->label_note->setText(GET_TEXT("CAMERASTATUS/62017", "<html><head/><body><p>Note:</p><p>1.The rated power consumption of all PoE ports is %1W.</p><p>2.When the total power consumption exceeds the rated value, the system will close PoE ports in the order of channel numbers from large to small until the total power is less than the rated power.</p><p><br/></p></body></html>").arg(strTotalPower));

    sendMessage(REQUEST_FLAG_GET_POE_STATE, (void *)NULL, 0);
    //m_waitting->//showWait();
}

void TabPoeStatus::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_POE_STATE:
        ON_RESPONSE_FLAG_GET_POE_STATE(message);
        break;
    }
}

void TabPoeStatus::ON_RESPONSE_FLAG_GET_POE_STATE(MessageReceive *message)
{
    //m_waitting->//closeWait();

    struct resq_poe_state *poe_state = (struct resq_poe_state *)message->data;
    if (!poe_state) {
        return;
    }

    ui->tableView->clearContent();
    qreal usedPower = 0;
    int row = 0;
    for (int i = 0; i < poe_state->port_num; ++i) {
        if (row >= m_poeCount) {
            break;
        }

        ui->tableView->setItemIntValue(row, ColumnPort, i + 1);
        qreal power = poe_state->poe_port_power[i];
        if (power <= 0) {
            ui->tableView->setItemText(row, ColumnIP, "-");
            ui->tableView->setItemText(row, ColumnPower, "-");
            ui->tableView->setItemText(row, ColumnStatus, "-");
        } else {
            QString strIP = poe_state->ipaddr[i];
            if (strIP.isEmpty()) {
                strIP = GET_TEXT("CAMERASTATUS/62021", "Network Error");
            }
            QString strPower = QString(GET_TEXT("CAMERASTATUS/62020", "%1W").arg(power, 0, 'f', 2));
            QString strState = poe_state->connected[i] ? GET_TEXT("CHANNELMANAGE/30038", "Connected") : GET_TEXT("CHANNELMANAGE/30039", "Disconnected");
            ui->tableView->setItemText(row, ColumnIP, strIP);
            ui->tableView->setItemText(row, ColumnPower, strPower);
            ui->tableView->setItemText(row, ColumnStatus, strState);
        }
        usedPower += power;
        row++;
    }
    ui->tableView->reorder();

    ui->lineEdit_totalPower->setText(GET_TEXT("CAMERASTATUS/62020", "%1W").arg(usedPower, 0, 'f', 2));
    ui->lineEdit_remainPower->setText(GET_TEXT("CAMERASTATUS/62020", "%1W").arg(m_totalPower - usedPower, 0, 'f', 2));
}

void TabPoeStatus::resizeEvent(QResizeEvent *event)
{
    int tableWidth = ui->tableView->width();
    int columnWidth = (tableWidth - 50) / ui->tableView->columnCount();
    for (int i = 0; i < ui->tableView->columnCount(); ++i) {
        ui->tableView->setColumnWidth(i, columnWidth);
    }
    QWidget::resizeEvent(event);
}

void TabPoeStatus::showEvent(QShowEvent *event)
{
    if (!m_timer->isActive()) {
        m_timer->start();
    }

    QWidget::showEvent(event);
}

void TabPoeStatus::hideEvent(QHideEvent *event)
{
    m_timer->stop();

    QWidget::hideEvent(event);
}

void TabPoeStatus::onLanguageChanged()
{
    ui->label_totalPower->setText(GET_TEXT("CAMERASTATUS/62015", "Total Power Consumption"));
    ui->label_remainPower->setText(GET_TEXT("CAMERASTATUS/62016", "Remaining Power Consumption"));
}

void TabPoeStatus::onTimerout()
{
    sendMessage(REQUEST_FLAG_GET_POE_STATE, (void *)NULL, 0);
    //m_waitting->//showWait();
}
