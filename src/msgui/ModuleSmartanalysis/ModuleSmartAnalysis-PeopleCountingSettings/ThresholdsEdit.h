#ifndef THRESHOLDSEDIT_H
#define THRESHOLDSEDIT_H

#include "BaseShadowDialog.h"

extern "C"
{
#include "msdb.h"
}


namespace Ui {
class ThresholdsEdit;
}

class ThresholdsEdit : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit ThresholdsEdit(QWidget *parent = nullptr);
    ~ThresholdsEdit();
    void initializeData(PCNT_CNT_E enableFlag, const int *alarmThresholds);
    void getThresholdsData(PCNT_CNT_E &enableFlag, int *alarmThresholds);

private slots:
    void onLanguageChanged();
    void on_pushButtonOk_clicked();
    void on_pushButtonCancel_clicked();

private:
    Ui::ThresholdsEdit *ui;
};

#endif // THRESHOLDSEDIT_H
