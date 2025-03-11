#ifndef UPGRADETHREAD_H
#define UPGRADETHREAD_H

#include "MsObject.h"
#include <QThread>

extern "C" {
#include "msg.h"
}

class UpgradeThread : public MsObject {
    Q_OBJECT

public:
    explicit UpgradeThread();
    ~UpgradeThread() override;

    void stopThread();
    //
    void startLocalUpgrade(const QString &filePath, bool reset);
    //
    void getOnlineUpgradeImage(const QString &filePath);
    void startOnlineUpgrade();

    //
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_UPGRADE_SYSTEM(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_UPGRADE_IMAGE(MessageReceive *message);

    int copyFile(const char *dest, const char *src);

private slots:
    void onStartLocalUpgrade();
    //
    void onGetOnlineUpgradeImage();
    void onStartOnlineUpgrade();

signals:
    void upgradeFinished(int result);
    void downloadFinished(int result);

public slots:

private:
    QThread m_thread;

    QString m_filePath;
    int m_reset = 0;
};

#endif // UPGRADETHREAD_H
