#ifndef DISKSMARTTEST_H
#define DISKSMARTTEST_H

#include "MsObject.h"
#include <QTimer>

class MessageReceive;

#define gDiskSmartTest DiskSmartTest::instance()

class DiskSmartTest : public MsObject {
    Q_OBJECT

public:
    explicit DiskSmartTest(QObject *parent = nullptr);
    ~DiskSmartTest();

    static DiskSmartTest &instance();

    void setTestPort(int port);
    void clearTestPort();
    int testPort();
    int testProgress();

    void startGetProcess();
    bool isTesting();

    void processMessage(MessageReceive *message);

protected:
    void ON_RESPONSE_FLAG_GET_SMART_PROCESS(MessageReceive *message);

signals:
    void progressChanged(int port, int testProgress);

private slots:
    void onProcessTimer();

private:
    QTimer *m_progressTimer = nullptr;
    int m_port = -1;
    int m_progress = 0;
};

#endif // DISKSMARTTEST_H
