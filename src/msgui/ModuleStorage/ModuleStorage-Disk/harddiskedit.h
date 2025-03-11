#ifndef DARDDISKEDIT_H
#define DARDDISKEDIT_H

#include "abstractdiskedit.h"

namespace Ui {
class HardDiskEdit;
}

class HardDiskEdit : public AbstractDiskEdit {
    Q_OBJECT

public:
    explicit HardDiskEdit(QWidget *parent = nullptr);
    ~HardDiskEdit();

    void setDiskInfo(const BaseDiskInfo &info) override;

    void processMessage(MessageReceive *message);

private:
    void ON_RESPONSE_FLAG_SET_MSFS_PORT(MessageReceive *message);

private slots:
    void onLanguageChanged();

    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

private:
    Ui::HardDiskEdit *ui;
};

#endif // DARDDISKEDIT_H
