#ifndef PAGECONTROL_H
#define PAGECONTROL_H

#include <QWidget>

namespace Ui {
class PageControl;
}

class PageControl : public QWidget
{
    Q_OBJECT

public:
    explicit PageControl(QWidget *parent = 0);
    ~PageControl();

private:
    Ui::PageControl *ui;
};

#endif // PAGECONTROL_H
