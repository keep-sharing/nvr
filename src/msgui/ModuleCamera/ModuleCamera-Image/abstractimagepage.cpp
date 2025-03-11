#include "abstractimagepage.h"
#include "imagesetting.h"

AbstractImagePage::AbstractImagePage(QWidget *parent)
    : MsWidget(parent)
{
    m_imageSetting = static_cast<ImageSetting *>(parent);
}

void AbstractImagePage::setDrawWidget(QWidget *widget)
{
    if (m_imageSetting) {
        m_imageSetting->setDrawWidget(widget);
    }
}

void AbstractImagePage::setDrawItem(QGraphicsItem *item)
{
    if (m_imageSetting) {
        m_imageSetting->setDrawItem(item);
    }
}

void AbstractImagePage::back()
{
    if (m_imageSetting) {
        m_imageSetting->back();
    }
}

void AbstractImagePage::setChannelConnected(bool connected)
{
    m_isConnected = connected;
}

bool AbstractImagePage::isChannelConnected()
{
    return m_isConnected;
}

int AbstractImagePage::getImageSdkVersion(const char *sdkVersion, int len)
{
    if (m_imageSetting) {
        return m_imageSetting->getImageSdkVersion(sdkVersion, len);
    }

    return 0;
}

void AbstractImagePage::updatePlayVideo(int channel)
{
    if (m_imageSetting) {
        m_imageSetting->updatePlayVideo(channel);
    }
}


int AbstractImagePage::compareSdkVersions(const char *sdkVersion, const char *dstSdkVersion)
{
    if (m_imageSetting) {
        return m_imageSetting->compareSdkVersions(sdkVersion, dstSdkVersion);
    }
    return 0;
}