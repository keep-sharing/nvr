#include "imagesetting.h"
#include "ui_imagesetting.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "imagepagedaynight.h"
#include "imagepagedisplay.h"
#include "imagepageenhancement.h"
#include "imagepagemask.h"
#include "imagepageosd.h"
#include "imagepageroi.h"
#include "ImagePageDisplayMulti.h"
#include "ImagePageEnhancementMulti.h"
#include "ImagePageDayNightMulti.h"

ImageSetting::ImageSetting(QWidget *parent)
    : AbstractCameraPage(parent)
    , ui(new Ui::ImageSetting)
{
    ui->setupUi(this);

    ui->tabBar->addTab(GET_TEXT("IMAGE/37001", "Display"));
    ui->tabBar->addTab(GET_TEXT("IMAGE/37002", "Enhancement"));
    ui->tabBar->addTab(GET_TEXT("IMAGE/37003", "Day/Night Settings"));
    ui->tabBar->addTab(GET_TEXT("IMAGE/37004", "OSD"));
    ui->tabBar->addTab(GET_TEXT("IMAGE/37005", "Privacy Mask"));
    ui->tabBar->addTab(GET_TEXT("IMAGE/37006", "ROI"));
    connect(ui->tabBar, SIGNAL(tabClicked(int)), this, SLOT(onTabClicked(int)));

    ui->buttonGroup_channel->setCount(qMsNvr->maxChannel());
    connect(ui->buttonGroup_channel, SIGNAL(buttonClicked(int)), this, SLOT(onChannelClicked(int)));
}

ImageSetting::~ImageSetting()
{
    delete ui;
}

void ImageSetting::setDrawWidget(QWidget *widget)
{
    ui->commonVideo->showDrawWidget(widget);
}

void ImageSetting::setDrawItem(QGraphicsItem *item)
{
    ui->commonVideo->addGraphicsItem(item);
}

bool ImageSetting::isChannelConnected(int channel)
{
    return LiveView::instance()->isChannelConnected(channel);
}

void ImageSetting::initializeData()
{
    ui->buttonGroup_channel->editCurrentIndex(0);
    m_currentChannel = 0;
    ui->commonVideo->playVideo(m_currentChannel);

    ui->tabBar->setCurrentTab(0);
}

void ImageSetting::dealMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void ImageSetting::updatePlayVideo(int channel)
{
    if (channel != m_currentChannel) {
        return;
    }
    ui->commonVideo->playVideo(channel);
}

void ImageSetting::onTabClicked(int index)
{
    m_currentItem = static_cast<ImageItem>(index);
    if ( (m_currentItem == ItemDisplay || m_currentItem == ItemEnhancement || m_currentItem == ItemDayNight)) {
        m_pageIndex = m_currentItem + ui->tabBar->tabCount();
    } else {
        m_pageIndex = m_currentItem;
    }
    AbstractImagePage *page = m_itemMap.value(m_pageIndex, nullptr);
    if (!page) {
        switch (m_currentItem) {
        case ItemDisplay:
            
                page = new ImagePageDisplay(this);
            
            break;
        case ItemEnhancement:
            
                page = new ImagePageEnhancement(this);
            
            break;
        case ItemDayNight:
           
                page = new ImagePageDayNight(this);
            
            break;
        case ItemOsd:
            page = new ImagePageOsd(this);
            break;
        case ItemPrivacyMask:
            page = new ImagePageMask(this);
            break;
        case ItemRoi:
            page = new ImagePageRoi(this);
            break;
        default:
            break;
        }
        if (page) {
            m_itemMap.insert(m_pageIndex, page);
        }
    }
    //
    QLayoutItem *item = ui->gridLayout->itemAtPosition(0, 0);
    if (item) {
        QWidget *widget = item->widget();
        if (widget) {
            widget->hide();
        }
        ui->gridLayout->removeItem(item);
        delete item;
    }
    //
    setDrawWidget(nullptr);
    if (page) {
        ui->gridLayout->addWidget(page, 0, 0);
        page->show();
        bool connected = isChannelConnected(m_currentChannel);
        page->setChannelConnected(connected);
        page->initializeData(m_currentChannel);
    }
}

void ImageSetting::onChannelClicked(int channel)
{
    m_currentChannel = channel;
    ui->commonVideo->playVideo(channel);

    //
    bool connected = isChannelConnected(channel);
    if ( (m_currentItem == ItemDisplay || m_currentItem == ItemEnhancement || m_currentItem == ItemDayNight)) {
        m_pageIndex = m_currentItem + ui->tabBar->tabCount();
    }  else {
        m_pageIndex = m_currentItem;
    }
    msprintf("gsjt m_pageIndex:%d",m_pageIndex);
    if (m_itemMap.contains(m_pageIndex)) {
        AbstractImagePage *page = m_itemMap.value(m_pageIndex);
        page->setChannelConnected(connected);
        page->initializeData(m_currentChannel);
        if (m_pageIndex != m_pageIndexOld) {
            QLayoutItem *item = ui->gridLayout->itemAtPosition(0, 0);
            if (item) {
                QWidget *widget = item->widget();
                if (widget) {
                    widget->hide();
                }
                ui->gridLayout->removeItem(item);
                delete item;
            }
            setDrawWidget(nullptr);
            ui->gridLayout->addWidget(page, 0, 0);
            page->show();
        }

    } else {
        onTabClicked(m_currentItem);
    }
    m_pageIndexOld = m_pageIndex;
}

int ImageSetting::getImageSdkVersion(const char *sdkVersion, int len)
{
    int version = 0;
    char tmp[32];
    int i = 0, j = 0;

    for (i = 0, j = 0; i < len; i++) {
        if (sdkVersion[i] != '.')
            tmp[j++] = sdkVersion[i];
    }
    tmp[j] = '\0';
    qDebug() << "sdk version:" << sdkVersion << " " << atoi(tmp);
    version = atoi(tmp);

    return version;
}

int ImageSetting::compareSdkVersions(const char *sdkVersion, const char *dstSdkVersion)
{
    char *sdkVersion1 = new char[strlen(sdkVersion) + 1];
    char *sdkVersion2 = new char[strlen(dstSdkVersion) + 1];
    char *token1, *token2;
    int num1, num2;

    strcpy(sdkVersion1, sdkVersion);
    strcpy(sdkVersion2, dstSdkVersion);
    token1 = strtok(sdkVersion1, ".");
    token2 = strtok(sdkVersion2, ".");

    while (token1 != NULL && token2 != NULL) {
        num1 = atoi(token1);
        num2 = atoi(token2);

        if (num1 < num2) {
            return -1;
        } else if (num1 > num2) {
            return 1;
        }

        // 继续比较下一个子字符串
        token1 = strtok(NULL, ".");
        token2 = strtok(NULL, ".");
    }

    if (token1 == NULL && token2 != NULL) {
        return -1;
    } else if (token1 != NULL && token2 == NULL) {
        return 1;
    }

    return 0;
}