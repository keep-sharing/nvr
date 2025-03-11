#pragma once

#include <QObject>

class Watermark;
struct ms_water_mark;
class WatermarkPrivate : public QObject
{
    Q_OBJECT
public:
    WatermarkPrivate(QObject *parent);
    void resetUI();
    void initializeUI();
    void ON_RESPONSE_FLAG_GET_IPC_WATERMARK(void *);
    void ON_RESPONSE_FLAG_SET_IPC_WATERMARK(void *);
    bool isSupported();
    void setIsSupported(bool);
    void fetchData(ms_water_mark *);
signals:
    void isSupportedChanged();
public slots:
    void onLanguageChanged();
    void onIsSupportedChanged();
public:
    Watermark *q;
    bool m_isSupported = true;
};
