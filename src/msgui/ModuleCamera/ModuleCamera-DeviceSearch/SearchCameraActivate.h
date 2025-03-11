#ifndef SEARCHCAMERAACTIVATE_H
#define SEARCHCAMERAACTIVATE_H

#include "BaseShadowDialog.h"

class MessageReceive;

namespace Ui {
class SearchCameraActivate;
}

class SearchCameraActivate : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit SearchCameraActivate(QWidget *parent = 0);
    ~SearchCameraActivate();

private slots:
    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

public:
    void onLanguageChanged();

private:
    Ui::SearchCameraActivate *ui;

signals:
    void sigCameraActive(QString password);
};

#endif // SEARCHCAMERAACTIVATE_H
