#include "HolidaySetting.h"
#include "ui_HolidaySetting.h"
#include "HolidayEdit.h"
#include "MsLanguage.h"
#include "MsMessage.h"
#include "MyDebug.h"
#include "RunnableHolidayData.h"
#include <QThreadPool>

HolidaySetting::HolidaySetting(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::HolidaySetting)
{
    ui->setupUi(this);

    QStringList headerList;
    headerList << GET_TEXT("HOLIDAY/73000", "ID");
    headerList << GET_TEXT("HOLIDAY/73001", "Holiday Name");
    headerList << GET_TEXT("DISKMANAGE/72109", "Status");
    headerList << GET_TEXT("HOLIDAY/73003", "Start Date");
    headerList << GET_TEXT("HOLIDAY/73004", "End Date");
    headerList << GET_TEXT("COMMON/1019", "Edit");
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setSortingEnabled(false);
    ui->tableView->setColumnCount(headerList.size());
    ui->tableView->setHeaderCheckable(false);
    ui->tableView->setItemDelegateForColumn(ColumnEdit, new ItemButtonDelegate(QPixmap(":/common/common/edit.png"), this));
    connect(ui->tableView, SIGNAL(itemClicked(int, int)), this, SLOT(onTableItemClicked(int, int)));

    m_dbHolidayList = new holiday[MAX_HOLIDAY];
    m_holidayEdit = new HolidayEdit(this);
    onLanguageChanged();
}

HolidaySetting::~HolidaySetting()
{
    delete[] m_dbHolidayList;
    delete ui;
}

void HolidaySetting::initializeData()
{
    memset(m_dbHolidayList, 0, sizeof(holiday) * MAX_HOLIDAY);
    int count = 0;
    read_holidays(SQLITE_FILE_NAME, m_dbHolidayList, &count);

    ui->tableView->clearContent();
    ui->tableView->setRowCount(MAX_HOLIDAY);
    for (int row = 0; row < MAX_HOLIDAY; ++row) {
        refreshTable(row);
    }
}

void HolidaySetting::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void HolidaySetting::resizeEvent(QResizeEvent *event)
{
    int columnWidth = (ui->tableView->width() - 50) / ui->tableView->columnCount();
    for (int column = 0; column < ui->tableView->columnCount(); ++column) {
        ui->tableView->setColumnWidth(column, columnWidth);
    }
    QWidget::resizeEvent(event);
}

QString HolidaySetting::weekDateString(int month, int week, int day) const
{
    static QString monthList[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
    static QString weekList[] = { "First", "Second", "Third", "Fourth", "Last" };
    static QString dayList[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
    QString result = QString("The %1 %2 in %3")
                         .arg(week >= 0 && week <= 6 ? weekList[week - 1] : QString())
                         .arg(day >= 0 && day < 7 ? dayList[day] : QString())
                         .arg(month > 0 && month < 13 ? monthList[month - 1] : QString());
    return result;
}

void HolidaySetting::refreshTable(int row)
{
    const holiday &day = m_dbHolidayList[row];
    ui->tableView->setItemIntValue(row, ColumnID, row + 1);
    ui->tableView->setItemText(row, ColumnName, day.name);
    ui->tableView->setItemText(row, ColumnStatus, day.enable == 1 ? GET_TEXT("COMMON/1009", "Enable") : GET_TEXT("COMMON/1018", "Disable"));
    QString startDate, endDate;
    switch (day.type) {
    case 0:
        startDate = QString("%1-%2-%3").arg(day.start_year).arg(day.start_mon).arg(day.start_mday);
        endDate = QString("%1-%2-%3").arg(day.end_year).arg(day.end_mon).arg(day.end_mday);
        break;
    case 1:
        startDate = QString("%1-%2").arg(day.start_mon).arg(day.start_mday);
        endDate = QString("%1-%2").arg(day.end_mon).arg(day.end_mday);
        break;
    case 2:
        startDate = weekDateString(day.start_mon, day.start_mweek, day.start_wday);
        endDate = weekDateString(day.end_mon, day.end_mweek, day.end_wday);
        break;
    }
    ui->tableView->setItemText(row, ColumnStartDate, startDate);
    ui->tableView->setItemText(row, ColumnEndData, endDate);
}

void HolidaySetting::onLanguageChanged()
{
    QStringList headerList;
    headerList << GET_TEXT("HOLIDAY/73000", "ID");
    headerList << GET_TEXT("HOLIDAY/73001", "Holiday Name");
    headerList << GET_TEXT("DISKMANAGE/72109", "Status");
    headerList << GET_TEXT("HOLIDAY/73003", "Start Date");
    headerList << GET_TEXT("HOLIDAY/73004", "End Date");
    headerList << GET_TEXT("COMMON/1019", "Edit");
    ui->tableView->setHorizontalHeaderLabels(headerList);

    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButtonApply->setText(GET_TEXT("COMMON/1003", "Apply"));
}

void HolidaySetting::onSaveFinished()
{
    sendMessageOnly(REQUEST_FLAG_SET_HOLIDAY, (void *)NULL, 0);
    //closeWait();
}

void HolidaySetting::onTableItemClicked(int row, int column)
{
    switch (column) {
    case ColumnEdit: {
        holiday &holidayInfo = m_dbHolidayList[row];
        m_holidayEdit->showHolidayEdit(&holidayInfo);
        m_holidayEdit->exec();
        refreshTable(row);
        break;
    }
    default:
        break;
    }
}

void HolidaySetting::on_pushButtonApply_clicked()
{
    //showWait();
    QThreadPool::globalInstance()->start(new RunnableHolidayData(m_dbHolidayList, this, "onSaveFinished"));
}

void HolidaySetting::on_pushButton_back_clicked()
{
    emit sig_back();
}
