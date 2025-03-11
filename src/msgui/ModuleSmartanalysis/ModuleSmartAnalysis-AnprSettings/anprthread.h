#ifndef ANPRTHREAD_H
#define ANPRTHREAD_H

#include <QMap>
#include <QMutex>
#include <QObject>
#include <QThread>
#include <QWaitCondition>

extern "C" {
#include "msdb.h"
}

#define MAX_ANPR_LIST_COUNT 10000

class AnprThread : public QObject {
    Q_OBJECT

public:
    enum RepeatMode {
        ModeNone,
        ModeIgnore,
        ModeReplace
    };

    explicit AnprThread(QObject *parent = nullptr);

    void stopThread();

    void setImportRepeatMode(RepeatMode mode);
    void importAnprList(const QString &filePath);

    void deleteAnprList(const QList<anpr_list> &list);

    void stopOperate();

private:
    bool isStop();

signals:
    void importError(const QString &error);
    void importDealRepeat();
    void importProgress(int progress);
    void importResult(int succeedCount, int failedCount, int errorCount, bool isReachLimit, bool isCancel);

    void deleteProgress(int progress);
    void deleteResult();

private slots:
    void onImportAnprList(const QString &filePath);
    void onDeleteAnprList();

private:
    QThread m_thread;

    RepeatMode m_importRepeatMode = ModeNone;
    QMap<QString, anpr_list> m_existedAnprMap;
    QList<anpr_list> m_importAnprList;
    QList<anpr_list> m_deleteAnprList;

    QMutex m_mutex;
    QWaitCondition m_wait;
    bool m_isStop = false;
};

#endif // ANPRTHREAD_H
