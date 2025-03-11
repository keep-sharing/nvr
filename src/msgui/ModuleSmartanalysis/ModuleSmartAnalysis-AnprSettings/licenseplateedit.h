#ifndef LICENSEPLATEEDIT_H
#define LICENSEPLATEEDIT_H

#include "BaseShadowDialog.h"

namespace Ui {
class LicensePlateEdit;
}

class LicensePlateEdit : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit LicensePlateEdit(QWidget *parent = nullptr);
    ~LicensePlateEdit();

    void setLicensePlate(const QString &plate, const QString &type);

private slots:
    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

private:
    Ui::LicensePlateEdit *ui;

    QString m_oldPlate;
    QString m_oldType;
};

#endif // LICENSEPLATEEDIT_H
