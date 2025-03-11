#include "TargetInfoVca.h"
#include "MsLanguage.h"

TargetInfoVca::TargetInfoVca(MS_VCA_ALARM *alarm)
    : TargetInfo()
{
    m_type = TARGET_VCA;
    memcpy(&m_info, alarm, sizeof(MS_VCA_ALARM));
    m_smallImageData = QByteArray((char *)alarm->jpgdata, alarm->jpgsize);
}

TargetInfoVca::~TargetInfoVca()
{

}

int TargetInfoVca::channel() const
{
    return m_info.chnid;
}

QString TargetInfoVca::timeString() const
{
    return m_info.ptime;
}

QString TargetInfoVca::vcaEventString() const
{
    QString text;
    switch (m_info.event) {
    case VCA_REGIONIN:
        text = GET_TEXT("SMARTEVENT/55001", "Region Entrance");
        break;
    case VCA_REGIONOUT:
        text = GET_TEXT("SMARTEVENT/55002", "Region Exiting");
        break;
    case VCA_ADVANCED_MOTION:
        text = GET_TEXT("SMARTEVENT/55003", "Advanced Motion Detection");
        break;
    case VCA_TAMPER:
        text = GET_TEXT("SMARTEVENT/55004", "Tamper Detection");
        break;
    case VCA_LINECROSS:
        text = GET_TEXT("SMARTEVENT/55005", "Line Crossing");
        break;
    case VCA_LOITERING:
        text = GET_TEXT("SMARTEVENT/55006", "Loitering");
        break;
    case VCA_HUMAN:
        text = GET_TEXT("SMARTEVENT/55007", "Human Detection");
        break;
    case VCA_PEOPLECNT:
        text = GET_TEXT("SMARTEVENT/55008", "People Counting");
        break;
    case VCA_OBJECT_LEFT:
    case VCA_OBJECT_REMOVE:
        text = GET_TEXT("SMARTEVENT/55055", "Object Left/Removed");
        break;
    default:
        break;
    }
    return text;
}

QString TargetInfoVca::vcaObjectString() const
{
    QString text;
    switch (m_info.objtype) {
    case VCA_OBJ_NONE:
        text = GET_TEXT("TARGETMODE/103205", "N/A");
        break;
    case VCA_OBJ_HUMAN:
        text = GET_TEXT("TARGETMODE/103201", "Human");
        break;
    case VCA_OBJ_CAR:
        text = GET_TEXT("TARGETMODE/103202", "Vehicle");
        break;
    }
    return text;
}
