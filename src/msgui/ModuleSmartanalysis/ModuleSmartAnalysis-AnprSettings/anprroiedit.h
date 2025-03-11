#ifndef ANPRROIEDIT_H
#define ANPRROIEDIT_H

#include "BaseShadowDialog.h"

namespace Ui {
class AnprRoiEdit;
}

class AnprRoiEdit : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit AnprRoiEdit(QWidget *parent = nullptr);
    ~AnprRoiEdit();

    void showEdit(const QString &name, bool isTrapeziform);
    QString name() const;

private slots:
    void onLanguageChanged() override;
    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

private:
    Ui::AnprRoiEdit *ui;
};

#endif // ANPRROIEDIT_H
