#ifndef EXPOSURESCHEDULEEDIT_H
#define EXPOSURESCHEDULEEDIT_H

#include "BaseShadowDialog.h"
#include "exposurescheduletime.h"

class MyTimeRangeEdit;

namespace Ui {
class ExposureScheduleEdit;
}

class ExposureScheduleEdit : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit ExposureScheduleEdit(QWidget *parent = nullptr);
    ~ExposureScheduleEdit();

    void setScheduleArray(ExposureManualValue** scheduleArray);
    ExposureManualValue** scheduleArray() const;

    void addType(const QString &name, int type);
    void clearType();

    void setManualModeMap(const QMap<int, ExposureManualValue> &map);

private:
    void showTime();
    bool saveSetting();

signals:
    void editingFinished();

private slots:
    void onLanguageChanged();

    void on_comboBox_day_activated(int index);
    void on_comboBox_type_activated(int index);
    void on_comboBox_manual_activated(int index);

    void onTimeEditingFinished(int beginMinutes, int endMinutes);
    void onTimeCleared(int beginMinutes, int endMinutes);

    void on_pushButton_copy_clicked();
    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

private:
    Ui::ExposureScheduleEdit *ui;

    QList<MyTimeRangeEdit *> m_timeEditList;

    ExposureManualValue **m_scheduleArray;
};

#endif // EXPOSURESCHEDULEEDIT_H
