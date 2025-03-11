#ifndef SEARCHCAMERAEDIT_H
#define SEARCHCAMERAEDIT_H

#include "BaseShadowDialog.h"

extern "C" {
#include "msg.h"
}

class MessageReceive;

namespace Ui {
class SearchCameraEdit;
}

class SearchCameraEdit : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit SearchCameraEdit(QWidget *parent = nullptr);
    ~SearchCameraEdit();

    void showEdit(const req_set_ipcaddr &set_ipcaddr);
    void showEdit(const req_set_ipcaddr_batch &set_ipcaddr);
    req_set_ipcaddr currentIpcaddrInfo() const;

    void processMessage(MessageReceive *message);

protected:
    void ON_RESPONSE_FLAG_IP_CONFLICT_BY_DEV(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_IPCADDR(MessageReceive *message);

private slots:
    void onLanguageChanged();

    void on_comboBox_protocol_activated(int index);
    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

private:
    void initializeProtocol();
    bool isInputValid();

private:
    Ui::SearchCameraEdit *ui;

    req_set_ipcaddr m_ipcaddrSource;
    req_set_ipcaddr m_ipcaddr;
};

#endif // SEARCHCAMERAEDIT_H
