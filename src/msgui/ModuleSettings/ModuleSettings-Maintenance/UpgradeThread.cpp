#include "UpgradeThread.h"
#include "centralmessage.h"
#include "MyDebug.h"

UpgradeThread::UpgradeThread()
    : MsObject(nullptr)
{
    moveToThread(&m_thread);
    m_thread.start();
}

UpgradeThread::~UpgradeThread()
{

}

void UpgradeThread::stopThread()
{
    m_thread.quit();
    m_thread.wait();
}

void UpgradeThread::startLocalUpgrade(const QString &filePath, bool reset)
{
    m_filePath = filePath;
    m_reset = reset;
    QMetaObject::invokeMethod(this, "onStartLocalUpgrade");
}

void UpgradeThread::getOnlineUpgradeImage(const QString &filePath)
{
    m_filePath = filePath;
    m_reset = 0;
    QMetaObject::invokeMethod(this, "onGetOnlineUpgradeImage");
}

void UpgradeThread::startOnlineUpgrade()
{
    QMetaObject::invokeMethod(this, "onStartOnlineUpgrade");
}

void UpgradeThread::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_UPGRADE_SYSTEM:
        ON_RESPONSE_FLAG_UPGRADE_SYSTEM(message);
        break;
    case RESPONSE_FLAG_GET_UPGRADE_IMAGE:
        ON_RESPONSE_FLAG_GET_UPGRADE_IMAGE(message);
        break;
    }
}

void UpgradeThread::ON_RESPONSE_FLAG_UPGRADE_SYSTEM(MessageReceive *message)
{
    int result = *((int *)message->data);
    qMsDebug() << "result:" << result;
    emit upgradeFinished(result);
}

void UpgradeThread::ON_RESPONSE_FLAG_GET_UPGRADE_IMAGE(MessageReceive *message)
{
    int result = *((int *)message->data);
    qMsDebug() << "result:" << result;
    emit downloadFinished(result);
}

int UpgradeThread::copyFile(const char *dest, const char *src)
{
    FILE *src_fp;
    FILE *dest_fp;
    unsigned char buf[2048];
    int count;

    if (src == NULL || dest == NULL) {
        return -1;
    }

    src_fp = fopen(src, "rb");
    if (src_fp == NULL) {
        return -1;
    }
    dest_fp = fopen(dest, "wb");
    if (dest_fp == NULL) {
        fclose(src_fp);
        return -1;
    }

    while ((count = fread(buf, 1, 2048, src_fp))) {
        fwrite(buf, 1, count, dest_fp);
    }

    fclose(src_fp);
    fclose(dest_fp);

    return 0;
}

void UpgradeThread::onStartLocalUpgrade()
{
    copyFile("/dev/shm/updateFM.bin", m_filePath.toStdString().c_str());
    sendMessage(REQUEST_FLAG_UPGRADE_SYSTEM, (void *)&m_reset, sizeof(int));
}

void UpgradeThread::onGetOnlineUpgradeImage()
{
    struct ms_online_upgrade_image online_upgrade_image;
    memset(&online_upgrade_image, 0, sizeof(online_upgrade_image));
    snprintf(online_upgrade_image.pUrl, sizeof(online_upgrade_image.pUrl), "%s", m_filePath.toStdString().c_str());
    snprintf(online_upgrade_image.filepath, sizeof(online_upgrade_image.filepath), "%s", UPDATE_IMAGE_NAME);

    sendMessage(REQUEST_FLAG_GET_UPGRADE_IMAGE, (void *)&online_upgrade_image, sizeof(online_upgrade_image));
}

void UpgradeThread::onStartOnlineUpgrade()
{
    sendMessage(REQUEST_FLAG_UPGRADE_SYSTEM, (void *)&m_reset, sizeof(int));
}

