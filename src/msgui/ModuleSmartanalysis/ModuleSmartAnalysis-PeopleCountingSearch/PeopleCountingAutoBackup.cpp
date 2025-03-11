#include "PeopleCountingAutoBackup.h"
#include "DrawItemTimeHeatMap.h"
#include "LiveView.h"
#include "MessageFilter.h"
#include "MsDevice.h"
#include "MsMessage.h"
#include "MyDebug.h"
#include "PeopleCountingData.h"
#include "myqt.h"
#include "qjson/include/parser.h"
#include <QFile>
#include <QPainter>
#include <QStringList>
#include <QThreadPool>
#include <qmath.h>

PeopleCountingAutoBackup::PeopleCountingAutoBackup()
    : MsObject()
{
    moveToThread(&m_thread);
    connect(&m_thread, SIGNAL(started()), this, SLOT(onThreadStarted()));
    connect(&m_thread, SIGNAL(finished()), this, SLOT(onThreadFinished()));
    m_thread.start();
    gMessageFilter.installMessageFilter(RESPONSE_FLAG_GUI_PEOPLECNT_AUTO_BACKUP, this);
}

PeopleCountingAutoBackup::~PeopleCountingAutoBackup()
{
    m_thread.quit();
    m_thread.wait();
}

PeopleCountingAutoBackup &PeopleCountingAutoBackup::instance()
{
    static PeopleCountingAutoBackup self;
    return self;
}

void PeopleCountingAutoBackup::filterMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GUI_PEOPLECNT_AUTO_BACKUP:
        ON_RESPONSE_FLAG_GUI_PEOPLECNT_AUTO_BACKUP(message);
        break;
    }
}

void PeopleCountingAutoBackup::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPC_PEOPLE_REPORT:
        ON_RESPONSE_FLAG_GET_IPC_PEOPLE_REPORT(message);
        break;
    case RESPONSE_FLAG_GET_EXPORT_DISK:
        ON_RESPONSE_FLAG_GET_EXPORT_DISK(message);
        break;
    case RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY:
        ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(message);
        break;
    case RESPONSE_FLAG_GET_IPC_HEATMAP_SUPPORT:
        ON_RESPONSE_FLAG_GET_IPC_HEATMAP_SUPPORT(message);
        break;
    case RESPONSE_FLAG_GET_IPC_SNAPHOST:
        ON_RESPONSE_FLAG_GET_IPC_SNAPHOST(message);
        break;
    case RESPONSE_FLAG_SEARCH_HEAT_MAP_REPORT:
        ON_RESPONSE_FLAG_SEARCH_HEAT_MAP_REPORT(message);
        break;
    }
}

void PeopleCountingAutoBackup::ON_RESPONSE_FLAG_GUI_PEOPLECNT_AUTO_BACKUP(MessageReceive *message)
{
    qMsDebug() << "begin";
    REPORT_AUTO_BACKUP_SETTING_S *data = static_cast<REPORT_AUTO_BACKUP_SETTING_S *>(message->data);
    m_mutex.lock();
    m_taskList.append(*data);
    qMsDebug() << "isWorking:" << m_isWorking;
    if (!m_isWorking) {
        QMetaObject::invokeMethod(this, "startBackupPeopleCount", Qt::QueuedConnection);
        m_isWorking = true;
    }
    m_mutex.unlock();
    qMsDebug() << "end";
}

void PeopleCountingAutoBackup::ON_RESPONSE_FLAG_GET_IPC_PEOPLE_REPORT(MessageReceive *message)
{
    char *data = static_cast<char *>(message->data);
    m_mutex.lock();
    m_reportText = QString(data);
    m_wait.wakeAll();
    m_mutex.unlock();
}

void PeopleCountingAutoBackup::ON_RESPONSE_FLAG_GET_EXPORT_DISK(MessageReceive *message)
{
    QString path;
    struct resp_usb_info *usb_info_list = (struct resp_usb_info *)message->data;
    int count = message->header.size / sizeof(struct resp_usb_info);
    for (int i = 0; i < count; ++i) {
        const resp_usb_info &usb_info = usb_info_list[i];
        if (!QString(usb_info.dev_path).isEmpty() && usb_info.type != DISK_TYPE_NAS && usb_info.bRec != true) {
            path = QString(usb_info.dev_path);
            break;
        }
    }
    m_mutex.lock();
    m_usbPath = path;
    m_wait.wakeAll();
    m_mutex.unlock();
}

void PeopleCountingAutoBackup::ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(MessageReceive *message)
{
    m_mutex.lock();
    resp_image_display *image_display = static_cast<resp_image_display *>(message->data);
    m_heatmapData.corridorMode = image_display->image.corridormode;
    m_heatmapData.imageRotation = image_display->image.imagerotation;
    m_heatmapData.lencorrect = image_display->image.lencorrect;

    m_wait.wakeAll();
    m_mutex.unlock();
}

void PeopleCountingAutoBackup::ON_RESPONSE_FLAG_GET_IPC_HEATMAP_SUPPORT(MessageReceive *message)
{
    m_mutex.lock();
    m_heatmapSupport = *((int *)message->data);
    m_wait.wakeAll();
    m_mutex.unlock();
}

void PeopleCountingAutoBackup::ON_RESPONSE_FLAG_GET_IPC_SNAPHOST(MessageReceive *message)
{
    RespSnapshotChatHeader *header = static_cast<RespSnapshotChatHeader *>(message->data);
    m_mutex.lock();
    if (header) {
        if (header->result != -1 && header->result != -2) {
            message->image1 = QImage::fromData(static_cast<uchar *>(message->data) + sizeof(RespSnapshotChatHeader), message->header.size - sizeof(RespSnapshotChatHeader));
            m_heatmapData.backgroundImage = message->image1;
        } else {
            m_heatmapData.backgroundImage = QImage();
        }
    }
    m_wait.wakeAll();
    m_mutex.unlock();
}

void PeopleCountingAutoBackup::ON_RESPONSE_FLAG_SEARCH_HEAT_MAP_REPORT(MessageReceive *message)
{
    char *report = static_cast<char *>(message->data);
    m_mutex.lock();
    if (report) {
        QByteArray ba(report, message->header.size);
        QString strText(ba);
        m_heatmapData.text = strText;
    } else {
        m_heatmapData.text = "";
    }
    m_wait.wakeAll();
    m_mutex.unlock();
}

void PeopleCountingAutoBackup::makeGroupData(const REPORT_AUTO_BACKUP_SETTING_S &setting, BackupInfo &info)
{
    RunnableSearchPeopleCountingByGroup::SearchInfo search;
    for (int i = 0; i < MAX_PEOPLECNT_GROUP; ++i) {
        if (setting.triGroups[i] == '1') {
            search.groupList.append(i);
            info.channelCount++;
        }
    }
    if (search.groupList.isEmpty()) {
        info.error = -1;
        return;
    }
    search.timeRange = (REPORT_AUTO_BACKUP_TIME_RANGE_E)setting.timeRange;
    //进一法精确到小时
    uint time = QDateTime(QDate::currentDate(), QTime::fromString(setting.backupTime, "HH:mm:ss")).toTime_t();
    int hour = qCeil(time / 3600.0);
    search.endDateTime = QDateTime::fromTime_t(hour * 3600);
    switch (search.timeRange) {
    case REPORT_AUTO_BACKUP_TIME_RANGE_ALL:
        search.startDateTime = QDateTime(QDate(2020, 1, 1), QTime(0, 0));
        break;
    case REPORT_AUTO_BACKUP_TIME_RANGE_LAST_DAY:
        search.startDateTime = search.endDateTime.addDays(-1);
        break;
    case REPORT_AUTO_BACKUP_TIME_RANGE_LAST_WEEK:
        search.startDateTime = search.endDateTime.addDays(-7);
        break;
    default:
        qMsWarning() << "invalid time range:" << setting.timeRange;
        return;
    }

    m_mutex.lock();
    QThreadPool::globalInstance()->start(new RunnableSearchPeopleCountingByGroup(this, "onSearchByGroupFinished", search));
    m_wait.wait(&m_mutex);
    m_mutex.unlock();
    //
    QMap<int, QList<int>> groupChannelMap = gPeopleCountingData.groupsAllChannels();

    ms_system("rm -r /tmp/peoplecnt_by_group > /dev/null");
    ms_system("mkdir -p /tmp/peoplecnt_by_group > /dev/null");

    info.backupDate = QDate::currentDate();
    info.backupTime = QTime::fromString(setting.backupTime, "HH:mm:ss");
    info.backupDateTime = QDateTime(info.backupDate, info.backupTime);

    info.mailTitle = "People Counting by Group Report";
    info.mailReportType = "People Counting by Group";

    info.packageName = QString("NVR_People_Counting_by_Group_%1.tar").arg(info.backupDateTime.toString("yyyyMMddHHmmss"));
    info.packagePath = QString("/tmp/peoplecnt_by_group/%1").arg(info.packageName);

    if (setting.timeRange == REPORT_AUTO_BACKUP_TIME_RANGE_ALL) {
        info.mailStartDateTime = m_groupSearchInfo.startDateTime;
        info.mailEndDateTime = search.endDateTime;
    } else {
        info.mailStartDateTime = search.startDateTime;
        info.mailEndDateTime = search.endDateTime;
    }

    for (auto groupIter = m_groupSearchInfo.dataMap.constBegin(); groupIter != m_groupSearchInfo.dataMap.constEnd(); ++groupIter) {
        int group = groupIter.key();
        const auto &channels = groupChannelMap.value(group);
        QString filePath = QString("/tmp/peoplecnt_by_group/NVR_People_Counting_by_Group_Group%1_%2.csv").arg(group + 1, 2, 10, QLatin1Char('0')).arg(info.backupDateTime.toString("yyyyMMddHHmmss"));
        QFile file(filePath);
        if (!file.open(QFile::WriteOnly)) {
            qMsWarning() << filePath << file.errorString();
            continue;
        }
        info.filePathList.append(filePath);
        QTextStream stream(&file);
        stream << "Time,"
               << "Total Sum,";
        for (int i = 0; i < channels.size(); ++i) {
            int channel = channels.at(i);
            stream << QString("CH%1").arg(channel + 1, 2, 10, QLatin1Char('0')) << ",";
        }
        stream << "\n";
        const auto &groupData = groupIter.value();
        for (auto rowIter = groupData.constBegin(); rowIter != groupData.constEnd(); ++rowIter) {
            uint secs = rowIter.key();
            switch (setting.timeRange) {
            case REPORT_AUTO_BACKUP_TIME_RANGE_LAST_DAY:
                stream << QString("%1-%2").arg(QDateTime::fromTime_t(secs).toString("yyyy/MM/dd HH:mm")).arg(QDateTime::fromTime_t(secs + 3600).toString("HH:mm")) << ",";
                break;
            case REPORT_AUTO_BACKUP_TIME_RANGE_LAST_WEEK:
            case REPORT_AUTO_BACKUP_TIME_RANGE_ALL:
                stream << QString("%1").arg(QDateTime::fromTime_t(secs).toString("yyyy/MM/dd")) << ",";
                break;
            default:
                qMsWarning() << "invalid time range:" << setting.timeRange;
                return;
            }
            //
            int total = 0;
            const auto &valueMap = rowIter.value();
            for (auto channelIter = valueMap.constBegin(); channelIter != valueMap.constEnd(); ++channelIter) {
                total += channelIter.value();
            }
            stream << total << ",";
            //
            for (int i = 0; i < channels.size(); ++i) {
                int channel = channels.at(i);
                int value = valueMap.value(channel, 0);
                stream << value << ",";
            }
            stream << "\n";
        }
        file.close();
    }
}

void PeopleCountingAutoBackup::makeCameraData(const REPORT_AUTO_BACKUP_SETTING_S &setting, BackupInfo &info)
{
    int channelCount = 0;
    for (int i = 0; i < MAX_REAL_CAMERA; ++i) {
        if (setting.triChannels[i] == '1') {
            channelCount++;
        }
    }
    if (channelCount < 1) {
        info.error = -1;
        return;
    }

    info.backupDate = QDate::currentDate();
    info.backupTime = QTime::fromString(setting.backupTime, "HH:mm:ss");
    info.backupDateTime = QDateTime(info.backupDate, info.backupTime);

    info.mailStartDateTime = info.backupDateTime;
    info.mailEndDateTime = info.backupDateTime;

    ms_system("rm -r /tmp/peoplecnt_by_camera > /dev/null");
    ms_system("mkdir -p /tmp/peoplecnt_by_camera > /dev/null");

    info.mailTitle = "People Counting by Camera Report";
    info.mailReportType = "People Counting by Camera";

    //get ipc data
    ReqIpcPeopleReport report;
    memset(&report, 0, sizeof(report));
    report.mainType = 0;
    report.lineMask = setting.lineMask & 15;
    if (setting.lineMask & 16) {
        report.lineMask |= 15;
    }
    switch (setting.timeRange) {
    case REPORT_AUTO_BACKUP_TIME_RANGE_ALL: {
        report.reportType = 4;
        break;
    }
    case REPORT_AUTO_BACKUP_TIME_RANGE_LAST_DAY: {
        report.reportType = 0;
        QDateTime startDateTime = info.backupDateTime.addDays(-1);
        snprintf(report.startTime, sizeof(report.startTime), "%s", startDateTime.toString("yyyy-MM-dd-HH:mm:ss").toStdString().c_str());
        info.mailStartDateTime = startDateTime;
        break;
    }
    case REPORT_AUTO_BACKUP_TIME_RANGE_LAST_WEEK: {
        report.reportType = 1;
        QDateTime startDateTime = info.backupDateTime.addDays(-7);
        snprintf(report.startTime, sizeof(report.startTime), "%s", startDateTime.toString("yyyy-MM-dd-HH:mm:ss").toStdString().c_str());
        info.mailStartDateTime = startDateTime;
        break;
    }
    default:
        qMsCritical() << "invalid time range:" << setting.timeRange;
        return;
    }
    //
    for (int channel = 0; channel < MAX_REAL_CAMERA; ++channel) {
        if (setting.triChannels[channel] != '1') {
            continue;
        }
        //
        report.chnid = channel;
        m_mutex.lock();
        sendMessage(REQUEST_FLAG_GET_IPC_PEOPLE_REPORT, &report, sizeof(report));
        m_wait.wait(&m_mutex);
        QString text = m_reportText;
        m_mutex.unlock();
        qMsDebug() << "channel:" << channel << "\n"
                   << qPrintable(text);
        if (text.isEmpty()) {
            qMsWarning() << "text is empty";
            continue;
        }
        //QMap<line, QList<column>>
        int lineMask = setting.lineMask << 1;
        lineMask |= (lineMask & (1 << 5)) >> 5;
        for (int line = 0; line < 5; line++) {
            if (lineMask & (1 << line)) {
                QMap<int, QList<QString>> dataMap;
                QString filePath;
                if (line > 0) {
                    filePath = QString("/tmp/peoplecnt_by_camera/NVR_People_Counting_by_Camera_CH%1_Line%2_%3.csv").arg(channel + 1, 2, 10, QLatin1Char('0')).arg(line).arg(info.backupDateTime.toString("yyyyMMddHHmmss"));
                } else {
                    filePath = QString("/tmp/peoplecnt_by_camera/NVR_People_Counting_by_Camera_CH%1_Total_%2.csv").arg(channel + 1, 2, 10, QLatin1Char('0')).arg(info.backupDateTime.toString("yyyyMMddHHmmss"));
                }

                info.packageName = QString("NVR_People_Counting_by_Camera_%1.tar").arg(info.backupDateTime.toString("yyyyMMddHHmmss"));
                info.packagePath = QString("/tmp/peoplecnt_by_camera/%1").arg(info.packageName);
                auto &titleList = dataMap[0];
                titleList << "Time"
                          << "Sum"
                          << "People Entered"
                          << "People Existed";
                //
                QJson::Parser parser;
                bool ok;
                auto result = parser.parse(text.toLatin1(), &ok);
                if (ok) {
                    auto rootMap = result.toMap();
                    auto rootList = rootMap.value("report").toList();
                    QStringList incountList;
                    QStringList outcountList;
                    QString starttime;
                    QString endtime;

                    if (!rootList.isEmpty()) {
                        if (line > 0) {
                            auto itemMap = rootList[line - 1].toMap();
                            starttime = itemMap.value("starttime").toString();
                            endtime = itemMap.value("endtime").toString();

                            QString incount = itemMap.value("incount").toString();
                            QString outcount = itemMap.value("outcount").toString();
                            incountList = incount.split(",");
                            outcountList = outcount.split(",");
                        } else {
                            calculateTime(rootList, starttime, endtime);
                            incountList = calculateTotal(rootList, "incount");
                            outcountList = calculateTotal(rootList, "outcount");
                        }
                    } else if (!rootMap.isEmpty() && (line == 0 || line == 1)) {
                        starttime = rootMap.value("starttime").toString();
                        endtime = rootMap.value("endtime").toString();

                        QString incount = rootMap.value("incount").toString();
                        QString outcount = rootMap.value("outcount").toString();
                        incountList = incount.split(",");
                        outcountList = outcount.split(",");
                    } else {
                        continue;
                    }
                    QStringList starttimeList = starttime.split(",", QString::SkipEmptyParts);
                    QStringList endtimeList = endtime.split(",", QString::SkipEmptyParts);
                    if (incountList.size() != outcountList.size()) {
                        qMsCritical() << "invalid text.";
                        continue;
                    }
                    if (setting.timeRange == REPORT_AUTO_BACKUP_TIME_RANGE_ALL) {
                        if (starttimeList.size() != incountList.size() || endtimeList.size() != incountList.size()) {
                            qMsWarning() << "invalid text, probably IPC not supported 'Export All'.";
                            continue;
                        }
                    }
                    //邮件起止时间
                    if (!starttimeList.isEmpty()) {
                        QDateTime dateTime = QDateTime::fromString(starttimeList.first(), "yyyy/MM/dd HH:mm:ss");
                        if (info.mailStartDateTime == info.backupDateTime) {
                            info.mailStartDateTime = dateTime;
                        } else if (dateTime < info.mailStartDateTime) {
                            info.mailStartDateTime = dateTime;
                        }
                    }
                    if (!endtimeList.isEmpty()) {
                        QDateTime dateTime = QDateTime::fromString(endtimeList.last(), "yyyy/MM/dd HH:mm:ss");
                        if (info.mailEndDateTime == info.backupDateTime) {
                            info.mailEndDateTime = dateTime;
                        } else if (dateTime > info.mailEndDateTime) {
                            info.mailEndDateTime = dateTime;
                        }
                    }
                    //
                    if (!starttimeList.isEmpty()) {
                        QDateTime startDateTime = QDateTime::fromString(starttimeList.first(), "yyyy/MM/dd HH:mm:ss");
                        for (int time = 0; time < incountList.size(); ++time) {
                            auto &row = dataMap[time + 1];
                            switch (setting.timeRange) {
                            case REPORT_AUTO_BACKUP_TIME_RANGE_ALL: {
                                QDateTime dateTime = QDateTime::fromString(starttimeList.at(time), "yyyy/MM/dd HH:mm:ss");
                                row << QString("%1").arg(dateTime.toString("yyyy/MM/dd"));
                                break;
                            }
                            case REPORT_AUTO_BACKUP_TIME_RANGE_LAST_DAY: {
                                QDateTime start = startDateTime.addSecs(time * 3600);
                                QDateTime end = start.addSecs(3600);
                                int hour1 = start.time().hour();
                                int hour2 = end.time().hour();
                                row << QString("%1 %2:00-%3:00").arg(start.toString("yyyy/MM/dd")).arg(hour1, 2, 10, QLatin1Char('0')).arg(hour2, 2, 10, QLatin1Char('0'));
                                break;
                            }
                            case REPORT_AUTO_BACKUP_TIME_RANGE_LAST_WEEK: {
                                QDateTime dateTime = startDateTime.addDays(time);
                                row << QString("%1 %2").arg(dateTime.toString("yyyy/MM/dd")).arg(MyQt::weekString(dateTime.date().dayOfWeek()));
                                break;
                            }
                            default:
                                break;
                            }
                            int in = incountList.at(time).toInt();
                            int out = outcountList.at(time).toInt();
                            int sum = in + out;
                            row.append(QString::number(sum));
                            row.append(QString::number(in));
                            row.append(QString::number(out));
                        }
                    }
                } else {
                    qMsWarning() << "invalid json:" << text;
                    //不支持
                    continue;
                }
                //
                QFile file(filePath);
                if (file.open(QFile::WriteOnly)) {
                    QTextStream stream(&file);
                    for (auto iter = dataMap.constBegin(); iter != dataMap.constEnd(); ++iter) {
                        const QList<QString> &list = iter.value();
                        for (int i = 0; i < list.size(); ++i) {
                            stream << list.at(i) << ",";
                        }
                        stream << "\n";
                    }
                    file.close();
                    info.filePathList.append(filePath);
                } else {
                    qMsWarning() << filePath << file.errorString();
                }
            }
        }

        //
    }
}

void PeopleCountingAutoBackup::makeRegionData(const REPORT_AUTO_BACKUP_SETTING_S &setting, PeopleCountingAutoBackup::BackupInfo &info)
{
    int channelCount = 0;
    for (int i = 0; i < MAX_REAL_CAMERA; ++i) {
        if (setting.triChannels[i] == '1') {
            channelCount++;
        }
    }
    if (channelCount < 1) {
        info.error = -1;
        return;
    }

    info.backupDate = QDate::currentDate();
    info.backupTime = QTime::fromString(setting.backupTime, "HH:mm:ss");
    info.backupDateTime = QDateTime(info.backupDate, info.backupTime);

    info.mailStartDateTime = info.backupDateTime;
    info.mailEndDateTime = info.backupDateTime;

    ms_system("rm -r /tmp/peoplecnt_by_region > /dev/null");
    ms_system("mkdir -p /tmp/peoplecnt_by_region > /dev/null");

    info.mailTitle = "Regional People Counting Report";
    info.mailReportType = "Regional People Counting";

    //get ipc data
    ReqIpcPeopleReport report;
    memset(&report, 0, sizeof(report));
    report.mainType = 1;
    switch (setting.timeRange) {
    case REPORT_AUTO_BACKUP_TIME_RANGE_ALL: {
        report.reportType = 4;
        break;
    }
    case REPORT_AUTO_BACKUP_TIME_RANGE_LAST_DAY: {
        report.reportType = 0;
        QDateTime startDateTime = info.backupDateTime.addDays(-1);
        snprintf(report.startTime, sizeof(report.startTime), "%s", startDateTime.toString("yyyy-MM-dd-HH:mm:ss").toStdString().c_str());
        info.mailStartDateTime = startDateTime;
        break;
    }
    case REPORT_AUTO_BACKUP_TIME_RANGE_LAST_WEEK: {
        report.reportType = 1;
        QDateTime startDateTime = info.backupDateTime.addDays(-7);
        snprintf(report.startTime, sizeof(report.startTime), "%s", startDateTime.toString("yyyy-MM-dd-HH:mm:ss").toStdString().c_str());
        info.mailStartDateTime = startDateTime;
        break;
    }
    default:
        qMsCritical() << "invalid time range:" << setting.timeRange;
        return;
    }
    //
    for (int channel = 0; channel < MAX_REAL_CAMERA; ++channel) {
        if (setting.triChannels[channel] != '1') {
            continue;
        }
        //
        report.chnid = channel;
        m_mutex.lock();
        sendMessage(REQUEST_FLAG_GET_IPC_PEOPLE_REPORT, &report, sizeof(report));
        m_wait.wait(&m_mutex);
        QString text = m_reportText;
        m_mutex.unlock();
        qMsDebug() << "channel:" << channel << "\n"
                   << qPrintable(text);
        if (text.isEmpty()) {
            qMsWarning() << "text is empty";
            continue;
        }
        //QMap<line, QList<column>>
        QMap<int, QList<QString>> dataMap;

        QString filePath = QString("/tmp/peoplecnt_by_region/NVR_Regional_People_Counting_CH%1_%2.csv").arg(channel + 1, 2, 10, QLatin1Char('0')).arg(info.backupDateTime.toString("yyyyMMddHHmmss"));
        info.packageName = QString("NVR_Regional_People_Counting_%1.tar").arg(info.backupDateTime.toString("yyyyMMddHHmmss"));
        info.packagePath = QString("/tmp/peoplecnt_by_region/%1").arg(info.packageName);
        auto &titleList = dataMap[0];
        titleList << "Time"
                  << "Total"
                  << "Region1"
                  << "Region2"
                  << "Region3"
                  << "Region4";
        //
        QJson::Parser parser;
        bool ok;
        auto result = parser.parse(text.toLatin1(), &ok);
        if (ok) {
            auto rootMap = result.toMap();
            if (!rootMap.contains("incount0")) {
                qMsCritical() << "invalid text:" << text;
                continue;
            }
            QString incount0 = rootMap.value("incount0").toString();
            QString incount1 = rootMap.value("incount1").toString();
            QString incount2 = rootMap.value("incount2").toString();
            QString incount3 = rootMap.value("incount3").toString();
            if (!rootMap.contains("starttime")) {
                //不支持
                continue;
            }
            if (!rootMap.contains("endtime")) {
                //不支持
                continue;
            }
            QString starttime = rootMap.value("starttime").toString();
            QString endtime = rootMap.value("endtime").toString();
            QStringList incount0List = incount0.split(",", QString::SkipEmptyParts);
            QStringList incount1List = incount1.split(",", QString::SkipEmptyParts);
            QStringList incount2List = incount2.split(",", QString::SkipEmptyParts);
            QStringList incount3List = incount3.split(",", QString::SkipEmptyParts);
            QStringList starttimeList = starttime.split(",", QString::SkipEmptyParts);
            QStringList endtimeList = endtime.split(",", QString::SkipEmptyParts);
            if (incount0List.size() != incount1List.size() || incount2List.size() != incount3List.size() || incount0List.size() != incount2List.size()) {
                qMsCritical() << "invalid text.";
                continue;
            }
            if (setting.timeRange == REPORT_AUTO_BACKUP_TIME_RANGE_ALL) {
                if (starttimeList.size() != incount0List.size() || endtimeList.size() != incount0List.size()) {
                    qMsWarning() << "invalid text, probably IPC not supported 'Export All'.";
                    continue;
                }
            }
            //邮件起止时间
            if (!starttimeList.isEmpty()) {
                QDateTime dateTime = QDateTime::fromString(starttimeList.first(), "yyyy/MM/dd HH:mm:ss");
                if (info.mailStartDateTime == info.backupDateTime) {
                    info.mailStartDateTime = dateTime;
                } else if (dateTime < info.mailStartDateTime) {
                    info.mailStartDateTime = dateTime;
                }
            }
            if (!endtimeList.isEmpty()) {
                QDateTime dateTime = QDateTime::fromString(endtimeList.last(), "yyyy/MM/dd HH:mm:ss");
                if (info.mailEndDateTime == info.backupDateTime) {
                    info.mailEndDateTime = dateTime;
                } else if (dateTime > info.mailEndDateTime) {
                    info.mailEndDateTime = dateTime;
                }
            }
            //
            if (!starttimeList.isEmpty()) {
                QDateTime startDateTime = QDateTime::fromString(starttimeList.first(), "yyyy/MM/dd HH:mm:ss");
                for (int i = 0; i < incount0List.size(); ++i) {
                    auto &row = dataMap[i + 1];
                    switch (setting.timeRange) {
                    case REPORT_AUTO_BACKUP_TIME_RANGE_ALL: {
                        QDateTime dateTime = QDateTime::fromString(starttimeList.at(i), "yyyy/MM/dd HH:mm:ss");
                        row << QString("%1").arg(dateTime.toString("yyyy/MM/dd"));
                        break;
                    }
                    case REPORT_AUTO_BACKUP_TIME_RANGE_LAST_DAY: {
                        QDateTime start = startDateTime.addSecs(i * 3600);
                        QDateTime end = start.addSecs(3600);
                        int hour1 = start.time().hour();
                        int hour2 = end.time().hour();
                        row << QString("%1 %2:00-%3:00").arg(start.toString("yyyy/MM/dd")).arg(hour1, 2, 10, QLatin1Char('0')).arg(hour2, 2, 10, QLatin1Char('0'));
                        break;
                    }
                    case REPORT_AUTO_BACKUP_TIME_RANGE_LAST_WEEK: {
                        QDateTime dateTime = startDateTime.addDays(i);
                        row << QString("%1 %2").arg(dateTime.toString("yyyy/MM/dd")).arg(MyQt::weekString(dateTime.date().dayOfWeek()));
                        break;
                    }
                    default:
                        break;
                    }
                    int in0 = incount0List.at(i).toInt();
                    int in1 = incount1List.at(i).toInt();
                    int in2 = incount2List.at(i).toInt();
                    int in3 = incount3List.at(i).toInt();
                    int sum = in0 + in1 + in2 + in3;
                    row.append(QString::number(sum));
                    row.append(QString::number(in0));
                    row.append(QString::number(in1));
                    row.append(QString::number(in2));
                    row.append(QString::number(in3));
                }
            }
        }

        //
        QFile file(filePath);
        if (file.open(QFile::WriteOnly)) {
            QTextStream stream(&file);
            for (auto iter = dataMap.constBegin(); iter != dataMap.constEnd(); ++iter) {
                const QList<QString> &list = iter.value();
                for (int i = 0; i < list.size(); ++i) {
                    stream << list.at(i) << ",";
                }
                stream << "\n";
            }
            file.close();
            info.filePathList.append(filePath);
        } else {
            qMsWarning() << filePath << file.errorString();
        }
        //
    }
}

void PeopleCountingAutoBackup::makeHeatMapData(const REPORT_AUTO_BACKUP_SETTING_S &setting, PeopleCountingAutoBackup::BackupInfo &info)
{
    if (info.channelCount < 1) {
        info.error = -1;
        return;
    }

    info.backupDate = QDate::currentDate();
    info.backupTime = QTime::fromString(setting.backupTime, "HH:mm:ss");
    info.backupDateTime = QDateTime(info.backupDate, info.backupTime);

    info.mailStartDateTime = info.backupDateTime;
    info.mailEndDateTime = info.backupDateTime;

    ms_system("rm -r /tmp/heat_map > /dev/null");
    ms_system("mkdir -p /tmp/heat_map > /dev/null");

    info.mailTitle = "Heat Map Report";
    info.mailReportType = "Heat Map";

    //get heatmap data image & text
    ms_heat_map_report report;
    memset(&report, 0, sizeof(report));

    switch (setting.timeRange) {
    case REPORT_AUTO_BACKUP_TIME_RANGE_ALL: {
        report.subType = -1;
        QDateTime startDateTime = info.backupDateTime.addYears(-5);
        snprintf(report.pTime, sizeof(report.pTime), "%s", startDateTime.toString("yyyy-MM-dd-HH:mm:ss").toStdString().c_str());
        info.mailStartDateTime = startDateTime;
        break;
    }
    case REPORT_AUTO_BACKUP_TIME_RANGE_LAST_DAY: {
        report.subType = 0;
        QDateTime startDateTime = info.backupDateTime.addDays(-1);
        snprintf(report.pTime, sizeof(report.pTime), "%s", startDateTime.toString("yyyy-MM-dd-HH:mm:ss").toStdString().c_str());
        info.mailStartDateTime = startDateTime;
        break;
    }
    case REPORT_AUTO_BACKUP_TIME_RANGE_LAST_WEEK: {
        report.subType = 1;
        QDateTime startDateTime = info.backupDateTime.addDays(-7);
        snprintf(report.pTime, sizeof(report.pTime), "%s", startDateTime.toString("yyyy-MM-dd-HH:mm:ss").toStdString().c_str());
        info.mailStartDateTime = startDateTime;
        break;
    }
    case REPORT_AUTO_BACKUP_TIME_RANGE_LAST_HOUR:
    case REPORT_AUTO_BACKUP_TIME_RANGE_INTERVAL_1_HOUR: {
        report.subType = 4;
        QDateTime startDateTime = info.backupDateTime.addSecs(-3600);
        snprintf(report.pTime, sizeof(report.pTime), "%s", startDateTime.toString("yyyy-MM-dd-HH:mm:ss").toStdString().c_str());
        info.mailStartDateTime = startDateTime;
        break;
    }
    default:
        qMsCritical() << "invalid time range:" << setting.timeRange;
        return;
    }

    //space
    if (setting.fileFormat & REPORT_AUTO_BACKUP_FORMAT_HEATMAP_SPACE) {
        initializeColorImage();
        report.mainType = 0;
        for (int i = 0; i < MAX_REAL_CAMERA; ++i) {
            if (setting.triChannels[i] != '1') {
                continue;
            }
            if (!LiveView::instance()->isChannelConnected(i)) {
                continue;
            }
            mutexSendMessage(REQUEST_FLAG_GET_IPC_HEATMAP_SUPPORT, i);
            int result = m_heatmapSupport;

            if (result != HEATMAP_SUPPORT_YES) {
                qMsDebug() << "channel:" << i << "not support space";
                continue;
            }
            //get m_heatmapData
            if (qMsNvr->isFisheye(i)) {
                m_heatmapData.mode = ModeFisheye;
            } else {
                m_heatmapData.mode = ModePanoramicMiniBullet;
            }
            mutexSendMessage(REQUEST_FLAG_GET_IPCIMAGE_DISPLAY, i);
            mutexSendMessage(REQUEST_FLAG_GET_IPC_SNAPHOST, i);

            report.chnid = i;
            m_mutex.lock();
            sendMessage(REQUEST_FLAG_SEARCH_HEAT_MAP_REPORT, &report, sizeof(ms_heat_map_report));
            m_wait.wait(&m_mutex);
            QString text = m_heatmapData.text;
            m_mutex.unlock();
            qMsDebug() << "channel:" << i << "\n";
            if (text.isEmpty()) {
                qMsWarning() << "text is empty";
                continue;
            }
            QString filePath = QString("/tmp/heat_map/NVR_Heat_Map_CH%1_%2.png").arg(i + 1, 2, 10, QLatin1Char('0')).arg(info.backupDateTime.toString("yyyyMMddHHmmss"));
            info.packageName = QString("NVR_Heat_Map_%1.tar").arg(info.backupDateTime.toString("yyyyMMddHHmmss"));
            info.packagePath = QString("/tmp/heat_map/%1").arg(info.packageName);
            makeSpaceHeatMap(text, m_heatmapData.corridorMode, m_heatmapData.imageRotation, m_heatmapData.mode, filePath);
            info.filePathList.append(filePath);
        }
    }
    //time
    if (setting.fileFormat & REPORT_AUTO_BACKUP_FORMAT_HEATMAP_TIME) {
        report.mainType = 1;
        for (int i = 0; i < MAX_REAL_CAMERA; ++i) {
            if (setting.triChannels[i] != '1') {
                continue;
            }
            if (!LiveView::instance()->isChannelConnected(i)) {
                continue;
            }
            mutexSendMessage(REQUEST_FLAG_GET_IPC_HEATMAP_SUPPORT, i);
            int result = m_heatmapSupport;

            if (result == HEATMAP_SUPPORT_NO) {
                continue;
            }
            report.chnid = i;
            m_mutex.lock();
            sendMessage(REQUEST_FLAG_SEARCH_HEAT_MAP_REPORT, &report, sizeof(ms_heat_map_report));
            m_wait.wait(&m_mutex);
            QString text = m_heatmapData.text;
            m_mutex.unlock();
            qMsDebug() << "channel:" << i << "\n";
            if (text.isEmpty()) {
                qMsWarning() << "text is empty";
                continue;
            }
            QString filePath = QString("/tmp/heat_map/NVR_Heat_Map_CH%1_%2.csv").arg(i + 1, 2, 10, QLatin1Char('0')).arg(info.backupDateTime.toString("yyyyMMddHHmmss"));
            info.packageName = QString("NVR_Heat_Map_%1.tar").arg(info.backupDateTime.toString("yyyyMMddHHmmss"));
            info.packagePath = QString("/tmp/heat_map/%1").arg(info.packageName);
            makeTimeHeatMap(text, setting.timeRange, info.mailStartDateTime, filePath);
            info.filePathList.append(filePath);
        }
    }
}

void PeopleCountingAutoBackup::makeSpaceHeatMap(const QString &text, int corridorMode, int imageRotation, int mode, QString &filePath)
{
    QImage alphaImage;
    QImage heatMapImage;
    QImage backgroundImage = m_heatmapData.backgroundImage;

    int max = 0;
    int min = 0;
    int width = 192;
    int height = 192;
    //QRegExp rx(R"("max":(\d+),"min":(\d+),"map_w":(\d+),"map_h":(\d+),"data":\[(.*)\])");
    QRegExp rx(R"tt((.*),"data":\[(.*)\])tt");
    if (rx.indexIn(text) != -1) {
        QString strParameters = rx.cap(1);

        QRegExp rxMaxMin(R"("max":(\d+),"min":(\d+))");
        if (rxMaxMin.indexIn(strParameters) != -1) {
            QString strMax = rxMaxMin.cap(1);
            QString strMin = rxMaxMin.cap(2);
            max = strMax.toInt();
            min = strMin.toInt();
        }
        QRegExp rxWidthHeight(R"("map_w":(\d+),"map_h":(\d+))");
        if (rxWidthHeight.indexIn(strParameters) != -1) {
            QString strWidth = rxWidthHeight.cap(1);
            QString strHeight = rxWidthHeight.cap(2);
            width = strWidth.toInt();
            height = strHeight.toInt();
        }

        alphaImage = QImage(width, height, QImage::Format_ARGB32);
        alphaImage.fill(Qt::transparent);
        heatMapImage = QImage(width, height, QImage::Format_ARGB32);
        heatMapImage.fill(Qt::transparent);

        QString strData = rx.cap(2);
        QStringList dataList = strData.split("},{");
        qDebug() << QString("HeatMapThread::onMakeHeatMap, max: %1, min: %2, width: %3, height: %4, size: %5").arg(max).arg(min).arg(width).arg(height).arg(dataList.size());
        QRegExp rx2(R"("x":(\d+),"y":(\d+),"value":(\d+))");

        qreal radius = 1.9;

        QPainter alphaPainter(&alphaImage);
        alphaPainter.setRenderHint(QPainter::Antialiasing);
        for (int i = 0; i < dataList.size(); ++i) {
            QString temp = dataList.at(i);
            if (rx2.indexIn(temp) != -1) {
                int x = rx2.cap(1).toInt();
                int y = rx2.cap(2).toInt();
                int value = rx2.cap(3).toInt();

                int alpha = (qreal)(value - min) / (max - min) * 255;

                QRadialGradient gradient(x, y, radius);
                gradient.setColorAt(0, QColor(0, 0, 0, alpha));
                gradient.setColorAt(1, QColor(0, 0, 0, 0));

                alphaPainter.setPen(Qt::NoPen);
                alphaPainter.setBrush(gradient);
                alphaPainter.drawEllipse(QPointF(x, y), radius, radius);
            }
        }

        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < height; ++y) {
                int alpha = qAlpha(alphaImage.pixel(x, y));
                if (alpha == 0) {
                    continue;
                }
                int finalAlpha = alpha;
                if (finalAlpha > 128) {
                    finalAlpha = 128;
                }
                QColor color = m_colorImage.pixel(alpha - 1, 15);
                color.setAlpha(finalAlpha);
                heatMapImage.setPixel(x, y, color.rgba());
            }
        }

    } else {
        qWarning() << "error heatmap data:" << text;
    }
    //
    m_mutex.lock();
    QPainter painter(&backgroundImage);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    QRect heatRect;
    switch (mode) {
    case ModeFisheye:
        heatRect.setWidth(qMin(backgroundImage.width(), backgroundImage.height()));
        heatRect.setHeight(qMin(backgroundImage.width(), backgroundImage.height()));
        heatRect.moveCenter(backgroundImage.rect().center());
        break;
    case ModePanoramicMiniBullet:
        heatRect = backgroundImage.rect();
        break;
    default:
        break;
    }
    painter.save();
    switch (corridorMode) {
    case 1: {
        //顺时针旋转90度
        QMatrix matrix;
        matrix.rotate(90);
        heatMapImage = heatMapImage.transformed(matrix);
        break;
    }
    case 2: {
        //逆时针旋转90度
        QMatrix matrix;
        matrix.rotate(270);
        heatMapImage = heatMapImage.transformed(matrix);
        break;
    }
    default:
        break;
    }
    switch (imageRotation) {
    case 1: {
        //180度旋转
        QMatrix matrix;
        matrix.rotate(180);
        heatMapImage = heatMapImage.transformed(matrix);
        break;
    }
    case 2: {
        //水平翻转
        heatMapImage = heatMapImage.mirrored(true, false);
        break;
    }
    case 3: {
        //垂直翻转
        heatMapImage = heatMapImage.mirrored(false, true);
        break;
    }
    default:
        break;
    }
    painter.drawImage(heatRect, heatMapImage);
    painter.restore();

    m_mutex.unlock();

    backgroundImage.save(filePath, "PNG");
    gMsMessage.syncFile();
}

void PeopleCountingAutoBackup::makeTimeHeatMap(const QString &text, int reportType, const QDateTime &dateTime, QString &filePath)
{
    QList<int> valuesList;
    QRegExp rx(R"("max":(\d+),"min":(\d+),"data":\[(.*)\])");
    if (rx.indexIn(text) != -1) {
        QString strData = rx.cap(3);
        QStringList strDataList = strData.split(",");
        for (int i = 0; i < strDataList.size(); ++i) {
            valuesList.append(strDataList.at(i).toInt());
        }
    }
    QFile file(filePath);
    if (!file.open(QFile::WriteOnly)) {
        qMsWarning() << file.errorString();
        return;
    }
    QDateTime startDateTime = dateTime;
    startDateTime.setTime(QTime(dateTime.time().hour(), 0, 0));
    QDateTime endDateTime;
    QTextStream ts(&file);
    ts << QString("StartTime,EndTime,Value(s)\r\n");
    for (int i = 0; i < valuesList.size(); ++i) {
        switch (reportType) {
        case REPORT_AUTO_BACKUP_TIME_RANGE_LAST_DAY:
            //2020-03-12 02:00:00,2020-03-12 02:59:59,0
            if (i > 0) {
                startDateTime = startDateTime.addSecs(3600);
            }
            endDateTime = startDateTime;
            endDateTime.setTime(QTime(startDateTime.time().hour(), 59, 59));
            break;
        case REPORT_AUTO_BACKUP_TIME_RANGE_ALL:
        case REPORT_AUTO_BACKUP_TIME_RANGE_LAST_WEEK:
            //2020-02-01 00:00:00,2020-02-01 23:59:59,0
            if (i > 0) {
                startDateTime = startDateTime.addDays(1);
            }
            endDateTime = startDateTime.addDays(1);
            endDateTime = endDateTime.addSecs(-60);
            break;
        }
        ts << QString("%1,%2,%3\r\n").arg(startDateTime.toString("yyyy-MM-dd HH:mm:ss")).arg(endDateTime.toString("yyyy-MM-dd HH:mm:ss")).arg(valuesList.at(i));
    }
    file.close();
    gMsMessage.syncFile();
}

void PeopleCountingAutoBackup::mutexSendMessage(int type, int channel)
{
    Q_UNUSED(type)
    Q_UNUSED(channel)
}

void PeopleCountingAutoBackup::initializeColorImage()
{
    m_colorImage = QImage(255, 30, QImage::Format_ARGB32);

    QPainter painter(&m_colorImage);
    painter.setPen(Qt::NoPen);
    QLinearGradient linear(0, 0, 255, 0);
    linear.setColorAt(0.25, Qt::blue);
    linear.setColorAt(0.55, Qt::green);
    linear.setColorAt(0.85, Qt::yellow);
    linear.setColorAt(1, Qt::red);
    painter.setBrush(linear);
    painter.drawRect(m_colorImage.rect());
}

QStringList PeopleCountingAutoBackup::calculateTotal(const QList<QVariant> &list, const QString &key)
{
    QStringList itemList;
    QMap<QString, int> dataMap;
    for (int var = 0; var < 4; ++var) {
        auto itemMap = list[var].toMap();
        QString strCount = itemMap.value(key).toString();
        QStringList strCountList = strCount.split(",");

        QString stratTime = itemMap.value("starttime").toString();
        QStringList starttimeList = stratTime.split(",", QString::SkipEmptyParts);
        //export all
        if (starttimeList.count() > 1 || starttimeList.count() == strCountList.count()) {
            for (int i = 0; i < starttimeList.count(); ++i) {
                if (dataMap.find(starttimeList.at(i)) != dataMap.end()) {
                    dataMap[starttimeList.at(i)] += strCountList.at(i).toInt();
                } else {
                    dataMap.insert(starttimeList.at(i), strCountList.at(i).toInt());
                }
            }
        } else {
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
    }
    if (!dataMap.empty()) {
        for (auto iter : dataMap) {
            itemList.append(QString("%1").arg(iter));
        }
    }
    return itemList;
}

void PeopleCountingAutoBackup::calculateTime(const QList<QVariant> &list, QString &startTime, QString &endTime)
{
    QMap<QString, QString> dataMap;
    for (int var = 0; var < 4; ++var) {
        auto itemMap = list[var].toMap();
        QString strat = itemMap.value("starttime").toString();
        QStringList starttimeList = strat.split(",", QString::SkipEmptyParts);

        QString end = itemMap.value("endtime").toString();
        QStringList endtimeList = end.split(",", QString::SkipEmptyParts);
        for (int i = 0; i < starttimeList.count(); ++i) {
            if (dataMap.find(starttimeList.at(i)) == dataMap.end()) {
                dataMap.insert(starttimeList.at(i), endtimeList.at(i));
            }
        }
    }
    for (auto iter = dataMap.begin(); iter != dataMap.end(); iter++) {
        startTime += iter.key() + ",";
        endTime += iter.value() + ",";
    }
    startTime = startTime.left(startTime.size() - 1);
    endTime = endTime.left(endTime.size() - 1);
}

void PeopleCountingAutoBackup::onSearchByGroupFinished(RunnableSearchPeopleCountingByGroup::ResultInfo info)
{
    m_mutex.lock();
    m_groupSearchInfo = info;
    m_wait.wakeAll();
    m_mutex.unlock();
}

void PeopleCountingAutoBackup::onThreadStarted()
{
}

void PeopleCountingAutoBackup::onThreadFinished()
{
}

void PeopleCountingAutoBackup::startBackupPeopleCount()
{
    qMsDebug() << "begin";
    m_mutex.lock();
    if (m_taskList.isEmpty()) {
        m_isWorking = false;
        m_mutex.unlock();
        qMsDebug() << "return 1";
        return;
    }
    REPORT_AUTO_BACKUP_SETTING_S task = m_taskList.takeFirst();
    m_mutex.unlock();

    BackupInfo info;

    for (int i = 0; i < MAX_REAL_CAMERA; ++i) {
        if (task.triChannels[i] == '1') {
            info.channelCount++;
        }
    }

    switch (task.reportType) {
    case REPORT_AUTO_BACKUP_PEOPLECNT_BY_GROUP:
        makeGroupData(task, info);
        break;
    case REPORT_AUTO_BACKUP_PEOPLECNT_BY_CAMERA:
        makeCameraData(task, info);
        break;
    case REPORT_AUTO_BACKUP_PEOPLECNT_BY_REGION:
        makeRegionData(task, info);
        break;
    case REPORT_AUTO_BACKUP_HEATMAP:
        makeHeatMapData(task, info);
        break;
    default:
        break;
    }
    if (info.error != 0) {
        qMsCritical() << "info.error:" << info.error;
    } else if (info.filePathList.size() < 1) {
        qMsCritical() << "info.filePathList.size:" << info.filePathList.size();
    } else {

    }

    //
    m_mutex.lock();
    if (!m_taskList.isEmpty()) {
        QMetaObject::invokeMethod(this, "startBackupPeopleCount", Qt::QueuedConnection);
    } else {
        m_isWorking = false;
    }
    m_mutex.unlock();
    qMsDebug() << "end";
}
