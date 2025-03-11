#ifndef MYLINEEDITMAC_H
#define MYLINEEDITMAC_H

#include <QLineEdit>

class MyLineEditMac : public QLineEdit
{
    Q_OBJECT
public:
    explicit MyLineEditMac(QWidget *parent = nullptr);

    QString text();

signals:

public slots:
};

#endif // MYLINEEDITMAC_H
