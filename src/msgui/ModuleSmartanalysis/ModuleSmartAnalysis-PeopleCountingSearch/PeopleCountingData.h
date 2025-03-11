#ifndef PEOPLECOUNTINGDATA_H
#define PEOPLECOUNTINGDATA_H

#include <QColor>
#include <QDateTime>
#include <QEventLoop>
#include <QMap>
#include <QObject>
#include <QThread>

class QPainter;
class PeopleCountingDataThread;

extern "C" {
#include "msdb.h"
}

#define TOTAL_LINE_INDEX -1
#define gPeopleCountingData PeopleCountingData::instance()

enum ChartMode {
    LineChart,
    HistogramChart,
    Histogram2Chart
};

enum ReportType {
    DailyReport,
    WeeklyReport,
    MonthlyReport,
    YearlyReport
};

enum StatisticsType {
    StatisticsNone = -1,
    PeopleEntered = 0,
    PeopleExited = 1,
    PeopleAll = 2
};

struct PeopleGridData {
    int yMinValue = 0;
    int yMaxValue = 50;
    int yLineCount = 11;

    int xMinValue = 0;
    int xMaxValue = 23;
    int xLineCount = 24;

    QList<QString> xAxisStrings;
};

struct PeopleLineData {
    //<line id>, 为了划线顺序
    QList<int> lines;
    //<line id, <x index, value>>
    QMap<int, QMap<int, int>> lineData;

    void clear()
    {
        lines.clear();
        lineData.clear();
    }
};

struct HistogramKey {
    int id = 0;
    int value = 0;

    HistogramKey()
    {
    }
    HistogramKey(int _id, int _value)
    {
        id = _id;
        value = _value;
    }

    bool operator<(const HistogramKey &other) const
    {
        if (value != other.value) {
            return value > other.value;
        } else {
            return id < other.id;
        }
    }
};

struct HistogramValue {
    int value;
    QColor color;

    HistogramValue()
    {
    }
    HistogramValue(int _value, QColor _color)
    {
        value = _value;
        color = _color;
    }
};

struct PeopleHistogramData {
    //key1: x, key2: line id
    QMap<int, QMap<HistogramKey, HistogramValue>> histogramData;
    //key: line id
    QMap<int, HistogramValue> histogramData2;

    void clear()
    {
        histogramData.clear();
        histogramData2.clear();
    }
};

struct PeopleCameraData {
    PeopleGridData lineGridData;
    PeopleGridData histogramGridData;
    PeopleGridData histogramGridData2;
    //
    PeopleLineData lineData;
    PeopleHistogramData histogramData;

    void clear()
    {
        lineGridData.xAxisStrings.clear();
        histogramGridData.xAxisStrings.clear();
        histogramGridData2.xAxisStrings.clear();
        lineData.clear();
        histogramData.clear();
    }
};

struct PeopleGroupData {
    //key: group id
    QMap<int, PeopleCameraData> peopleDataMap;

    void clear()
    {
        peopleDataMap.clear();
    }
};

class PeopleCountingData : public QObject {
    Q_OBJECT
public:
    explicit PeopleCountingData(QObject *parent = 0);
    ~PeopleCountingData();

    static PeopleCountingData &instance();

    people_cnt_info *peopleInfo();

    void setSearchInfo(PEOPLECNT_SEARCH_TYPE_E searchType, ReportType report, StatisticsType statistics, const QDateTime &start, const QList<int> checkedList);
    void waitForSearchData();

    QColor color(int index);

    PEOPLECNT_SEARCH_TYPE_E searchType() const;
    ReportType reportType() const;
    StatisticsType statisticsType() const;
    QString statisticsTypeString() const;
    QDateTime startDateTime() const;
    QDateTime endDateTime() const;

    PeopleGridData peopleLineGridData(int groupFilter = -1) const;
    PeopleGridData peopleHistogramGridData(int groupFilter = -1) const;
    PeopleGridData peopleHistogramGridData2(int groupFilter = -1) const;
    PeopleLineData peopleLineData(int groupFilter = -1) const;
    PeopleHistogramData peopleHistogramData(int groupFilter = -1) const;

    QMap<int, QString> groupsName();
    //最多10个通道
    QMap<int, QList<int>> groupsChannels();
    //所有通道
    QMap<int, QList<int>> groupsAllChannels();

    static bool hasChannel(const QString &text);

    //ipc
    void setCurrentChannel(int channel);
    int currentChannel() const;
    void dealData(const QString &text, const int line = -1);

    void clearCameraMap();
    bool isHasMapKey(int key);
    //total line data
    QStringList calculateTotal(const QList<QVariant> &list, const QString &key);

private:
signals:

public slots:

private:
    QEventLoop m_eventLoop;

    people_cnt_info *m_people_info = nullptr;

    PEOPLECNT_SEARCH_TYPE_E m_searchType = PEOPLECNT_SEARCH_BY_GROUP;
    ReportType m_reportType = DailyReport;
    StatisticsType m_statisticsType = PeopleEntered;
    QDateTime m_startDateTime;
    QDateTime m_endDateTime;
    QList<int> m_checkedList;

    int m_count = 0;
    peoplecnt_logs *m_logs = nullptr;

    PeopleGroupData m_groupData;
    PeopleCameraData m_cameraData;
    QMap<int, PeopleCameraData> m_cameraDataMap;

    QMap<int, QColor> m_colorMap;

    //ipc
    int m_currentChannel = -1;

    //
    PeopleCountingDataThread *m_searchThread = nullptr;
};

class PeopleCountingDataThread : public QThread {
    Q_OBJECT
public:
    explicit PeopleCountingDataThread(QObject *parent);
    ~PeopleCountingDataThread();

    quint64 id = 0;
    PEOPLECNT_SEARCH_TYPE_E searchType = PEOPLECNT_SEARCH_BY_GROUP;
    ReportType reportType = DailyReport;
    StatisticsType statisticsType = PeopleEntered;
    QDateTime startDateTime;
    QDateTime endDateTime;
    quint64 startSecUTC;
    quint64 endSecUTC;
    QList<int> checkedList;

    int count = 0;
    peoplecnt_logs *logs = nullptr;

    PeopleGroupData groupData;
    PeopleCameraData cameraData;

protected:
    void run() override;

private:
    void makeGroupData();
    PeopleCameraData makeCameraData(int groupFilter = -1);

signals:
    void searchFinished();

private:
};

#endif // PEOPLECOUNTINGDATA_H
