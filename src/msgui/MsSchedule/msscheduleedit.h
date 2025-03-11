#ifndef MSSCHEDULEEDIT_H
#define MSSCHEDULEEDIT_H

#include "BaseShadowDialog.h"
#include "scheduletime.h"

class MyTimeRangeEdit;

namespace Ui {
class MsScheduleEdit;
}

class MsScheduleEdit : public BaseShadowDialog {
    Q_OBJECT

public:
    explicit MsScheduleEdit(QWidget *parent = nullptr);
    ~MsScheduleEdit() override;

    void setScheduleArray(uint16_t** scheduleArray);
    uint16_t** scheduleArray() const;

    void addType(const QString &name, int type);
    void clearType();
    void setSingleEditType(int type);

    void setHolidayVisible(bool visible);
    void setDayVisible(bool visible);

private:
    void showTime();
    bool saveSetting();

signals:
    void editingFinished();

private slots:
    void onLanguageChanged();

    void on_comboBox_day_activated(int index);
    void on_comboBox_type_activated(int index);

    void onTimeEditingFinished(int beginMinutes, int endMinutes);
    void onTimeCleared(int beginMinutes, int endMinutes);

    void on_pushButton_copy_clicked();
    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

protected:
    Ui::MsScheduleEdit *ui;

    QList<MyTimeRangeEdit *> m_timeEditList;

    uint16_t **m_scheduleArray;

    //兼容IPC没有Holiday
    bool m_holidayVisible = true;
};

#endif // MSSCHEDULEEDIT_H
