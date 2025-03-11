#include "PageAnprSettings.h"
#include "ui_PageAnprSettings.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "TabAnprBlackList.h"
#include "TabAnprListManagement.h"
#include "TabAnprSettings.h"
#include "TabAnprVisitor.h"
#include "TabAnprWhiteList.h"
#include "AnprAutoBackupSetting.h"
#include "centralmessage.h"
#include <QtDebug>

PageAnprSettings::PageAnprSettings(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::AnprSetting)
{
    ui->setupUi(this);

    ui->tabBar->addTab(GET_TEXT("SMARTEVENT/55009", "Settings"));
    ui->tabBar->addTab(GET_TEXT("ANPR/103006", "List Management"));
    ui->tabBar->addTab(GET_TEXT("ANPR/103007", "Black List Mode"));
    ui->tabBar->addTab(GET_TEXT("ANPR/103008", "White List Mode"));
    ui->tabBar->addTab(GET_TEXT("ANPR/103009", "Visitor Mode"));
    ui->tabBar->addTab(GET_TEXT("AUTOBACKUP/172003", "Auto Backup Settings"));
    connect(ui->tabBar, SIGNAL(tabClicked(int)), this, SLOT(onTabClicked(int)));

    ui->buttonGroup_channel->setCount(qMsNvr->maxChannel());
    connect(ui->buttonGroup_channel, SIGNAL(buttonClicked(int)), this, SLOT(onChannelClicked(int)));

    onLanguageChanged();
}

PageAnprSettings::~PageAnprSettings()
{
    delete ui;
}

void PageAnprSettings::setDrawWidget(QWidget *widget)
{
    ui->commonVideo->showDrawWidget(widget);
}

void PageAnprSettings::addGraphicsItem(QGraphicsItem *item)
{
    ui->commonVideo->addGraphicsItem(item);
}

bool PageAnprSettings::isChannelConnected(int channel)
{
    return LiveView::instance()->isChannelConnected(channel);
}

void PageAnprSettings::initializeData()
{
    ui->buttonGroup_channel->editCurrentIndex(0);
    m_currentChannel = 0;
    ui->tabBar->setCurrentTab(0);
}

void PageAnprSettings::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_LPR_SUPPORT:
        ON_RESPONSE_FLAG_GET_LPR_SUPPORT(message);
        break;
    }

    AnprBasePage *page = m_itemMap.value(m_currentItem, nullptr);
    if (page) {
        page->processMessage(message);
    }
}

void PageAnprSettings::ON_RESPONSE_FLAG_GET_LPR_SUPPORT(MessageReceive *message)
{
    ms_lpr_support_info *lpr_support_info = (ms_lpr_support_info *)message->data;
    if (lpr_support_info) {
        qDebug() << QString("AnprSetting::ON_RESPONSE_FLAG_GET_LPR_SUPPORT, channel: %1, support: %2, version: %3")
                        .arg(lpr_support_info->chanid)
                        .arg(lpr_support_info->vca_support)
                        .arg(lpr_support_info->vca_version);
    } else {
        qWarning() << QString("AnprSetting::ON_RESPONSE_FLAG_GET_LPR_SUPPORT, data is null.");
    }

    AnprBasePage::setAnprSupportInfo(lpr_support_info);
}

void PageAnprSettings::selectChannel(int channel)
{
  ui->commonVideo->adjustVideoRegion();
    ui->commonVideo->playVideo(channel);

    //
    bool connected = isChannelConnected(channel);
    AnprBasePage::setCurrentChannel(m_currentChannel);
    AnprBasePage::setChannelConnected(connected);
    //
    if (m_itemMap.contains(m_currentItem)) {
        AnprBasePage *page = m_itemMap.value(m_currentItem);
        page->initializeData(m_currentChannel);
    }
}

void PageAnprSettings::onLanguageChanged()
{
}

void PageAnprSettings::onTabClicked(int index)
{
    m_currentItem = static_cast<AnprItem>(index);
    AnprBasePage *page = m_itemMap.value(m_currentItem, nullptr);
    if (!page) {
        switch (m_currentItem) {
        case ItemSetting:
            page = new TabAnprSettings(this);
            break;
        case ItemListManagement:
            page = new TabAnprListManagement(this);
            break;
        case ItemBlackList:
            page = new TabAnprBlackList(this);
            break;
        case ItemWhiteList:
            page = new TabAnprWhiteList(this);
            break;
        case ItemVisitor:
            page = new TabAnprVisitor(this);
            break;
        case ItemAutoBackup:
            page = new AnprAutoBackupSetting(this);
          break;
        default:
            break;
        }
        if (page) {
            m_itemMap.insert(m_currentItem, page);
        }
    }
    if (m_currentItem == ItemListManagement || m_currentItem == ItemAutoBackup) {
        ui->widget_top->hide();
    } else {
        ui->widget_top->show();
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

        //
        if (m_currentItem == ItemListManagement || m_currentItem == ItemAutoBackup) {
            page->initializeData(m_currentChannel);
        } else {
            selectChannel(m_currentChannel);
        }
    }
}

void PageAnprSettings::onChannelClicked(int channel)
{
    m_currentChannel = channel;
    selectChannel(channel);
}
