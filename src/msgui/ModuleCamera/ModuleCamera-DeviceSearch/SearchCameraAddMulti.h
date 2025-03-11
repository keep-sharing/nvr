#ifndef SEARCHCAMERAADDMULTI_H
#define SEARCHCAMERAADDMULTI_H

#include "BaseShadowDialog.h"
#include <QEventLoop>

extern "C" {
#include "msg.h"
}

class MessageReceive;

namespace Ui {
class SearchCameraAddMulti;
}

class SearchCameraAddMulti : public BaseShadowDialog {
    Q_OBJECT

    enum TableColumn {
        ColumnChecked,
        ColumnIP,
        ColumnMAC,
        ColumnResult
    };

public:
    explicit SearchCameraAddMulti(QWidget *parent = 0);
    ~SearchCameraAddMulti();

    void setAddList(const QList<resq_search_ipc> &list);

    void processMessage(MessageReceive *message);

private:
    void ON_RESPONSE_FLAG_TEST_IPCCONNECT(MessageReceive *message);
    void ON_RESPONSE_FLAG_CHECK_IPC_FISHEYE_INFO(MessageReceive *message);

    void addCamera(int channel, int stream_id, const resq_search_ipc &search_ipc);

private slots:
    void onLanguageChanged();

    void on_pushButton_close_clicked();
    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

signals:
    void changed();

private:
    bool isInputValid();
    int waitForTestIpcConnect(const resq_search_ipc *search_ipc);

private:
    Ui::SearchCameraAddMulti *ui;

    QList<resq_search_ipc> m_addList;

    QEventLoop m_eventLoop;

    IPC_CONN_RES m_ipcTestResult;

    //fisheye
    ms_ipc_fisheye_info m_fisheye_info;
};

#endif // SEARCHCAMERAADDMULTI_H
