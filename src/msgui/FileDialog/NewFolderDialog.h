#ifndef NEWFOLDERDIALOG_H
#define NEWFOLDERDIALOG_H

#include "BaseShadowDialog.h"

namespace Ui {
class NewFolderDialog;
}

class NewFolderDialog : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit NewFolderDialog(QWidget *parent = nullptr);
    ~NewFolderDialog();

    QString name() const;

protected:
    void showEvent(QShowEvent *event) override;

signals:
    void sig_newFolder(const QString &name);

private slots:
    void onLanguageChanged() override;

    void on_pushButton_create_clicked();
    void on_pushButton_cancel_clicked();

private:
    Ui::NewFolderDialog *ui;
};

#endif // NEWFOLDERDIALOG_H
