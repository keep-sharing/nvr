#ifndef SEARCHCAMERAEDITMULTI_H
#define SEARCHCAMERAEDITMULTI_H

#include "BaseShadowDialog.h"

extern "C" {
#include "msg.h"
}

class MessageReceive;

namespace Ui {
class SearchCameraEditMulti;
}

class SearchCameraEditMulti : public BaseShadowDialog
{
    Q_OBJECT

    enum TableColumn
    {
        ColumnCheck,
        ColumnNumber,
        ColumnMac,
        ColumnOldIP,
        ColumnNewIP,
        ColumnResult
    };

public:
    explicit SearchCameraEditMulti(QWidget *parent = nullptr);
    ~SearchCameraEditMulti();

    int execEdit(const QList<req_set_ipcaddr_batch> &list);

    void processMessage(MessageReceive *message);

private:
    void ON_RESPONSE_FLAG_GET_IPCADDR_BATCH(MessageReceive *message);
    bool isInputValid();

private slots:
    void onLanguageChanged();

    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

    void on_lineEdit_ip_textChanged(const QString &ip);

private:
    Ui::SearchCameraEditMulti *ui;

    QList<req_set_ipcaddr_batch> m_listSetIpcAddr;
};

#endif // SEARCHCAMERAEDITMULTI_H
