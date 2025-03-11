#ifndef EDITIMAGEREGIONTYPE_H
#define EDITIMAGEREGIONTYPE_H

#include "BaseShadowDialog.h"

namespace Ui {
class EditImageRegionType;
}

class EditImageRegionType : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit EditImageRegionType(QWidget *parent = nullptr);
    ~EditImageRegionType();
    void setColor(int color);
    int getColor();

private slots:
    void onLanguageChanged();
    void on_pushButtonOK_clicked();
    void on_pushButtonCancel_clicked();

private:
    Ui::EditImageRegionType *ui;
};

#endif // EDITIMAGEREGIONTYPE_H
