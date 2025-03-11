#ifndef ROUTEREDIT_H
#define ROUTEREDIT_H

#include "BaseShadowDialog.h"

extern "C" {
#include "msdb.h"
}

namespace Ui {
class RouterEdit;
}

class RouterEdit : public BaseShadowDialog {
    Q_OBJECT

public:
    explicit RouterEdit(QWidget *parent = nullptr);
    ~RouterEdit();

    void setNetinfo(struct network *info);
    void updateInfo(int index);

private slots:
    void on_pushButton_back_clicked();

private:
    Ui::RouterEdit *ui;

    struct network _netinfo;
};

#endif // ROUTEREDIT_H
