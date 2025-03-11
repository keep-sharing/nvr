#include "AdvancedSettings.h"
#include "ui_AdvancedSettings.h"
#include "AbstractAdvancedSettingsPage.h"
#include "advancedsettings_p.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "Watermark.h"
#include <QDebug>
#include <QScopedValueRollback>

AdvancedSettings::AdvancedSettings(QWidget *parent)
    : AbstractCameraPage(parent)
    , ui(new Ui::AdvancedSettings)
{
    ui->setupUi(this);
    d = new AdvancedSettingsPrivate(this);

    ui->tabBar->addTab(GET_TEXT("CAMERAADVANCE/106001", "Watermark"));
    connect(ui->tabBar, SIGNAL(tabClicked(int)), d, SLOT(onTabClicked(int)));

    ui->channelsGroup->setCount(qMsNvr->maxChannel());
    connect(ui->channelsGroup, SIGNAL(buttonClicked(int)), d, SLOT(onChannelClicked(int)));
}

AdvancedSettings::~AdvancedSettings()
{
    delete ui;
}

void AdvancedSettings::initializeData()
{
    QScopedValueRollback<bool> cleanup(d->isInitializing);
    d->isInitializing = true;
    ui->tabBar->setCurrentTab(0);
    ui->channelsGroup->setCurrentIndex(0);
}

void AdvancedSettings::dealMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void AdvancedSettings::setDrawWidget(QWidget *widget)
{
    ui->commonVideo->showDrawWidget(widget);
}

void AdvancedSettingsPrivate::onTabClicked(int index)
{
    auto oldItem = currentItem;
    currentItem = static_cast<ItemCategory>(index);

    if (currentItem == ItemNone)
        return;

    if (items.contains(oldItem))
        items[oldItem]->hideDrawWidget();
    AbstractAdvancedSettingsPage *page = nullptr;
    if (!items.contains(currentItem)) {
        switch (currentItem) {
        case ItemWatermark:
            page = new Watermark(q);
            break;
        default:
            break;
        }
        items[currentItem] = page;
    } else {
        page = items[currentItem];
    }

    QLayoutItem *item = q->ui->gridLayout->itemAtPosition(0, 0);
    if (item) {
        QWidget *widget = item->widget();
        if (widget)
            widget->hide();
        q->ui->gridLayout->removeItem(item);
        delete item;
    }

    q->setDrawWidget(nullptr);
    if (page) {
        q->ui->gridLayout->addWidget(page, 0, 0);
        page->show();
        page->setChannelConnected(isChannelConnected(currentChannel));
        page->setChannel(currentChannel);
        page->initializeData();
    }
}

void AdvancedSettingsPrivate::onChannelClicked(int channel)
{
    currentChannel = channel;
    q->ui->commonVideo->playVideo(channel);

    if (items.contains(currentItem)) {
        AbstractAdvancedSettingsPage *page = items[currentItem];
        page->setChannelConnected(isChannelConnected(currentChannel));
        page->setChannel(currentChannel);
        page->initializeData();
    }
}

bool AdvancedSettingsPrivate::isChannelConnected(int channel)
{
    return LiveView::instance()->isChannelConnected(channel);
}

AdvancedSettingsPrivate::AdvancedSettingsPrivate(QObject *parent)
    : QObject(parent)
{
    q = qobject_cast<AdvancedSettings *>(parent);
}
