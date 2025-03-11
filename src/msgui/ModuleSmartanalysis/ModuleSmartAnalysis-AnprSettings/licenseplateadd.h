#ifndef LICENSEPLATEADD_H
#define LICENSEPLATEADD_H

#include "BaseShadowDialog.h"

namespace Ui {
class LicensePlateAdd;
}

class LicensePlateAdd : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit LicensePlateAdd(QWidget *parent = nullptr);
    ~LicensePlateAdd();

private slots:
    void onLanguageChanged();

    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

private:
    Ui::LicensePlateAdd *ui;
};

#endif // LICENSEPLATEADD_H
