#ifndef PEOPLECOUNTINGBACKUP_H
#define PEOPLECOUNTINGBACKUP_H

/******************************************************************
* @brief    人数统计报表手动导出
* @author   LiuHuanyu
* @date     2021-08-10
******************************************************************/

#include "MyFileSystemDialog.h"
#include "PeopleCountingData.h"
#include <QThread>

class PeopleCountingBackup : public QThread {
    Q_OBJECT

public:
    enum BackupResult {
        BackupResultUnKonw = 0,
        BackupResultSuccessed = 1,
        BackupResultFailed = 2,
        BackupResultSomeSuccessed = 3
    };
    explicit PeopleCountingBackup(QObject *parent = 0);

    void startBackup(const ExportPeopleCountingInfo &info);

    void setGroups(const QList<int> &groups);
    void setSelectTypes(const QList<int> &typeList);
    void setViewportGeometry(const QRect &rc);
    void setChartMode(const ChartMode &mode);
    void setMainTitle(const QString &text);
    void setSubTitle(const QString &text);
    void setSubTitleForBackup(const QString &text);

    void setChannels(const QList<int> &channels);

    void setTextMap(const QMap<int, QString> &textMap);
    void setLineMask(int lineMask);

protected:
    void run() override;

private:
    int backupCameraCSV();
    int backupGroupCSV();
    int backupRegionCSV();

    int backupCameraPDF();
    int backupGroupPDF();
    int backupRegionPDF();

    int backupCameraPNG();
    int backupGroupPNG();
    int backupRegionPNG();

    int backupCSV(const QString &filePath, int groupFilter, const QList<int> &channels);
    void drawCSV(QPainter *painter, int groupFilter, const QList<int> &channels);

    int backupPDF(const QString &filePath, int groupFilter, const QList<int> &channels);
    int backupPNG(const QString &filePath, int groupFilter, const QList<int> &channels);

    QMap<int, QList<QString>> calHorizontalNames(QPainter *painter, qreal perWidth, const QList<int> &channels, int *maxRowCount);

signals:
    void backupFinished(int result);

public slots:

private:
    ExportPeopleCountingInfo m_info;

    ReportType m_recordType;
    StatisticsType m_statisticsType;
    QDateTime m_startDateTime;

    QMap<int, QString> m_groupNameMap;
    QMap<int, QList<int>> m_groupChannelsMap;
    QMap<int, QList<int>> m_groupAllChannelsMap;

    QList<int> m_selectTypes;
    QList<int> m_groups;
    QList<int> m_channels;
    QRect m_viewportGeometry;
    ChartMode m_chartMode;

    QString m_mainTitle;
    QString m_subTitle;
    QString m_subTitleForBackup;
    QMap<int ,QString> m_textMap;
    int m_lineMask = 0;
};

#endif // PEOPLECOUNTINGBACKUP_H
