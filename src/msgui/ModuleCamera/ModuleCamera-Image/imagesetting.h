#ifndef IMAGESETTING_H
#define IMAGESETTING_H

#include "abstractcamerapage.h"
#include "abstractimagepage.h"
#include <QGraphicsItem>
#include <QMap>
#include <QWidget>

namespace Ui {
class ImageSetting;
}

class ImageSetting : public AbstractCameraPage {
    Q_OBJECT

    enum ImageItem {
        ItemDisplay,
        ItemEnhancement,
        ItemDayNight,
        ItemOsd,
        ItemPrivacyMask,
        ItemRoi,
        ItemNone
    };

public:
    explicit ImageSetting(QWidget *parent = nullptr);
    ~ImageSetting();

    void setDrawWidget(QWidget *widget);
    void setDrawItem(QGraphicsItem *item);

    int getImageSdkVersion(const char *sdkVersion, int len);

    bool isChannelConnected(int channel);

    virtual void initializeData() override;
    virtual void dealMessage(MessageReceive *message) override;

    void updatePlayVideo(int channel);
    int compareSdkVersions(const char *sdkVersion, const char *dstSdkVersion);

private slots:
    void onTabClicked(int index);
    void onChannelClicked(int channel);

private:
    Ui::ImageSetting *ui;

    int m_currentChannel = -1;

    ImageItem m_currentItem = ItemNone;
    QMap<int, AbstractImagePage *> m_itemMap;
    int m_pageIndex;
    int m_pageIndexOld;
};

#endif // IMAGESETTING_H
