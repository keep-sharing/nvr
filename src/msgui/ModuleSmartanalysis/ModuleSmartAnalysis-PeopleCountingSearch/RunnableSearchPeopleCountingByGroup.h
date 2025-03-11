#ifndef RUNNABLESEARCHPEOPLECOUNTINGBYGROUP_H
#define RUNNABLESEARCHPEOPLECOUNTINGBYGROUP_H

#include <QDateTime>
#include <QMap>
#include <QMetaType>
#include <QRunnable>

extern "C" {
#include "msdb.h"
}

class RunnableSearchPeopleCountingByGroup : public QRunnable {
public:
    struct SearchInfo {
        QList<int> groupList;
        QDateTime startDateTime;
        QDateTime endDateTime;
        REPORT_AUTO_BACKUP_TIME_RANGE_E timeRange;
    };

    struct ResultInfo {
        //QMap<group, QMap<row, QMap<channel, value>>>
        QMap<int, QMap<int, QMap<int, int>>> dataMap;
        QDateTime startDateTime;
    };

    RunnableSearchPeopleCountingByGroup(QObject *obj, const QString &member, const SearchInfo &info);

    void run() override;

private:
    QObject *m_obj = nullptr;
    QString m_member;
    SearchInfo m_searchInfo;
};
Q_DECLARE_METATYPE(RunnableSearchPeopleCountingByGroup::SearchInfo)
Q_DECLARE_METATYPE(RunnableSearchPeopleCountingByGroup::ResultInfo)

#endif // RUNNABLESEARCHPEOPLECOUNTINGBYGROUP_H
