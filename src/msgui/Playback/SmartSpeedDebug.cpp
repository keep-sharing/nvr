#include "SmartSpeedDebug.h"
#include "ui_SmartSpeedDebug.h"

SmartSpeedDebug::SmartSpeedDebug(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::SmartSpeedDebug)
{
    ui->setupUi(this);

    setTitleWidget(ui->widget_title);
    setWindowModality(Qt::NonModal);

    ui->tableWidget_general->setColumnCount(2);
    ui->tableWidget_general->setColumnWidth(0, 400);
    ui->tableWidget_event->setColumnCount(1);
    ui->tableWidget_range->setColumnCount(1);
}

SmartSpeedDebug::~SmartSpeedDebug()
{
    delete ui;
}

void SmartSpeedDebug::clearInfo()
{
}

void SmartSpeedDebug::setGeneralInfo(const QList<resp_search_common_backup> &commonList)
{
    ui->tableWidget_general->clearContents();
    ui->tableWidget_general->setRowCount(commonList.size());
    for (int i = 0; i < commonList.size(); ++i) {
        const resp_search_common_backup &backup = commonList.at(i);
        setTableItemText(ui->tableWidget_general, i, 0, QString("%1 - %2").arg(backup.pStartTime).arg(backup.pEndTime));
        setTableItemText(ui->tableWidget_general, i, 1, backupTypeString(backup.enEvent));
    }
}

void SmartSpeedDebug::setEventInfo(const QList<resp_search_event_backup> &eventList)
{
    ui->tableWidget_event->clearContents();
    ui->tableWidget_event->setRowCount(eventList.size());
    for (int i = 0; i < eventList.size(); ++i) {
        const resp_search_event_backup &backup = eventList.at(i);
        setTableItemText(ui->tableWidget_event, i, 0, QString("%1 - %2").arg(backup.pStartTime).arg(backup.pEndTime));
        setTableItemText(ui->tableWidget_event, i, 1, backupTypeString(backup.enEvent));
    }
}

void SmartSpeedDebug::setSmartRangeInfo(const QMap<DateTimeRange, int> &rangeMap)
{
    ui->tableWidget_range->clearContents();
    ui->tableWidget_range->setRowCount(rangeMap.size());
    int row = 0;
    for (auto iter = rangeMap.constBegin(); iter != rangeMap.constEnd(); ++iter) {
        const DateTimeRange &range = iter.key();
        setTableItemText(ui->tableWidget_range, row, 0, QString("%1 - %2").arg(range.startDateTime.toString("yyyy-MM-dd HH:mm:ss")).arg(range.endDateTime.toString("yyyy-MM-dd HH:mm:ss")));
        row++;
    }
}

void SmartSpeedDebug::setCurrentGeneralText(const QString &text)
{
    ui->label_currentGeneral->setText(text);
}

void SmartSpeedDebug::setNextGeneralText(const QString &text)
{
    ui->label_nextGeneral->setText(text);
}

void SmartSpeedDebug::setCurrentEventText(const QString &text)
{
    ui->label_currentEvent->setText(text);
}

void SmartSpeedDebug::setNextEventText(const QString &text)
{
    ui->label_nextEvent->setText(text);
}

void SmartSpeedDebug::setRealTimeText(const QString &text)
{
    ui->label_realTime->setText(text);
}

void SmartSpeedDebug::setSpeedText(const QString &text)
{
    ui->label_speed->setText(text);
}

void SmartSpeedDebug::setTableItemText(QTableWidget *widget, int row, int column, const QString &text)
{
    QTableWidgetItem *item = widget->item(row, column);
    if (!item) {
        item = new QTableWidgetItem();
        widget->setItem(row, column, item);
    }
    item->setText(text);
}

QString SmartSpeedDebug::backupTypeString(int type)
{
    QString text;
    switch (type) {
    case REC_EVENT_TIME:
        text = "REC_EVENT_TIME";
        break;
    default:
        text = "REC_EVENT_EVENT";
        break;
    }
    return text;
}

void SmartSpeedDebug::on_toolButton_close_clicked()
{
    hide();
}

void SmartSpeedDebug::on_pushButton_refresh_clicked()
{
    emit refresh();
}
