#ifndef MYDATEEDIT_H
#define MYDATEEDIT_H

#include <QDateEdit>

class MyCalendarWidget;

class MyDateEdit : public QDateEdit
{
    Q_OBJECT
public:
    explicit MyDateEdit(QWidget *parent = nullptr);

    bool setProperty(const char *name, const QVariant &value);

protected:
    void mousePressEvent(QMouseEvent *event) override;

signals:

public slots:

private:
    MyCalendarWidget *m_calendarWidget = nullptr;
};

#endif // MYDATEEDIT_H
