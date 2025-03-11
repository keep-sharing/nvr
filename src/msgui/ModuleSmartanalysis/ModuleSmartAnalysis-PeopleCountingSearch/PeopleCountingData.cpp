#include "PeopleCountingData.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "qjson/include/parser.h"
#include <QElapsedTimer>
#include <QFile>
#include <QPainter>
#include <qmath.h>

extern "C" {

}

int yAxisMaxValue(int maxValue)
{
    int yLineMaxValue = 0;
    if (maxValue < 10) {
        yLineMaxValue = 10;
    } else if (maxValue > 1000) {
        yLineMaxValue = qCeil(maxValue / 500.0) * 500;
    } else {
        yLineMaxValue = qCeil(maxValue / 50.0) * 50;
    }
    return yLineMaxValue;
}

QString weekString(int week)
{
    switch (week) {
    case Qt::Monday:
        return GET_TEXT("COMMON/1025", "Monday");
    case Qt::Tuesday:
        return GET_TEXT("COMMON/1026", "Tuesday");
    case Qt::Wednesday:
        return GET_TEXT("COMMON/1027", "Wednesday");
    case Qt::Thursday:
        return GET_TEXT("COMMON/1028", "Thursday");
    case Qt::Friday:
        return GET_TEXT("COMMON/1029", "Friday");
    case Qt::Saturday:
        return GET_TEXT("COMMON/1030", "Saturday");
    case Qt::Sunday:
        return GET_TEXT("COMMON/1024", "Sunday");
    default:
        return QString();
    }
}

PeopleCountingData::PeopleCountingData(QObject *parent)
    : QObject(parent)
{
    m_colorMap.insert(TOTAL_LINE_INDEX, QColor(8, 106, 187));
    m_colorMap.insert(0, QColor(255, 205, 51));
    m_colorMap.insert(1, QColor(30, 214, 186));
    m_colorMap.insert(2, QColor(242, 79, 33));
    m_colorMap.insert(3, QColor(120, 212, 239));
    m_colorMap.insert(4, QColor(199, 90, 239));
    m_colorMap.insert(5, QColor(241, 152, 90));
    m_colorMap.insert(6, QColor(65, 160, 13));
    m_colorMap.insert(7, QColor(243, 82, 169));
    m_colorMap.insert(8, QColor(169, 165, 207));
    m_colorMap.insert(9, QColor(64, 0, 128));
}

PeopleCountingData::~PeopleCountingData()
{
    if (m_logs) {
        ms_free(m_logs);
        m_logs = nullptr;
    }
    if (m_people_info) {
        delete m_people_info;
        m_people_info = nullptr;
    }
}

PeopleCountingData &PeopleCountingData::instance()
{
    static PeopleCountingData self;
    return self;
}

people_cnt_info *PeopleCountingData::peopleInfo()
{
    if (!m_people_info) {
        m_people_info = new people_cnt_info;
    }
    memset(m_people_info, 0, sizeof(people_cnt_info));
    return m_people_info;
}

void PeopleCountingData::waitForSearchData()
{
    //
    quint64 id = 0;
    for (int i = 0; i < m_checkedList.size(); ++i) {
        int index = m_checkedList.at(i);
        id |= (static_cast<quint64>(1) << index);
    }

    //搜索
    if (!m_searchThread) {
        m_searchThread = new PeopleCountingDataThread(this);
        connect(m_searchThread, SIGNAL(searchFinished()), &m_eventLoop, SLOT(quit()));
    }
    m_searchThread->id = id;
    m_searchThread->searchType = m_searchType;
    m_searchThread->reportType = m_reportType;
    m_searchThread->statisticsType = m_statisticsType;
    m_searchThread->startDateTime = m_startDateTime;
    m_searchThread->endDateTime = m_endDateTime;
    m_searchThread->checkedList = m_checkedList;
    m_searchThread->start();
    //m_eventLoop.exec();

    //
    m_count = m_searchThread->count;
    m_groupData = m_searchThread->groupData;
    m_cameraDataMap[0] = m_searchThread->cameraData;
}

QColor PeopleCountingData::color(int index)
{
    return m_colorMap.value(index);
}

PEOPLECNT_SEARCH_TYPE_E PeopleCountingData::searchType() const
{
    return m_searchType;
}

ReportType PeopleCountingData::reportType() const
{
    return m_reportType;
}

StatisticsType PeopleCountingData::statisticsType() const
{
    return m_statisticsType;
}

QString PeopleCountingData::statisticsTypeString() const
{
    QString text;
    switch (m_statisticsType) {
    case PeopleEntered:
        text = GET_TEXT("OCCUPANCY/74209", "People Entered");
        break;
    case PeopleExited:
        text = GET_TEXT("OCCUPANCY/74210", "People Exited");
        break;
    case PeopleAll:
        text = GET_TEXT("OCCUPANCY/74211", "Sum");
        break;
    default:
        break;
    }
    return text;
}

QDateTime PeopleCountingData::startDateTime() const
{
    return m_startDateTime;
}

QDateTime PeopleCountingData::endDateTime() const
{
    return m_endDateTime;
}

PeopleGridData PeopleCountingData::peopleLineGridData(int groupFilter) const
{
    switch (m_searchType) {
    case PEOPLECNT_SEARCH_BY_GROUP:
        return m_groupData.peopleDataMap.value(groupFilter).lineGridData;
    default:
        if (groupFilter == -1) {
            return m_cameraDataMap[currentChannel()].lineGridData;
        } else {
            return m_cameraDataMap[groupFilter].lineGridData;
        }
    }
}

PeopleGridData PeopleCountingData::peopleHistogramGridData(int groupFilter) const
{
    switch (m_searchType) {
    case PEOPLECNT_SEARCH_BY_GROUP:
        return m_groupData.peopleDataMap.value(groupFilter).histogramGridData;
    default:
        if (groupFilter == -1) {
            return m_cameraDataMap[currentChannel()].histogramGridData;
        } else {
            return m_cameraDataMap[groupFilter].histogramGridData;
        }
    }
}

PeopleGridData PeopleCountingData::peopleHistogramGridData2(int groupFilter) const
{
    switch (m_searchType) {
    case PEOPLECNT_SEARCH_BY_GROUP:
        return m_groupData.peopleDataMap.value(groupFilter).histogramGridData2;
    default:
        if (groupFilter == -1) {
            return m_cameraDataMap[currentChannel()].histogramGridData2;
        } else {
            return m_cameraDataMap[groupFilter].histogramGridData2;
        }
    }
}

PeopleLineData PeopleCountingData::peopleLineData(int groupFilter) const
{
    switch (m_searchType) {
    case PEOPLECNT_SEARCH_BY_GROUP:
        return m_groupData.peopleDataMap.value(groupFilter).lineData;
    default:
        if (groupFilter == -1) {
            return m_cameraDataMap[currentChannel()].lineData;
        } else {
            return m_cameraDataMap[groupFilter].lineData;
        }
    }
}

PeopleHistogramData PeopleCountingData::peopleHistogramData(int groupFilter) const
{
    switch (m_searchType) {
    case PEOPLECNT_SEARCH_BY_GROUP:
        return m_groupData.peopleDataMap.value(groupFilter).histogramData;
    default:
        if (groupFilter == -1) {
            return m_cameraDataMap[currentChannel()].histogramData;
        } else {
            return m_cameraDataMap[groupFilter].histogramData;
        }
    }
}

QMap<int, QString> PeopleCountingData::groupsName()
{
    QMap<int, QString> map;

    for (int i = 0; i < MAX_PEOPLECNT_GROUP; ++i) {
        people_cnt_info *people_info = peopleInfo();
        const peoplecnt_setting &setting = people_info->sets[i];
        QString groupName(setting.name);
        if (groupName.isEmpty()) {
            groupName = QString("Group%1").arg(i + 1);
        }
        map.insert(i, groupName);
    }

    return map;
}

QMap<int, QList<int>> PeopleCountingData::groupsChannels()
{
    QMap<int, QList<int>> map;

    for (int i = 0; i < MAX_PEOPLECNT_GROUP; ++i) {
        auto &list = map[i];

        const PEOPLECNT_SETTING &setting = peopleInfo()->sets[i];
        QString checkedMask(setting.tri_channels);
        for (int i = 0; i < checkedMask.size(); ++i) {
            if (checkedMask.at(i) == QChar('1')) {
                list.append(i);
                if (list.size() > 9) {
                    break;
                }
            }
        }
    }

    return map;
}

QMap<int, QList<int>> PeopleCountingData::groupsAllChannels()
{
    QMap<int, QList<int>> map;

    for (int i = 0; i < MAX_PEOPLECNT_GROUP; ++i) {
        auto &list = map[i];

        const PEOPLECNT_SETTING &setting = peopleInfo()->sets[i];
        QString checkedMask(setting.tri_channels);
        for (int i = 0; i < checkedMask.size(); ++i) {
            if (checkedMask.at(i) == QChar('1')) {
                list.append(i);
            }
        }
    }

    return map;
}

bool PeopleCountingData::hasChannel(const QString &text)
{
    bool result = false;
    for (int i = 0; i < text.size(); ++i) {
        if (text.at(i) == QChar('1')) {
            result = true;
            break;
        }
    }
    return result;
}

void PeopleCountingData::setSearchInfo(PEOPLECNT_SEARCH_TYPE_E searchType, ReportType report, StatisticsType statistics, const QDateTime &start, const QList<int> checkedList)
{
    m_searchType = searchType;
    m_reportType = report;
    m_statisticsType = statistics;
    m_startDateTime = start;
    switch (report) {
    case DailyReport:
        m_startDateTime.setTime(QTime(m_startDateTime.time().hour(), 0));
        m_endDateTime = m_startDateTime.addDays(1);
        m_endDateTime = m_endDateTime.addSecs(-1);
        break;
    case WeeklyReport:
        m_startDateTime.setTime(QTime(m_startDateTime.time().hour(), 0));
        m_endDateTime = m_startDateTime.addDays(7);
        m_endDateTime = m_endDateTime.addSecs(-1);
        break;
    case MonthlyReport:
        //这里多了下个月第一天的0点数据，后面绘制的时候不显示这一天
        m_startDateTime.setTime(QTime(m_startDateTime.time().hour(), 0));
        m_endDateTime = m_startDateTime.addMonths(1);
        m_endDateTime = m_endDateTime.addSecs(-1);
        break;
    case YearlyReport:
        m_startDateTime.setTime(QTime(m_startDateTime.time().hour(), 0));
        m_endDateTime = m_startDateTime.addMonths(11);
        m_endDateTime = QDateTime(QDate(m_endDateTime.date().year(), m_endDateTime.date().month(), m_endDateTime.date().daysInMonth()), QTime(23, 59, 59));
        break;
    }
    m_checkedList = checkedList;
}

void PeopleCountingData::dealData(const QString &text, int line)
{
    PeopleCameraData cameraData;
    auto &lineGridData = cameraData.lineGridData;
    auto &histogramGridData = cameraData.histogramGridData;
    auto &histogramGridData2 = cameraData.histogramGridData2;
    auto &lineData = cameraData.lineData;
    auto &histogramData = cameraData.histogramData;

    //x轴刻度数量
    switch (m_reportType) {
    case DailyReport:
        lineGridData.xMaxValue = 23;
        lineGridData.xLineCount = 24;
        histogramGridData.xMaxValue = lineGridData.xMaxValue + 1;
        histogramGridData.xLineCount = lineGridData.xLineCount + 1;
        histogramGridData2.xMaxValue = 11;
        histogramGridData2.xLineCount = 12;
        break;
    case WeeklyReport:
        lineGridData.xMaxValue = 6;
        lineGridData.xLineCount = 7;
        histogramGridData.xMaxValue = lineGridData.xMaxValue + 1;
        histogramGridData.xLineCount = lineGridData.xLineCount + 1;
        histogramGridData2.xMaxValue = 11;
        histogramGridData2.xLineCount = 12;
        break;
    case MonthlyReport:
        lineGridData.xMaxValue = m_startDateTime.daysTo(m_endDateTime);
        lineGridData.xLineCount = lineGridData.xMaxValue + 1;
        histogramGridData.xMaxValue = lineGridData.xMaxValue + 1;
        histogramGridData.xLineCount = lineGridData.xLineCount + 1;
        histogramGridData2.xMaxValue = 11;
        histogramGridData2.xLineCount = 12;
        break;
    case YearlyReport:
        lineGridData.xMaxValue = 11;
        lineGridData.xLineCount = lineGridData.xMaxValue + 1;
        histogramGridData.xMaxValue = lineGridData.xMaxValue + 1;
        histogramGridData.xLineCount = lineGridData.xLineCount + 1;
        histogramGridData2.xMaxValue = 11;
        histogramGridData2.xLineCount = 12;
        break;
    }

    //color
    QMap<int, QColor> colorMap;
    int colorIndex = -1;
    for (int i = 0; i < m_checkedList.size(); ++i) {
        int channel = m_checkedList.at(i);
        lineData.lines.append(channel);
        colorMap.insert(channel, gPeopleCountingData.color(colorIndex));
        colorIndex++;
    }

    //QMap<line, QList<value>>
    //For People Counting, line is {Sum, People Entered, Peole Exited}
    //For Region People Counting, line is {Sum, Region1, Region2, Region3, Region4}
    QMap<int, QList<int>> dataMap;
    switch (m_searchType) {
    case PEOPLECNT_SEARCH_BY_CAMERA: {
        if (text.isEmpty()) {
            break;
        }
        QJson::Parser parser;
        bool ok;
        auto result = parser.parse(text.toLatin1(), &ok);
        if (ok) {
            auto rootMap = result.toMap();
            auto rootList = rootMap.value("report").toList();
            QStringList incountList;
            QStringList outcountList;
            if (!rootList.isEmpty()) {
                if (line) {
                    auto itemMap = rootList[line - 1].toMap();
                    QString incount = itemMap.value("incount").toString();
                    QString outcount = itemMap.value("outcount").toString();
                    incountList = incount.split(",");
                    outcountList = outcount.split(",");
                } else {
                    incountList = calculateTotal(rootList, "incount");
                    outcountList = calculateTotal(rootList, "outcount");
                }
            } else if (!rootMap.isEmpty() && (line == 0 || line == 1)) {
                QString incount = rootMap.value("incount").toString();
                QString outcount = rootMap.value("outcount").toString();
                incountList = incount.split(",");
                outcountList = outcount.split(",");
            } else {
                break;
            }
            QMS_ASSERT(incountList.size() == outcountList.size());
            for (int i = 0; i < incountList.size(); ++i) {
                int in = incountList.at(i).toInt();
                int out = outcountList.at(i).toInt();
                int sum = in + out;
                if (m_checkedList.contains(-1)) {
                    dataMap[TOTAL_LINE_INDEX].append(sum);
                }
                if (m_checkedList.contains(0)) {
                    dataMap[0].append(in);
                }
                if (m_checkedList.contains(1)) {
                    dataMap[1].append(out);
                }
            }
        } else {
            qMsWarning() << "invalid json:" << text;
        }
        break;
    }
    case PEOPLECNT_SEARCH_BY_REGION: {
        if (text.isEmpty()) {
            break;
        }
        QJson::Parser parser;
        bool ok;
        auto result = parser.parse(text.toLatin1(), &ok);
        if (ok) {
            auto rootMap = result.toMap();
            QString incount0 = rootMap.value("incount0").toString();
            QString incount1 = rootMap.value("incount1").toString();
            QString incount2 = rootMap.value("incount2").toString();
            QString incount3 = rootMap.value("incount3").toString();
            QStringList incount0List = incount0.split(",");
            QStringList incount1List = incount1.split(",");
            QStringList incount2List = incount2.split(",");
            QStringList incount3List = incount3.split(",");
            QMS_ASSERT(incount0List.size() == incount1List.size());
            QMS_ASSERT(incount1List.size() == incount2List.size());
            QMS_ASSERT(incount2List.size() == incount3List.size());
            for (int i = 0; i < incount0List.size(); ++i) {
                int in0 = incount0List.at(i).toInt();
                int in1 = incount1List.at(i).toInt();
                int in2 = incount2List.at(i).toInt();
                int in3 = incount3List.at(i).toInt();
                int sum = 0;
                if (m_checkedList.contains(0)) {
                    dataMap[0].append(in0);
                    sum += in0;
                }
                if (m_checkedList.contains(1)) {
                    dataMap[1].append(in1);
                    sum += in1;
                }
                if (m_checkedList.contains(2)) {
                    dataMap[2].append(in2);
                    sum += in2;
                }
                if (m_checkedList.contains(3)) {
                    dataMap[3].append(in3);
                    sum += in3;
                }
                if (m_checkedList.contains(-1)) {
                    dataMap[TOTAL_LINE_INDEX].append(sum);
                }
            }
        } else {
            qMsWarning() << "invalid json:" << text;
        }
        break;
    }
    default:
        break;
    }

    //line data
    int yMaxValue = 0;
    for (auto iter = dataMap.constBegin(); iter != dataMap.constEnd(); ++iter) {
        int line = iter.key();
        const auto &values = iter.value();
        auto &valueMap = lineData.lineData[line];
        for (int i = 0; i < values.size(); ++i) {
            int value = values.at(i);
            valueMap.insert(i, value);
            yMaxValue = qMax(yMaxValue, value);
        }
    }

    //histogram data
    int yMaxValue2 = 0;
    for (int i = 0; i < lineData.lines.size(); ++i) {
        int lineIndex = lineData.lines.at(i);
        const auto &valueMap = lineData.lineData.value(lineIndex);
        //histogram
        int yTotal = 0;
        for (int j = 0; j < lineGridData.xLineCount; ++j) {
            int xValue = j;
            int yValue = valueMap.value(j, 0);
            yTotal += yValue;
            histogramData.histogramData[xValue][HistogramKey(lineIndex, yValue)] = HistogramValue(yValue, colorMap.value(lineIndex));
        }
        //histogram2
        histogramData.histogramData2[lineIndex] = HistogramValue(yTotal, colorMap.value(lineIndex));
        yMaxValue2 = qMax(yMaxValue2, yTotal);
    }

    //y轴最大值
    lineGridData.yMaxValue = yAxisMaxValue(yMaxValue);
    histogramGridData.yMaxValue = yAxisMaxValue(yMaxValue);
    histogramGridData2.yMaxValue = yAxisMaxValue(yMaxValue2);

    //折线图, x轴刻度
    lineGridData.xAxisStrings.clear();
    for (int i = 0; i < lineGridData.xLineCount; ++i) {
        switch (m_reportType) {
        case DailyReport: {
            int hour = (m_startDateTime.time().hour() + i) % 24;
            lineGridData.xAxisStrings.append(QString("%1").arg(hour));
            break;
        }
        case WeeklyReport: {
            QDateTime dateTime = m_startDateTime.addDays(i);
            int week = dateTime.date().dayOfWeek();
            lineGridData.xAxisStrings.append(weekString(week));
            break;
        }
        case MonthlyReport: {
            QDateTime dateTime = m_startDateTime.addDays(i);
            lineGridData.xAxisStrings.append(dateTime.toString("dd"));
            break;
        }
        case YearlyReport: {
            QDateTime dateTime = m_startDateTime.addMonths(i);
            lineGridData.xAxisStrings.append(dateTime.toString("M"));
            break;
        }
        default:
            break;
        }
    }

    //直方图, x轴刻度
    histogramGridData.xAxisStrings.clear();
    for (int i = 0; i < histogramGridData.xLineCount; ++i) {
        switch (m_reportType) {
        case DailyReport: {
            int hour = (m_startDateTime.time().hour() + i) % 24;
            histogramGridData.xAxisStrings.append(QString("%1").arg(hour));
            break;
        }
        case WeeklyReport: {
            QDateTime dateTime = m_startDateTime.addDays(i);
            int week = dateTime.date().dayOfWeek();
            histogramGridData.xAxisStrings.append(weekString(week));
            break;
        }
        case MonthlyReport: {
            QDateTime dateTime = m_startDateTime.addDays(i);
            histogramGridData.xAxisStrings.append(dateTime.toString("dd"));
            break;
        }
        case YearlyReport: {
            QDateTime dateTime = m_startDateTime.addMonths(i);
            histogramGridData.xAxisStrings.append(dateTime.toString("M"));
            break;
        }
        default:
            break;
        }
    }
    m_cameraDataMap.insert(m_currentChannel, cameraData);

    //直方图2, x轴刻度
    histogramGridData2.xAxisStrings.clear();
}

void PeopleCountingData::clearCameraMap()
{
    m_cameraDataMap.clear();
}

bool PeopleCountingData::isHasMapKey(int key)
{
    if (m_cameraDataMap.find(key) != m_cameraDataMap.end()) {
        return true;
    }
    return false;
}

QStringList PeopleCountingData::calculateTotal(const QList<QVariant> &list, const QString &key)
{
    QStringList itemList;
    for (int var = 0; var < 4; ++var) {
        auto itemMap = list[var].toMap();
        QString strCount = itemMap.value(key).toString();
        QStringList strCountList = strCount.split(",");
        for (int i = 0; i < strCountList.size(); ++i) {
            if (var == 0) {
                itemList.append(strCountList.at(i));
            } else {
                int num = itemList.at(i).toInt();
                num += strCountList.at(i).toInt();
                itemList[i] = QString("%1").arg(num);
            }
        }
    }
    return itemList;
}

void PeopleCountingData::setCurrentChannel(int channel)
{
    m_currentChannel = channel;
}

int PeopleCountingData::currentChannel() const
{
    return m_currentChannel;
}

/**
 * @brief PeopleCountingDataThread::PeopleCountingDataThread
 * @param parent
 */
PeopleCountingDataThread::PeopleCountingDataThread(QObject *parent)
    : QThread(parent)
{
}

PeopleCountingDataThread::~PeopleCountingDataThread()
{
}

void PeopleCountingDataThread::run()
{
    startSecUTC = startDateTime.toTime_t();
    endSecUTC = endDateTime.toTime_t();
    count = 0;
    if (logs) {
        release_peoplecnt_log(&logs);
        logs = nullptr;
    }
    //
    QElapsedTimer timer;
    timer.start();
    SEARCH_PEOPLECNT_LOGS search;
    search.type = searchType;
    search.id = id;
    search.pdstart = startSecUTC / 86400;
    search.pdend = endSecUTC / 86400;
    qMsDebug() << "\nread_peoplecnt_logs begin"
               << "\ntype:" << search.type
               << "\nid:" << search.id
               << "\npdstart:" << search.pdstart
               << "\npdend:" << search.pdend
               << "\n"
               << QString("local time: %1 - %2").arg(startDateTime.toString("yyyy-MM-dd HH:mm:ss")).arg(endDateTime.toString("yyyy-MM-dd HH:mm:ss"))
               << "\n"
               << QString("utc time: %1 - %2").arg(startDateTime.toUTC().toString("yyyy-MM-dd HH:mm:ss")).arg(endDateTime.toUTC().toString("yyyy-MM-dd HH:mm:ss"));
    read_peoplecnt_logs(SQLITE_PEOPLE_NAME, &logs, &count, &search);
    qMsDebug() << "\nread_peoplecnt_logs end"
               << "\ncount:" << count;
    qMsDebug() << "read_peoplecnt_logs tooks:" << timer.elapsed() << "ms";

    //debug
#if 0
    QString text;
    text.append(QString("\ndate\tgroup\tchannel\tlocal\n"));
    for (int i = 0; i < m_count; ++i)
    {
        const peoplecnt_logs &log = m_logs[i];
        QDateTime startDateTime = QDateTime::fromTime_t(log.date * 86400);
        QDateTime endDateTime = startDateTime.addDays(1);
        text.append(QString("%1\t%2\t%3\t").arg(log.date).arg(log.groupid).arg(log.chnid));
        text.append(QString("%1 - %2\t\n").arg(startDateTime.toString("yyyy-MM-dd HH:mm:ss")).arg(endDateTime.toString("yyyy-MM-dd HH:mm:ss")));
    }
    MsDebug() << qPrintable(text);
#endif

    //
    switch (searchType) {
    case PEOPLECNT_SEARCH_BY_GROUP:
        makeGroupData();
        break;
    case PEOPLECNT_SEARCH_BY_CAMERA:
        cameraData = makeCameraData();
        break;
    default:
        break;
    }
    qMsDebug() << "searchData tooks:" << timer.elapsed() << "ms";

    emit searchFinished();
}

void PeopleCountingDataThread::makeGroupData()
{
    groupData.clear();

    for (int i = 0; i < checkedList.size(); ++i) {
        int group_id = checkedList.at(i);
        groupData.peopleDataMap[group_id] = makeCameraData(group_id);
    }
}

PeopleCameraData PeopleCountingDataThread::makeCameraData(int groupFilter)
{
    PeopleCameraData cameraData;
    auto &lineGridData = cameraData.lineGridData;
    auto &histogramGridData = cameraData.histogramGridData;
    auto &histogramGridData2 = cameraData.histogramGridData2;
    auto &lineData = cameraData.lineData;
    auto &histogramData = cameraData.histogramData;

    //x轴刻度数量
    switch (reportType) {
    case DailyReport:
        lineGridData.xMaxValue = 23;
        lineGridData.xLineCount = 24;
        histogramGridData.xMaxValue = lineGridData.xMaxValue + 1;
        histogramGridData.xLineCount = lineGridData.xLineCount + 1;
        histogramGridData2.xMaxValue = 11;
        histogramGridData2.xLineCount = 12;
        break;
    case WeeklyReport:
        lineGridData.xMaxValue = 6;
        lineGridData.xLineCount = 7;
        histogramGridData.xMaxValue = lineGridData.xMaxValue + 1;
        histogramGridData.xLineCount = lineGridData.xLineCount + 1;
        histogramGridData2.xMaxValue = 11;
        histogramGridData2.xLineCount = 12;
        break;
    case MonthlyReport:
        lineGridData.xMaxValue = startDateTime.daysTo(endDateTime);
        lineGridData.xLineCount = lineGridData.xMaxValue + 1;
        histogramGridData.xMaxValue = lineGridData.xMaxValue + 1;
        histogramGridData.xLineCount = lineGridData.xLineCount + 1;
        histogramGridData2.xMaxValue = 11;
        histogramGridData2.xLineCount = 12;
        break;
    case YearlyReport:
        //Group Search没有年报表
        break;
    }

    //
    QList<int> realCheckedList;
    if (groupFilter == -1) {
        realCheckedList = checkedList;
    } else {
        realCheckedList.append(TOTAL_LINE_INDEX);
        const PEOPLECNT_SETTING &setting = gPeopleCountingData.peopleInfo()->sets[groupFilter];
        QString checkedMask(setting.tri_channels);
        for (int i = 0; i < checkedMask.size(); ++i) {
            if (checkedMask.at(i) == QChar('1') && i < qMsNvr->maxChannel()) {
                realCheckedList.append(i);
            }
        }
    }
    QMap<int, QColor> colorMap;
    int colorIndex = -1;
    for (int i = 0; i < realCheckedList.size(); ++i) {
        int index = realCheckedList.at(i);
        lineData.lines.append(index);
        colorMap.insert(index, gPeopleCountingData.color(colorIndex));
        colorIndex++;
    }

    //
    int yMaxValue = 0;
    //line data
    for (int i = 0; i < count; ++i) {
        const peoplecnt_logs &log = logs[i];
        if (groupFilter != -1) {
            if (groupFilter != log.groupid) {
                continue;
            }
        }
        quint64 daySecs = log.date * 86400;
        for (int hour = 0; hour < MAX_LEN_24; ++hour) {
            quint64 currentSecs = daySecs + hour * 3600;
            if (currentSecs < startSecUTC || currentSecs > endSecUTC) {
                continue;
            }
            //
            int value = 0;
            switch (statisticsType) {
            case PeopleEntered:
                value = log.pcntin[hour];
                break;
            case PeopleExited:
                value = log.pcntout[hour];
                break;
            case PeopleAll:
                value = log.pcntin[hour] + log.pcntout[hour];
                break;
            default:
                break;
            }
            //
            int x = 0;
            if (reportType == DailyReport) {
                x = (currentSecs - startSecUTC) / 3600;
            } else {
                x = (daySecs - startSecUTC) / 86400;
            }
            //total line
            lineData.lineData[TOTAL_LINE_INDEX][x] += value;
            //other line
            lineData.lineData[log.chnid][x] += value;
            //y轴最大值
            int maxValue = lineData.lineData.value(TOTAL_LINE_INDEX).value(x);
            yMaxValue = qMax(yMaxValue, maxValue);
        }
    }

    //histogram data
    int yMaxValue2 = 0;
    histogramData.clear();
    for (int i = 0; i < lineData.lines.size(); ++i) {
        int lineIndex = lineData.lines.at(i);
        const QMap<int, int> &valueMap = lineData.lineData.value(lineIndex);
        //histogram
        int yTotal = 0;
        for (int j = 0; j < lineGridData.xLineCount; ++j) {
            int xValue = j;
            int yValue = valueMap.value(j, 0);
            yTotal += yValue;
            histogramData.histogramData[xValue][HistogramKey(lineIndex, yValue)] = HistogramValue(yValue, colorMap.value(lineIndex));
        }
        //histogram2
        histogramData.histogramData2[lineIndex] = HistogramValue(yTotal, colorMap.value(lineIndex));
        yMaxValue2 = qMax(yMaxValue2, yTotal);
    }

    //y轴最大值
    lineGridData.yMaxValue = yAxisMaxValue(yMaxValue);
    histogramGridData.yMaxValue = yAxisMaxValue(yMaxValue);
    histogramGridData2.yMaxValue = yAxisMaxValue(yMaxValue2);

    //折线图, x轴刻度
    lineGridData.xAxisStrings.clear();
    for (int i = 0; i < lineGridData.xLineCount; ++i) {
        switch (reportType) {
        case DailyReport: {
            int hour = startDateTime.time().hour() + i;
            if (hour > 23) {
                hour = 0;
            }
            lineGridData.xAxisStrings.append(QString("%1").arg(hour));
            break;
        }
        case WeeklyReport: {
            QDateTime dateTime = startDateTime.addDays(i);
            int week = dateTime.date().dayOfWeek();
            lineGridData.xAxisStrings.append(weekString(week));
            break;
        }
        case MonthlyReport: {
            QDateTime dateTime = startDateTime.addDays(i);
            lineGridData.xAxisStrings.append(dateTime.toString("dd"));
            break;
        }
        default:
            break;
        }
    }

    //直方图, x轴刻度
    histogramGridData.xAxisStrings.clear();
    for (int i = 0; i < histogramGridData.xLineCount; ++i) {
        switch (reportType) {
        case DailyReport: {
            int hour = startDateTime.time().hour() + i;
            if (hour > 23) {
                hour = 0;
            }
            histogramGridData.xAxisStrings.append(QString("%1").arg(hour));
            break;
        }
        case WeeklyReport: {
            QDateTime dateTime = startDateTime.addDays(i);
            int week = dateTime.date().dayOfWeek();
            histogramGridData.xAxisStrings.append(weekString(week));
            break;
        }
        case MonthlyReport: {
            QDateTime dateTime = startDateTime.addDays(i);
            histogramGridData.xAxisStrings.append(dateTime.toString("dd"));
            break;
        }
        default:
            break;
        }
    }

    //直方图2, x轴刻度
    histogramGridData2.xAxisStrings.clear();

    //
    return cameraData;
}
