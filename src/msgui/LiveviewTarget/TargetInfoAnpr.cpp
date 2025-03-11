#include "TargetInfoAnpr.h"
#include "AnprBackupInfo.h"
#include "MsLanguage.h"
#include "MyDebug.h"

TargetInfoAnpr::TargetInfoAnpr(lpr_metadata_info *pMsg, lpr_image_info *pImage)
    : TargetInfo()
{
    m_type = TARGET_ANPR;
    memcpy(&m_info, pMsg, sizeof(lpr_metadata_info));
    m_smallImageData = QByteArray(pImage->pdata, pImage->size);
}

TargetInfoAnpr::~TargetInfoAnpr()
{
}

int TargetInfoAnpr::channel() const
{
    return m_info.chnid;
}

QString TargetInfoAnpr::timeString() const
{
    return QString(m_info.ptime);
}

QString TargetInfoAnpr::licenseString() const
{
    return QString(m_info.plate);
}

QString TargetInfoAnpr::plateTypeString() const
{
    return QString(m_info.ptype);
}

QString TargetInfoAnpr::plateColorString() const
{
    return AnprBackupInfo::plateColorString(m_info.color1);
}

QString TargetInfoAnpr::vehicleTypeString() const
{
    return AnprBackupInfo::vehicleTypeString(m_info.vehicleType);
}

QString TargetInfoAnpr::vehicleBrandString() const
{
    return AnprBackupInfo::vehicleBrandString(m_info.brand);
}

QString TargetInfoAnpr::vehicleColorString() const
{
    return AnprBackupInfo::plateColorString(m_info.color0);
}

QString TargetInfoAnpr::speedString() const
{
    if (m_info.speed < 0) {
        return GET_TEXT("TARGETMODE/103205", "N/A");
    } else {
        return QString("%1km/h").arg(m_info.speed);
    }
}

QString TargetInfoAnpr::directionString() const
{
    QString text;
    switch (m_info.direction) {
    case 0:
        text = GET_TEXT("TARGETMODE/103205", "N/A");
        break;
    case 1:
        text = GET_TEXT("ANPR/103055", "Approach");
        break;
    case 2:
        text = GET_TEXT("ANPR/103056", "Away");
        break;
    default:
        qMsWarning() << "error direction:" << m_info.direction;
        break;
    }
    return text;
}

void TargetInfoAnpr::setAnprTypeString(const QString &type)
{
    snprintf(m_info.ptype, sizeof(m_info.ptype), "%s", type.toStdString().c_str());
}
