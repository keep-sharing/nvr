#ifndef MYLINEEDITTIME_H
#define MYLINEEDITTIME_H

#include "mylineedit.h"
#include "timeeditwidget.h"

class MyLineEditTime : public MyLineEdit
{
    Q_OBJECT
public:
    explicit MyLineEditTime(QWidget *parent = nullptr);

    int beginMinute() const;
    void setBeginMinute(int minute);
    int endMinute() const;
    void setEndMinute(int minute);

    void clearTime();

    bool isValid() const;

protected:
    void focusInEvent(QFocusEvent *) override;
    void focusOutEvent(QFocusEvent *) override;

private:
    void updateTime();
    void showTime();

signals:
    void timeEditingFinished(int beginMinute, int endMinute);

private slots:
    void onEditingFinished();

private:
    TimeEditWidget *m_timeEdit;

    int m_beginMinute = 0;
    int m_endMinute = 0;
};

#endif // MYLINEEDITTIME_H
