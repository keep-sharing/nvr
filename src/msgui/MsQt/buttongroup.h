#ifndef BUTTONGROUP_H
#define BUTTONGROUP_H

#include "mypushbutton.h"
#include <QButtonGroup>
#include <QWidget>

namespace Ui {
class ButtonGroup;
}

class ButtonGroup : public QWidget {
    Q_OBJECT
public:
    explicit ButtonGroup(QWidget *parent = 0);
    ~ButtonGroup();

    void setCount(int count);
    void setCountFromChannelName(int count);
    int count();

    //触发buttonClicked信号
    void setCurrentIndex(int index);
    //不触发buttonClicked信号
    void editCurrentIndex(int index);

    int currentIndex() const;

signals:
    void buttonClicked(int index);

private slots:
    void onButtonClicked(int index);

private:
    Ui::ButtonGroup *ui;

    int m_columnCount = 8;
    int m_count = 0;

    QButtonGroup *m_buttonGroup;
};

#endif // BUTTONGROUP_H
