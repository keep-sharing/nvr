#ifndef TARGETINFOMANAGER_H
#define TARGETINFOMANAGER_H

#include <QDateTime>
#include <QFile>
#include <QImage>
#include <QMutex>
#include <QObject>
#include <QThread>
#include <QTimer>
#include "TargetInfoAnpr.h"
#include "TargetInfoVca.h"
#include "TargetInfoFace.h"

extern "C" {
#include "msdb.h"
#include "msg.h"
}

#define MAX_TARGET_ITEM_COUNT 4

#define gTargetInfoManager TargetInfoManager::instance()

class TargetInfoManager : public QObject {
    Q_OBJECT
public:
    explicit TargetInfoManager(QObject *parent = nullptr);
    ~TargetInfoManager();

    static void anprCallback(void *msg, void *image);

    static TargetInfoManager &instance();

    void readyToQuit();

    void lock();
    bool tryLock();
    bool tryLock(int timeout);
    void unlock();

    void setReceiveEnable(bool enable);
    bool isReceiveEnable();

    void setTempAnprInfo(lpr_metadata_info *pMsg, lpr_image_info *pImage);
    void setTempVcaInfo(MS_VCA_ALARM *alarm);
    void setTempFaceInfo(MS_FACE_ALARM *face);

    bool isProcessingTargetInfo();
    void setIsProcessingTargetInfo(bool newIsProcessingTargetInfo);

    TargetInfo *getTargetInfo(int index) const;
    void clearTargetInfo();

    void updatePlateType();

    //
    void setDisplayFlags(int flags);
    bool checkDisplayFlag(int flag);

    //
    void emitVcaAlarm(MS_VCA_ALARM *alarm);

signals:
    void infoChanged();
    void infoCleared();
    void vcaAlarm(const MS_VCA_ALARM &alarm);

public slots:

private slots:
    void onThreadStarted();
    void onThreadFinished();

    void processTargetInfo();

private:
    static TargetInfoManager *s_anprInfoManager;

    QThread m_thread;

    QMutex m_tempMutex;
    QList<TargetInfo *> m_tempInfoList;

    QMutex m_mutex;
    QList<TargetInfo *> m_infoList;
    bool m_isProcessingTargetInfo = false;

    bool m_isReveive = false;
    int m_displayFlags = 0;
};

class AnprSimulation : public QThread {
    Q_OBJECT

public:
    explicit AnprSimulation(QObject *parent = nullptr)
        : QThread(parent)
    {
    }

protected:
    void run() override
    {
#if 1
        QByteArray ba;
        QFile file(":/wizard/wizard/wizard_background.jpg");
        if (file.open(QFile::ReadOnly))
        {
            ba = file.readAll();
        }

        //
        lpr_metadata_info data_info;
        lpr_image_info image_info;
        image_info.pdata = ba.data();
        image_info.size = ba.size();
        sleep(5);

        //
        MS_VCA_ALARM vca;
        memset(&vca, 0, sizeof(MS_VCA_ALARM));
        vca.jpgdata = (uchar *)ba.data();
        vca.jpgsize = ba.size();
        sleep(5);

        //
        MS_FACE_ALARM face;
        memset(&face, 0, sizeof(MS_VCA_ALARM));
        face.chnId = 0;
        face.faceCnt = 1;
        face.image[0].img.data = (uchar *)ba.data();
        face.image[0].img.size = ba.size();
        face.image[0].attribute.glasses = FACE_GLASSES_YES;
        sleep(5);

        //
        while (true)
        {
            data_info.chnid = 1;
            snprintf(data_info.plate, sizeof(data_info.plate), "%s", "12345");
            snprintf(data_info.ptype, sizeof(data_info.ptype), "%s", "Black");
            snprintf(data_info.ptime, sizeof(data_info.ptime), "%s", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
            TargetInfoManager::anprCallback(&data_info, &image_info);
            msleep(100);
            //
            vca.chnid = 1;
            vca.alarm = 1;
            snprintf(vca.ptime, sizeof(vca.ptime), "%s", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());

            msleep(100);
            //
            snprintf(face.time, sizeof(face.time), "%s", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());

            msleep(100);
        }
#endif
    }
};
#endif // TARGETINFOMANAGER_H
