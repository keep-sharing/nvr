#include "ModuleEvent.h"
#include "ui_ModuleEvent.h"
#include "AbstractSettingPage.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "NetworkPageMore.h"
#include "PageAlarmInput.h"
#include "PageAlarmOutput.h"
#include "PageAudioAlarm.h"
#include "PageException.h"
#include "PageMotionDetection.h"
#include "PageVideoLoss.h"
#include "smartevent.h"
#include <QtDebug>

ModuleEvent::ModuleEvent(QWidget *parent)
    : BaseSetting(BaseSetting::TypeEvent, parent)
    , ui(new Ui::ModuleEvent)
{
    ui->setupUi(this);
}

ModuleEvent::~ModuleEvent()
{
    delete ui;
}

void ModuleEvent::dealMessage(MessageReceive *message)
{
    AbstractSettingPage *page = nullptr;
    if (m_pageMap.contains(m_currentItemType)) {
        page = m_pageMap.value(m_currentItemType);
    }
    if (page) {
        page->dealMessage(message);
    }
}

QList<SettingItemInfo> ModuleEvent::itemList()
{
    QList<SettingItemInfo> list;
    //热备模式只保留Exception，且隐藏Exception - Email Linkage选项
    if (qMsNvr->isSlaveMode()) {
        list.append(SettingItemInfo(GET_TEXT("MOTION/51000", "Motion Detection"), ItemMotion, PERM_MODE_EVENT, PERM_EVENT_NONE, false));
        list.append(SettingItemInfo(GET_TEXT("AUDIO_ALARM/159000", "Audio Alarm"), ItemAudioAlarm, PERM_MODE_EVENT, PERM_EVENT_NONE, false));
        list.append(SettingItemInfo(GET_TEXT("VIDEOLOSS/50001", "Video Loss"), ItemVideoLoss, PERM_MODE_EVENT, PERM_EVENT_NONE, false));
        list.append(SettingItemInfo(GET_TEXT("ALARMIN/52001", "Alarm Input"), ItemAlarmInput, PERM_MODE_EVENT, PERM_EVENT_NONE, false));
        list.append(SettingItemInfo(GET_TEXT("VIDEOLOSS/50019", "Alarm Output"), ItemAlarmOutput, PERM_MODE_EVENT, PERM_EVENT_NONE, false));
        list.append(SettingItemInfo(GET_TEXT("EXCEPTION/54000", "Exception"), ItemException, PERM_MODE_EVENT, PERM_EVENT_NONE));
        list.append(SettingItemInfo(GET_TEXT("SMARTEVENT/55000", "VCA"), ItemSmartEvent, PERM_MODE_EVENT, PERM_EVENT_NONE, false));
    } else {
        list.append(SettingItemInfo(GET_TEXT("MOTION/51000", "Motion Detection"), ItemMotion, PERM_MODE_EVENT, PERM_EVENT_NONE));
        list.append(SettingItemInfo(GET_TEXT("AUDIO_ALARM/159000", "Audio Alarm"), ItemAudioAlarm, PERM_MODE_EVENT, PERM_EVENT_NONE));
        list.append(SettingItemInfo(GET_TEXT("VIDEOLOSS/50001", "Video Loss"), ItemVideoLoss, PERM_MODE_EVENT, PERM_EVENT_NONE));
        list.append(SettingItemInfo(GET_TEXT("ALARMIN/52001", "Alarm Input"), ItemAlarmInput, PERM_MODE_EVENT, PERM_EVENT_NONE));
        list.append(SettingItemInfo(GET_TEXT("VIDEOLOSS/50019", "Alarm Output"), ItemAlarmOutput, PERM_MODE_EVENT, PERM_EVENT_NONE));
        list.append(SettingItemInfo(GET_TEXT("EXCEPTION/54000", "Exception"), ItemException, PERM_MODE_EVENT, PERM_EVENT_NONE));
        list.append(SettingItemInfo(GET_TEXT("SMARTEVENT/55000", "VCA"), ItemSmartEvent, PERM_MODE_EVENT, PERM_EVENT_NONE));
    }
    return list;
}

void ModuleEvent::setCurrentItem(int item_id)
{
    //
    if (m_pageMap.contains(m_currentItemType)) {
        AbstractSettingPage *page = m_pageMap.value(m_currentItemType);
        ui->gridLayout->removeWidget(page);
        page->hide();
    }

    //
    m_currentItemType = static_cast<ItemType>(item_id);
    AbstractSettingPage *page = nullptr;
    if (m_pageMap.contains(m_currentItemType)) {
        page = m_pageMap.value(m_currentItemType);
    } else {
        switch (m_currentItemType) {
        case ItemMotion:
            page = new PageMotionDetection(this);
            break;
        case ItemVideoLoss:
            page = new PageVideoLoss(this);
            break;
        case ItemAlarmInput:
            page = new PageAlarmInput(this);
            break;
        case ItemAlarmOutput:
            page = new PageAlarmOutput(this);
            break;
        case ItemException:
            page = new PageException(this);
            break;
        case ItemSmartEvent:
            page = new SmartEvent(this);
            break;
        case ItemAudioAlarm:
            page = new PageAudioAlarm(this);
            break;
        default:
            break;
        }
        if (page) {
            connect(page, SIGNAL(sig_back()), this, SIGNAL(sig_close()));
            m_pageMap.insert(m_currentItemType, page);
        }
    }
    if (page) {
        ui->gridLayout->addWidget(page);
        page->show();
        page->initializeData();
    }
}

int ModuleEvent::currentItem()
{
    return m_currentItemType;
}
