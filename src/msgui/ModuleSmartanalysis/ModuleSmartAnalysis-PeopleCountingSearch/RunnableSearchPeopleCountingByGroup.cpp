#include "RunnableSearchPeopleCountingByGroup.h"
#include "MyDebug.h"
#include "PeopleCountingData.h"

RunnableSearchPeopleCountingByGroup::RunnableSearchPeopleCountingByGroup(QObject *obj, const QString &member, const SearchInfo &info)
    : QRunnable()
    , m_obj(obj)
    , m_member(member)
    , m_searchInfo(info)
{
    qRegisterMetaType<RunnableSearchPeopleCountingByGroup::ResultInfo>("RunnableSearchPeopleCountingByGroup::ResultInfo");
}

void RunnableSearchPeopleCountingByGroup::run()
{
    ResultInfo resultInfo;
    resultInfo.startDateTime = m_searchInfo.endDateTime;
    auto &dataMap = resultInfo.dataMap;

    int logCount = 0;
    peoplecnt_logs *logs = nullptr;

    uint startSecs = m_searchInfo.startDateTime.toTime_t();
    uint endSecs = m_searchInfo.endDateTime.toTime_t();

    auto groupChannels = gPeopleCountingData.groupsAllChannels();

    SEARCH_PEOPLECNT_LOGS search;
    memset(&search, 0, sizeof(search));
    search.type = PEOPLECNT_SEARCH_BY_GROUP;
    for (int i = 0; i < m_searchInfo.groupList.size(); ++i) {
        int group = m_searchInfo.groupList.at(i);
        search.id |= (static_cast<quint64>(1) << group);
        //没有数据的也要生成
        if (groupChannels.value(group).size() > 0) {
            dataMap[group];
        }
    }
    search.pdstart = startSecs / 86400;
    search.pdend = endSecs / 86400;
    qMsDebug() << "\nread_peoplecnt_logs begin"
               << "\ntype:" << search.type
               << "\nid:" << search.id
               << "\npdstart:" << search.pdstart
               << "\npdend:" << search.pdend
               << "\n"
               << QString("local time: %1 - %2").arg(m_searchInfo.startDateTime.toString("yyyy-MM-dd HH:mm:ss")).arg(m_searchInfo.endDateTime.toString("yyyy-MM-dd HH:mm:ss"))
               << "\n"
               << QString("utc time: %1 - %2").arg(m_searchInfo.startDateTime.toUTC().toString("yyyy-MM-dd HH:mm:ss")).arg(m_searchInfo.endDateTime.toUTC().toString("yyyy-MM-dd HH:mm:ss"));
    read_peoplecnt_logs(SQLITE_PEOPLE_NAME, &logs, &logCount, &search);
    //
    for (int j = 0; j < logCount; ++j) {
        const peoplecnt_logs &log = logs[j];
        if (!m_searchInfo.groupList.contains(log.groupid)) {
            continue;
        }
        auto &groupData = dataMap[log.groupid];
        quint64 daySecs = log.date * 86400;
        for (int hour = 0; hour < MAX_LEN_24; ++hour) {
            quint64 currentSecs = daySecs + hour * 3600;
            if (currentSecs < startSecs || currentSecs >= endSecs) {
                continue;
            }
            //
            int value = log.pcntin[hour] + log.pcntout[hour];
            if (value > 0) {
                //搜索全部时，获取最开始有数据的时间作为邮件的开始时间
                QDateTime time = QDateTime::fromTime_t(currentSecs);
                if (time < resultInfo.startDateTime) {
                    resultInfo.startDateTime = time;
                }
            }
            //
            switch (m_searchInfo.timeRange) {
            case REPORT_AUTO_BACKUP_TIME_RANGE_LAST_DAY:
                groupData[currentSecs][log.chnid] += value;
                break;
            default:
                groupData[daySecs][log.chnid] += value;
                break;
            }
        }
    }
    //
    if (logs) {
        release_peoplecnt_log(&logs);
        logs = nullptr;
    }
    //
    QMetaObject::invokeMethod(m_obj, m_member.toLocal8Bit().data(), Qt::DirectConnection, Q_ARG(RunnableSearchPeopleCountingByGroup::ResultInfo, resultInfo));
}
