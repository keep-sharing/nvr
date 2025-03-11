#ifndef PTZMASKEDIT_H
#define PTZMASKEDIT_H

#include "BaseShadowDialog.h"

extern "C" {
#include "msg.h"
}

namespace Ui {
class PtzMaskEdit;
}

class PtzMaskEdit : public BaseShadowDialog {
    Q_OBJECT

public:
    explicit PtzMaskEdit(QWidget *parent = nullptr);
    ~PtzMaskEdit();

    int execEdit(mask_area_ex *area, int maxZoomRatio);

private slots:
    void onLanguageChanged();
    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

private:
    Ui::PtzMaskEdit *ui;

    mask_area_ex *m_area = nullptr;
};

#endif // PTZMASKEDIT_H
