#ifndef MSSCHEDULEWIDGET_H
#define MSSCHEDULEWIDGET_H

#include <QWidget>
#include "msscheduledraw.h"
#include "msscheduleedit.h"

namespace Ui {
class MsScheduleWidget;
}

class MsScheduleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MsScheduleWidget(QWidget *parent = nullptr);
    ~MsScheduleWidget();

    void setCurrentType(const int &type);

    //自定义颜色
    void setTypeColor(int type, QColor color);
    void clearTypeColor();

    //自定义Edit里面的type，为了兼容之前的代码，addEditType之前需要先clearEditType
    void addEditType(const QString &name, int type);
    void clearEditType();
    void setSingleEditType(int type);

    void setHolidayVisible(bool visible);
    void setCheckDayEnable(bool enable);

    void setSchedule(schedule_day *schedule_day_array);
    void getSchedule(schedule_day *schedule_day_array);

//    void setSchedule(schedule_day *schedule);
//    void getSchedule(schedule_day *schedule);

private slots:
    void onLanguageChanged();
    void onEditingFinished();

    void on_pushButton_clearAll_clicked();
    void on_pushButton_selectAll_clicked();
    void on_pushButton_edit_clicked();

private:
    Ui::MsScheduleWidget *ui;

    MsScheduleEdit *m_schedultEdit;
};

#endif // MSSCHEDULEWIDGET_H
