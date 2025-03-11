#ifndef ADDPATROLKEYPOINT_H
#define ADDPATROLKEYPOINT_H

#include "BaseShadowDialog.h"

namespace Ui {
class AddPatrolKeyPoint;
}

class AddPatrolKeyPoint : public BaseShadowDialog
{
    Q_OBJECT

public:
    enum Mode
    {
        ModeNone,
        ModeAdd,
        ModeEdit,
    };
    explicit AddPatrolKeyPoint(Mode mode, int channel ,QWidget *parent = nullptr);
    ~AddPatrolKeyPoint();

    void showKeyPointAdd(int keyIndex);
    void showKeyPointEdit(int keyIndex);
    void setPresetPoint(int value);
    int presetPoint() const;
    void setScanTime(int value);
    int scanTime() const;
    void setScanSpeed(int value);
    int scanSpeed() const;

    void setScanTimeRange(int min, int max);

private slots:
    void onLanguageChanged();

    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

private:
    Ui::AddPatrolKeyPoint *ui;
};

#endif // ADDPATROLKEYPOINT_H
