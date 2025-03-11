#ifndef PACKETCAPTUREDATA_H
#define PACKETCAPTUREDATA_H

#include <QMutex>
#include <QThread>

class MessageReceive;
class QTimer;

class PacketCaptureData : public QObject {
    Q_OBJECT

public:
    enum State {
        StateStoped,
        StateWorking
    };

    explicit PacketCaptureData(QObject *parent = nullptr);
    ~PacketCaptureData() override;

    static PacketCaptureData *instance();

    void stopThread();

    PacketCaptureData::State state();
    void clear();

    void startCapture(int diskPort);
    void stopCapture();

    QString ip();
    void setIp(const QString &ip);

    QString port();
    void setPort(const QString &port);

    QString nic();
    void setNic(const QString &nic);

    QString path();
    void setPath(const QString &path);

    void dealMessage(MessageReceive *message);

private:
    void ON_RESPONSE_FLAG_UDISK_OFFLINE(MessageReceive *message);

    QString currentLanName();

signals:
    void threadStarted();

    void started();
    void finished();

private slots:
    void onThreadStarted();

    void onStartCapture();
    void onStopCapture();

    void onTimeout();

private:
    static PacketCaptureData *self;

    QThread m_thread;
    QMutex m_mutex;

    int m_diskPort = 0;

    QTimer *m_timer = nullptr;
    bool m_isTimeout = false;

    State m_state = StateStoped;

    QString m_ip;
    QString m_port;
    QString m_nic;
    QString m_path;
    QString m_tempPath;
};

#endif // PACKETCAPTUREDATA_H
