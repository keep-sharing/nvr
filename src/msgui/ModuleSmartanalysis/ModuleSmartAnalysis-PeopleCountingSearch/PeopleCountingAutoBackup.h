#ifndef PEOPLECOUNTINGAUTOBACKUP_H
#define PEOPLECOUNTINGAUTOBACKUP_H

#include "MsObject.h"
#include "RunnableSearchPeopleCountingByGroup.h"
#include "heatmapthread.h"
#include <QDateTime>
#include <QImage>
#include <QMap>
#include <QMutex>
#include <QStringList>
#include <QThread>
#include <QWaitCondition>

extern "C" {
#include "msdb.h"
}

#define gPeopleCountingAutoBackup PeopleCountingAutoBackup::instance()

class PeopleCountingAutoBackup : public MsObject {
    Q_OBJECT

    struct BackupInfo {
        int error = 0;

        QDate backupDate;
        QTime backupTime;
        QDateTime backupDateTime;

        QString mailTitle;
        QString mailReportType;
        QDateTime mailStartDateTime;
        QDateTime mailEndDateTime;

        QStringList filePathList;
        QString packageName;
        QString packagePath;

        int channelCount = 0;
    };

public:
    explicit PeopleCountingAutoBackup();
    ~PeopleCountingAutoBackup() override;

    static PeopleCountingAutoBackup &instance();

    void filterMessage(MessageReceive *message) override;
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GUI_PEOPLECNT_AUTO_BACKUP(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_PEOPLE_REPORT(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_EXPORT_DISK(MessageReceive *message);
    //heatmap
    void ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_HEATMAP_SUPPORT(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_IPC_SNAPHOST(MessageReceive *message);
    void ON_RESPONSE_FLAG_SEARCH_HEAT_MAP_REPORT(MessageReceive *message);

    void makeGroupData(const REPORT_AUTO_BACKUP_SETTING_S &setting, BackupInfo &info);
    void makeCameraData(const REPORT_AUTO_BACKUP_SETTING_S &setting, BackupInfo &info);
    void makeRegionData(const REPORT_AUTO_BACKUP_SETTING_S &setting, BackupInfo &info);
    void makeHeatMapData(const REPORT_AUTO_BACKUP_SETTING_S &setting, BackupInfo &info);

    void makeSpaceHeatMap(const QString &text, int corridorMode, int imageRotation, int mode, QString &filePath);
    void makeTimeHeatMap(const QString &text, int reportType, const QDateTime &dateTime, QString &filePath);

    void mutexSendMessage(int type, int channel);
    void initializeColorImage();

    //total line data
    QStringList calculateTotal(const QList<QVariant> &list, const QString &key);
    void calculateTime(const QList<QVariant> &list, QString &startTime, QString &endTime);

signals:

public slots:
    void onSearchByGroupFinished(RunnableSearchPeopleCountingByGroup::ResultInfo info);

private slots:
    void onThreadStarted();
    void onThreadFinished();

    void startBackupPeopleCount();

private:
    QThread m_thread;
    QMutex m_mutex;
    QWaitCondition m_wait;
    bool m_isWorking = false;

    QList<REPORT_AUTO_BACKUP_SETTING_S> m_taskList;
    //
    QString m_reportText;
    //
    RunnableSearchPeopleCountingByGroup::ResultInfo m_groupSearchInfo;
    //
    QString m_usbPath;
    //
    DrawHeatMapData m_heatmapData;
    int m_heatmapSupport;
    QImage m_colorImage;
};

#endif // PEOPLECOUNTINGAUTOBACKUP_H
