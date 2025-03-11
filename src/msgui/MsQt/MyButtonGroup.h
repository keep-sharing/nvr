#ifndef MYBUTTONGROUP_H
#define MYBUTTONGROUP_H

#include <QButtonGroup>

class MyButtonGroup : public QButtonGroup
{
    Q_OBJECT
public:
    explicit MyButtonGroup(QObject *parent = nullptr);

    void setCurrentButton(QAbstractButton *btn);
    void editCurrentButton(QAbstractButton *btn);

    void setCurrentId(int id);
    void editCurrentId(int id);

signals:

};

#endif // MYBUTTONGROUP_H
