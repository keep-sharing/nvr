#ifndef HEATMAPTHREAD_H
#define HEATMAPTHREAD_H

#include <QObject>
#include <QThread>
#include <QImage>
#include <QMutex>


enum HeatMapMode
{
    ModeFisheye,            //鱼眼
    ModePanoramicMiniBullet //全景筒
};

struct DrawHeatMapData {
    //from ON_RESPONSE_FLAG_SEARCH_HEAT_MAP_REPORT
    QString text;
    //from ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY
    int corridorMode = 0;
    int imageRotation = 0;
    int lencorrect = 0;
    //from isSupport
    HeatMapMode mode;
    //from ON_RESPONSE_FLAG_GET_IPC_SNAPHOST
    QImage backgroundImage;
};

class HeatMapThread : public QObject
{
    Q_OBJECT
public:
    explicit HeatMapThread(QObject *parent = nullptr);
    ~HeatMapThread();

    void stop();

    void setBackgroundImage(const QImage &image);
    void makeHeatMap(const DrawHeatMapData &drawHeatMapData);
    void saveSpaceHeatMap(const QString &filePath);

private:
    void initializeColorImage();

signals:
    void imageFinished(int max, int min, QImage colorImage, QImage heatmapImage);
    void saveFinished(bool result);

private slots:
    void onThreadStarted();

    void onMakeHeatMap(const QString &text, int corridorMode, int imageRotation, int mode);
    void onSaveSpaceHeatMap(QString filePath);

private:
    QThread m_thread;
    QMutex m_mutex;

    QImage m_backgroundImage;
    QImage m_colorImage;
};

#endif // HEATMAPTHREAD_H
