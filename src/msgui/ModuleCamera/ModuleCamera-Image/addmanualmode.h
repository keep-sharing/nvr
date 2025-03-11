#ifndef ADDMANUALMODE_H
#define ADDMANUALMODE_H

#include <QWidget>

namespace Ui {
class AddManualMode;
}

class AddManualMode : public QWidget
{
    Q_OBJECT

public:
    explicit AddManualMode(QWidget *parent = 0);
    ~AddManualMode();
private:
	void slotTranslateUi();

private:
    Ui::AddManualMode *ui;
};

#endif // ADDMANUALMODE_H
