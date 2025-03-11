#include "PosDisplayData.h"
#include "MsLanguage.h"

PosDisplayData::PosDisplayData(QObject *parent)
    : QObject(parent)
{
    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

PosDisplayData &PosDisplayData::instance()
{
    static PosDisplayData self;
    return self;
}

const QList<MsPos::ColorInfo> &PosDisplayData::colorList() const
{
    return m_colorList;
}

void PosDisplayData::setColorList(const QList<MsPos::ColorInfo> &newColorList)
{
    m_colorList = newColorList;
}

QColor PosDisplayData::colorFromIndex(int index)
{
    return m_colorList.at(index).color;
}

int PosDisplayData::fontSizeFromIndex(int index)
{
    switch (index) {
    case 0:
        return 15;
    case 1:
        return 25;
    case 2:
        return 40;
    default:
        return 25;
    }
}

QString PosDisplayData::protocolName(int protocol)
{
    QString name;
    switch (protocol) {
    case 0:
        name = "General";
        break;
    default:
        name = "Unknow";
        break;
    }
    return name;
}

int PosDisplayData::protocolValue(const QString &name)
{
    int value = 0;
    if (name == "General") {
        value = 0;
    }
    return value;
}

QString PosDisplayData::characterEncodingName(int encoding)
{
    QString name;
    switch (encoding) {
    case 0:
        name = "Unicode(UTF-8)";
        break;
    default:
        break;
    }
    return name;
}

int PosDisplayData::characterEncodingValue(const QString &name)
{
    int value = 0;
    if (name == "Unicode(UTF-8)") {
        value = 0;
    }
    return value;
}

void PosDisplayData::onLanguageChanged()
{
    m_colorList.clear();
    m_colorList.append(MsPos::ColorInfo(0, GET_TEXT("LIVEVIEW/20200", "White"), QColor(255, 255, 255)));
    m_colorList.append(MsPos::ColorInfo(1, GET_TEXT("LIVEVIEW/20201", "Red"), QColor(250, 37, 67)));
    m_colorList.append(MsPos::ColorInfo(2, GET_TEXT("LIVEVIEW/20202", "Pink"), QColor(206, 60, 159)));
    m_colorList.append(MsPos::ColorInfo(3, GET_TEXT("LIVEVIEW/20203", "Purple"), QColor(126, 41, 204)));
    m_colorList.append(MsPos::ColorInfo(4, GET_TEXT("LIVEVIEW/20204", "Dark Blue"), QColor(57, 76, 217)));
    m_colorList.append(MsPos::ColorInfo(5, GET_TEXT("LIVEVIEW/20205", "Cyan"), QColor(58, 226, 255)));
    m_colorList.append(MsPos::ColorInfo(6, GET_TEXT("LIVEVIEW/20206", "Dark Cyan"), QColor(48, 198, 171)));
    m_colorList.append(MsPos::ColorInfo(7, GET_TEXT("LIVEVIEW/20207", "Dark Green"), QColor(108, 175, 59)));
    m_colorList.append(MsPos::ColorInfo(8, GET_TEXT("LIVEVIEW/20208", "Yellow"), QColor(248, 210, 83)));
}
