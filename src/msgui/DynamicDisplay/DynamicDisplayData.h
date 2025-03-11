#ifndef DYNAMICDISPLAYDATA_H
#define DYNAMICDISPLAYDATA_H

/*******************************************
vca画框简介：同一个人在不同event中表现出的alarm会有所不同，所以会发送一个人在所有event里的画框数据。
其中event，alarm，class中的每一个bit位，均代表一种vca事件。即识别到一个人体框之后，要判断这个人体框属于哪些VCA事件，需要去解析对应bit位的值。(alarm，class同理)
以下定义为每一个bit所代表的VCA事件

typedef enum {

    VCA_REGION_ENTRANCE = 0,	// 1
    VCA_REGION_EXITING,			// 2
    VCA_ADVANCED_MOTION,		// 3
    VCA_TAMPER_DEFOCUS,			// 4
    VCA_LINE_CROSSING,			// 5
    VCA_OBJECT_LOITERING,		// 6
    VCA_HUMAN_DETECTION,		// 7
    VCA_PEOPLE_COUNTING,		// 8
    VCA_OBJECT_LEFT_REMOVED,	// 9
    VCA_EVENT_TYPE_MAX_NUM,

} VCA_EVENT_E;
********************************************/

#include <QObject>
#include <QMutex>
#include "PosData.h"

extern "C" {
#include "msg.h"
}

#define gDynamicDisplayData DynamicDisplayData::instance()

class DynamicDisplayData : public QObject
{
    Q_OBJECT
public:
    explicit DynamicDisplayData(QObject *parent = nullptr);
    ~DynamicDisplayData() override;

    static DynamicDisplayData &instance();

    void readyToQuit();

    //vca
    void updateVcaData(int channel);
    VacDynamicBoxALL *vcaRectData();

    //region rect
    void updateRegionRectData(int channel);
    RegionalRectInfo *regionRectData();

    //region alarm
    void updateRegionAlarmData(int channel);
    RegionalAlarmInfo regionAlarmData(int channel);
    int regionStayValue(int channel, int index);

    //pos
    void updatePosData(const PosData &data);

    //people cnt line
    void updatePeopleCntData(int channel);
    MS_PEOPLECNT_DATA peopleCntData(int channel);

signals:
    void dynamicDataChanged(int type, int channel);
    void posDataReceived(PosData data);

private:
    static QMutex s_mutex;

    VacDynamicBoxALL m_vcaInfo;
    RegionalRectInfo m_regionRectInfo;
    MS_PEOPLECNT_DATA m_peopleCntInfo;
};

#endif // DYNAMICDISPLAYDATA_H
