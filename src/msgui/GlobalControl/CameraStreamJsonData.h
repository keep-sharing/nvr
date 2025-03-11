#ifndef CAMERASTREAMJSONDATA_H
#define CAMERASTREAMJSONDATA_H

#include <QMetaType>
#include <QVariantMap>

class CameraStreamJsonData {
public:
    struct VideoInfo {
        //key: 1920*1080
        QMap<QString, int> framesMap;
        QList<QString> framesSizeList;
    };

    struct StreamInfo {
        //key: h264, h265, mjpeg
        QMap<QString, VideoInfo> videosMap;
    };
    explicit CameraStreamJsonData();
    explicit CameraStreamJsonData(QByteArray jsonData);
    ~ CameraStreamJsonData();

    void parseJson(const QByteArray &json);
    void parseStream(const QVariantMap &map, StreamInfo &stream);
    void parseVideo(const QVariantMap &map, VideoInfo &video);

    StreamInfo steamMapValue(const QString &key) const;
    bool mainEffectsMapIsContains(const QString &key);
    int mainEffectsMapValue(const QString &key) const;

private:
    //key: stream_0, stream_1, stream_2
    QMap<QString, StreamInfo> m_streamsMap;
    //key: 1920*1080, 主码流分辨率影响次码流帧率
    QMap<QString, int> m_mainEffectsMap;
};

#endif // CAMERASTREAMJSONDATA_H
