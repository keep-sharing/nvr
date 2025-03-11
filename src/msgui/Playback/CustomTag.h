#ifndef CUSTOMTAG_H
#define CUSTOMTAG_H

#include "BaseShadowDialog.h"

namespace Ui {
class CustomTag;
}

class CustomTag : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit CustomTag(QWidget *parent = nullptr);
    ~CustomTag();

    void addTag(const QString &title);
    void editTag(const QString &name);

    QString tagName();

private slots:
    void on_pushButton_add_clicked();
    void on_pushButton_cancel_clicked();

private:
    Ui::CustomTag *ui;
};

#endif // CUSTOMTAG_H
