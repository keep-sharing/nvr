#ifndef MYDATETIMEWIDGET_H
#define MYDATETIMEWIDGET_H

#include <QDateTimeEdit>

class MyDateTimeEdit : public QDateTimeEdit
{
    Q_OBJECT
public:
    explicit MyDateTimeEdit(QWidget *parent = nullptr);

signals:

public slots:
};

#endif // MYDATETIMEWIDGET_H
