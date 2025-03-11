#ifndef UPNPEDIT_H
#define UPNPEDIT_H

#include "BaseShadowDialog.h"

namespace Ui {
class UPnPEdit;
}

class UPnPEdit : public BaseShadowDialog {
    Q_OBJECT

public:
    explicit UPnPEdit(QWidget *parent = nullptr);
    ~UPnPEdit();

    void initializeData(const QString &type, int port);
    int externalPort();

private slots:
    void onLanguageChanged();

    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

private:
    Ui::UPnPEdit *ui;
};

#endif // UPNPEDIT_H
