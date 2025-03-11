#ifndef SEARCHCAMERAADD_H
#define SEARCHCAMERAADD_H

#include "BaseShadowDialog.h"
#include "CameraEditTabSettings.h"

class MessageReceive;

namespace Ui {
class SearchCameraAdd;
}

class SearchCameraAdd : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit SearchCameraAdd(QWidget *parent = nullptr);
    ~SearchCameraAdd();

    void showAdd(const resq_search_ipc *search_ipc);

    void processMessage(MessageReceive *message);

private:
    void ON_RESPONSE_FLAG_TEST_IPCCONNECT(MessageReceive *message);
    void ON_RESPONSE_FLAG_CHECK_IPC_FISHEYE_INFO(MessageReceive *message);

    bool isInputValid();
    void testIpcConnect();
    void lineEditIPShow(bool enable);

private slots:
    void onLanguageChanged();

    void on_comboBox_protocol_activated(int index);
    void on_comboBox_channel_activated(int index);
    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

    void on_lineEdit_ip_textEdited(const QString &arg1);
    void on_lineEdit_primary_textEdited(const QString &arg1);
    void on_lineEdit_secondary_textEdited(const QString &arg1);
    void on_comboBox_transportProtocol_activated(int index);

private:
    Ui::SearchCameraAdd *ui;

    QEventLoop m_eventLoop;

    resq_search_ipc m_sourceIpcInfo;

    IPC_CONN_RES m_ipcTestResult;

    //fisheye
    ms_ipc_fisheye_info m_fisheye_info;
    //protocol
    QString m_msddns;
    QString m_ipAddress;
    QString m_primary;
    QString m_secondary;
    IPC_PROTOCOL  m_perProtocol;
};

#endif // SEARCHCAMERAADD_H
