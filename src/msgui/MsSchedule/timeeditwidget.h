#ifndef TIMEEDITWIDGET_H
#define TIMEEDITWIDGET_H

#include <QWidget>

namespace Ui {
class TimeEditWidget;
}

class TimeEditWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TimeEditWidget(QWidget *parent = nullptr);
    ~TimeEditWidget();

    bool hasFocus() const;
    int beginMinute() const;
    void setBeginMinute(int minute);
    int endMinute() const;
    void setEndMinute(int minute);

signals:
    void editingFinished();

private slots:
    void on_spinBox_endHour_valueChanged(int arg1);

    void on_spinBox_beginHour_valueChanged(int arg1);

private:
    Ui::TimeEditWidget *ui;
};

#endif // TIMEEDITWIDGET_H
