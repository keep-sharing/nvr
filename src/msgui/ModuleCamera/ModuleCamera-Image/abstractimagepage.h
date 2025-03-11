#ifndef ABSTRACTIMAGEPAGE_H
#define ABSTRACTIMAGEPAGE_H

#include "MsWidget.h"
#include <QEventLoop>
#include <QGraphicsItem>

class ImageSetting;
class MessageReceive;

class AbstractImagePage : public MsWidget {
    Q_OBJECT
public:
    explicit AbstractImagePage(QWidget *parent = nullptr);

    void setDrawWidget(QWidget *widget);
    void setDrawItem(QGraphicsItem *item);

    void back();

    void setChannelConnected(bool connected);
    bool isChannelConnected();
    int getImageSdkVersion(const char *sdkVersion, int len);
    int compareSdkVersions(const char *sdkVersion, const char *dstSdkVersion);
    void updatePlayVideo(int channel);

    virtual void initializeData(int channel) = 0;

signals:
    void sig_back();

protected:
    QEventLoop m_eventLoop;

private:
    ImageSetting *m_imageSetting = nullptr;

    bool m_isConnected = false;
};

#endif // ABSTRACTIMAGEPAGE_H
