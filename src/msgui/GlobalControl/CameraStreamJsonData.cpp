#include "CameraStreamJsonData.h"
#include "qjson/include/parser.h"
#include "MyDebug.h"
CameraStreamJsonData::CameraStreamJsonData()
{
}

CameraStreamJsonData::CameraStreamJsonData(QByteArray jsonData)
{
    parseJson(jsonData);
}

CameraStreamJsonData::~CameraStreamJsonData()
{

}

void CameraStreamJsonData::parseJson(const QByteArray &json)
{
    m_streamsMap.clear();

    QJson::Parser parser;
    bool ok;
    QVariant result = parser.parse(json, &ok);
    if (!ok) {
        return;
    }
    QVariantMap rootMap = result.toMap();
    QVariantMap stream0Map = rootMap.value("stream_0").toMap();
    parseStream(stream0Map, m_streamsMap["stream_0"]);
    QVariantMap stream1Map = rootMap.value("stream_1").toMap();
    parseStream(stream1Map, m_streamsMap["stream_1"]);
    QVariantMap stream2Map = rootMap.value("stream_2").toMap();
    parseStream(stream2Map, m_streamsMap["stream_2"]);

    QVariantMap stream3Map = rootMap.value("stream_3").toMap();
    if (!stream3Map.isEmpty()) {
        parseStream(stream2Map, m_streamsMap["stream_3"]);
    }
    
    QVariantMap stream4Map = rootMap.value("stream_4").toMap();
    if (!stream4Map.isEmpty()) {
        parseStream(stream2Map, m_streamsMap["stream_4"]);
    }

    //
    m_mainEffectsMap.clear();
    QVariantMap mainEffectMap = rootMap.value("main_to_else").toMap();
    QVariantList mainEffectList = mainEffectMap.value("resolution").toList();
    for (int i = 0; i < mainEffectList.size(); ++i) {
        QVariantMap obj = mainEffectList.at(i).toMap();
        int width = obj.value("width").toInt();
        int height = obj.value("height").toInt();
        int rate = obj.value("frame_rate").toInt();
        QString key = QString("%1*%2").arg(width).arg(height);
        m_mainEffectsMap.insert(key, rate);
    }
}

void CameraStreamJsonData::parseStream(const QVariantMap &map, CameraStreamJsonData::StreamInfo &stream)
{
    QVariantMap h264Map = map.value("h264").toMap();
    parseVideo(h264Map, stream.videosMap["h264"]);
    QVariantMap h265Map = map.value("h265").toMap();
    parseVideo(h265Map, stream.videosMap["h265"]);
    QVariantMap mjpegMap = map.value("mjpeg").toMap();
    parseVideo(mjpegMap, stream.videosMap["mjpeg"]);
}

void CameraStreamJsonData::parseVideo(const QVariantMap &map, CameraStreamJsonData::VideoInfo &video)
{
    QVariantList list = map.value("resolution").toList();
    for (int i = 0; i < list.size(); ++i) {
        QVariantMap obj = list.at(i).toMap();
        int width = obj.value("width").toInt();
        int height = obj.value("height").toInt();
        int rate = obj.value("frame_rate").toInt();
        QString key = QString("%1*%2").arg(width).arg(height);
        video.framesMap.insert(key, rate);
        video.framesSizeList.push_back(key);
    }
}

CameraStreamJsonData::StreamInfo CameraStreamJsonData::steamMapValue(const QString &key) const
{
    return m_streamsMap.value(key);
}

bool CameraStreamJsonData::mainEffectsMapIsContains(const QString &key)
{
    return  m_mainEffectsMap.contains(key);
}

int CameraStreamJsonData::mainEffectsMapValue(const QString &key) const
{
    return  m_mainEffectsMap.value(key);
}
