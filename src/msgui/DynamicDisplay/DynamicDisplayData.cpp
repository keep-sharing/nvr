#include "DynamicDisplayData.h"
#include "LiveView.h"
#include "MyDebug.h"

QMutex DynamicDisplayData::s_mutex;
VacDynamicBoxALL gVcaInfo;
RegionalRectInfo gRegionalRectInfo;
QMap<int, RegionalAlarmInfo> gRegionalAlarmInfoMap;
QMap<int, MS_PEOPLECNT_DATA> gPeopleCntInfoMap;

DynamicDisplayData::DynamicDisplayData(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<PosData>("PosData");

    memset(&gVcaInfo, 0, sizeof(VacDynamicBoxALL));
    memset(&gRegionalRectInfo, 0, sizeof(RegionalRectInfo));
}

DynamicDisplayData::~DynamicDisplayData()
{

}

DynamicDisplayData &DynamicDisplayData::instance()
{
    static DynamicDisplayData self;
    return self;
}

void DynamicDisplayData::readyToQuit()
{
}

void DynamicDisplayData::updateVcaData(int channel)
{
    Q_UNUSED(channel)
}

VacDynamicBoxALL *DynamicDisplayData::vcaRectData()
{
    QMutexLocker locker(&s_mutex);
    memcpy(&m_vcaInfo, &gVcaInfo, sizeof(VacDynamicBoxALL));
    return &m_vcaInfo;
}

void DynamicDisplayData::updateRegionRectData(int channel)
{
    Q_UNUSED(channel)
}

RegionalRectInfo *DynamicDisplayData::regionRectData()
{
    QMutexLocker locker(&s_mutex);
    memcpy(&m_regionRectInfo, &gRegionalRectInfo, sizeof(RegionalRectInfo));
    return &m_regionRectInfo;
}

void DynamicDisplayData::updateRegionAlarmData(int channel)
{
    Q_UNUSED(channel)
}

RegionalAlarmInfo DynamicDisplayData::regionAlarmData(int channel)
{
    QMutexLocker locker(&s_mutex);
    RegionalAlarmInfo info;
    if (gRegionalAlarmInfoMap.contains(channel)) {
        info = gRegionalAlarmInfoMap.value(channel);
    } else {
        info.chnid = -1;
    }
    return info;
}

int DynamicDisplayData::regionStayValue(int channel, int index)
{
    QMutexLocker locker(&s_mutex);
    if (gRegionalAlarmInfoMap.contains(channel)) {
        return gRegionalAlarmInfoMap.value(channel).num[index];
    } else {
        return 0;
    }
}

void DynamicDisplayData::updatePosData(const PosData &data)
{
    emit posDataReceived(data);
}

void DynamicDisplayData::updatePeopleCntData(int channel)
{
    Q_UNUSED(channel)
}

MS_PEOPLECNT_DATA DynamicDisplayData::peopleCntData(int channel)
{
    QMutexLocker locker(&s_mutex);
    MS_PEOPLECNT_DATA info;
    if (gPeopleCntInfoMap.contains(channel)) {
        info = gPeopleCntInfoMap.value(channel);
    } else {
        memset(&info, 0, sizeof(MS_PEOPLECNT_DATA));
        info.chnId = -1;
    }
    return info;
}
