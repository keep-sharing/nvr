#ifndef MYLINEEDITIP_H
#define MYLINEEDITIP_H

#include <QLineEdit>

class MyLineEditIP : public QLineEdit
{
    Q_OBJECT
public:
    explicit MyLineEditIP(QWidget *parent = nullptr);

    QString text();

signals:

public slots:
};

#endif // MYLINEEDITIP_H
