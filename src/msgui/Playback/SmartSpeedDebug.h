#ifndef SMARTSPEEDDEBUG_H
#define SMARTSPEEDDEBUG_H

#include "BaseShadowDialog.h"
#include "DateTimeRange.h"

extern "C" {
#include "msg.h"
}

class QTableWidget;

namespace Ui {
class SmartSpeedDebug;
}

class SmartSpeedDebug : public BaseShadowDialog {
    Q_OBJECT

public:
    explicit SmartSpeedDebug(QWidget *parent = nullptr);
    ~SmartSpeedDebug();

    void clearInfo();
    void setGeneralInfo(const QList<resp_search_common_backup> &commonList);
    void setEventInfo(const QList<resp_search_event_backup> &eventList);
    void setSmartRangeInfo(const QMap<DateTimeRange, int> &rangeMap);

    void setCurrentGeneralText(const QString &text);
    void setNextGeneralText(const QString &text);
    void setCurrentEventText(const QString &text);
    void setNextEventText(const QString &text);
    void setRealTimeText(const QString &text);
    void setSpeedText(const QString &text);

private:
    void setTableItemText(QTableWidget *widget, int row, int column, const QString &text);
    QString backupTypeString(int type);

signals:
    void refresh();

private slots:
    void on_toolButton_close_clicked();

    void on_pushButton_refresh_clicked();

private:
    Ui::SmartSpeedDebug *ui;
};

#endif // SMARTSPEEDDEBUG_H
