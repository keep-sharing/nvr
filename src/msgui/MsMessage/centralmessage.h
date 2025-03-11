#ifndef CENTRALMESSAGE_H
#define CENTRALMESSAGE_H

#include "MessageReceive.h"
#include "MessageSend.h"
#include <QMetaType>
#include <QMutex>
#include <QThread>
#include <QTimer>
#include <QVariant>

extern "C" {
#include "msg.h"
}

#define gMsMessage CentralMessage::instance()

void MsSendMessage(int sendto, int type, const void *data, int size);
void MsSendMessage(int sendto, int type, const void *data, int size, MsObject *object);
void MsSendMessage(int sendto, int type, const void *data, int size, MsWidget *widget);
void MsSendMessage(int sendto, int type, const void *data, int size, MsGraphicsScene *scene);
void MsSendMessage(int sendto, int type, const void *data, int size, MessageObject *obj);

void MsWriteLog(log_data &log);

Q_DECLARE_METATYPE(resp_search_event_backup)

class CentralMessage : public QObject {
    Q_OBJECT

public:
    explicit CentralMessage(QObject *parent = 0);
    ~CentralMessage();

    static CentralMessage &instance();
    void stopThread();

    void sendMessage(int sendto, int type, const void *data, uint size, MessageObject *origin);

    void prepareSendMessage();
    void prepareReceivedMessage();

    void syncFile();
    void refreshAllCameraStatus(int delay_ms);

signals:
    void sig_message();
    void sig_cameraStatus(QMap<int, resp_camera_status> map);

private slots:
    void threadStarted();
    void threadFinished();

    void onSyncFile();
    void onSendMessage();
    void onGetIpcStatus();

private:
    QThread m_thread;
    QTimer *m_timerGetIpcStatus = nullptr;
};

#endif // CENTRALMESSAGE_H
