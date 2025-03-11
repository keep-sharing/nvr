#ifndef FORMATDIALOG_H
#define FORMATDIALOG_H

#include "BaseShadowDialog.h"

extern "C" {
#include "msg.h"
}

class MessageReceive;

namespace Ui {
class FormatDialog;
}

class FormatDialog : public BaseShadowDialog {
    Q_OBJECT

public:
    explicit FormatDialog(QWidget *parent = nullptr);
    ~FormatDialog();

    void formatDevice(const resp_usb_info &usb_info);

    void dealMessage(MessageReceive *message);
    void processMessage(MessageReceive *message) override;

protected:
    void ON_RESPONSE_FLAG_FORMAT_EXPORT_DISK(MessageReceive *message);
    void ON_RESPONSE_FLAG_PROGRESS_RETRIEVE_INIT(MessageReceive *message);

private slots:
    void onLanguageChanged() override;

    void on_pushButton_format_clicked();
    void on_pushButton_cancel_clicked();

private:
    Ui::FormatDialog *ui;

    resp_usb_info m_usb_info;
};

#endif // FORMATDIALOG_H
