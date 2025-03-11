#include "ModuleRetrieve.h"
#include "ui_ModuleRetrieve.h"
#include "MsLanguage.h"
#include "PageCommonBackup.h"
#include "PageEventBackup.h"
#include "PagePictureBackup.h"
#include <QtDebug>

ModuleRetrieve::ModuleRetrieve(QWidget *parent)
    : BaseSetting(BaseSetting::TypeRetrueve, parent)
    , ui(new Ui::Retrieve)
{
    ui->setupUi(this);
}

ModuleRetrieve::~ModuleRetrieve()
{
    delete ui;
}

NetworkResult ModuleRetrieve::dealNetworkCommond(const QString &commond)
{
    qDebug() << "Retrieve::dealNetworkCommond," << commond;

    NetworkResult result = NetworkReject;

    if (commond.startsWith("R_Click")) {
        if (isCloseable()) {
            emit sig_close();
            result = NetworkAccept;
        }
        return result;
    }

    AbstractSettingPage *page = nullptr;
    if (m_pageMap.contains(m_currentItemType)) {
        page = m_pageMap.value(m_currentItemType);
    }
    if (page) {
        result = page->dealNetworkCommond(commond);
    }
    return result;
}

void ModuleRetrieve::dealMessage(MessageReceive *message)
{
    AbstractSettingPage *page = nullptr;
    if (m_pageMap.contains(m_currentItemType)) {
        page = m_pageMap.value(m_currentItemType);
    }
    if (page) {
        page->dealMessage(message);
    }
}

QList<SettingItemInfo> ModuleRetrieve::itemList()
{
    QList<SettingItemInfo> list;
    list.append(SettingItemInfo(GET_TEXT("COMMONBACKUP/100000", "Common Backup"), ItemCommonBackup, PERM_MODE_RETRIEVE, PERM_RETRIEVE_NONE));
    list.append(SettingItemInfo(GET_TEXT("COMMONBACKUP/100003", "Event Backup"), ItemEventBackup, PERM_MODE_RETRIEVE, PERM_RETRIEVE_NONE));
    list.append(SettingItemInfo(GET_TEXT("COMMONBACKUP/100004", "Picture Backup"), ItemPictureBackup, PERM_MODE_RETRIEVE, PERM_RETRIEVE_NONE));
    return list;
}

void ModuleRetrieve::setCurrentItem(int item_id)
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
        case ItemCommonBackup:
            page = new PageCommonBackup(this);
            break;
        case ItemEventBackup:
            page = new PageEventBackup(this);
            break;
        case ItemPictureBackup:
            page = new PagePictureBackup(this);
            break;
        default:
            break;
        }
        if (page) {
            connect(page, SIGNAL(sig_back()), this, SIGNAL(sig_close()));
            connect(page, SIGNAL(updateVideoGeometry()), this, SIGNAL(updateVideoGeometry()));
            m_pageMap.insert(m_currentItemType, page);
        }
    }
    if (page) {
        ui->gridLayout->addWidget(page);
        page->initializeData();
        page->show();
        emit updateVideoGeometry();
    }
}

bool ModuleRetrieve::isCloseable()
{
    if (m_pageMap.contains(m_currentItemType)) {
        AbstractSettingPage *page = m_pageMap.value(m_currentItemType);
        return page->isCloseable();
    } else {
        return true;
    }
}

void ModuleRetrieve::closeSetting()
{
    if (m_pageMap.contains(m_currentItemType)) {
        AbstractSettingPage *page = m_pageMap.value(m_currentItemType);
        return page->closePage();
    }
    BaseSetting::closeSetting();
}

bool ModuleRetrieve::canAutoLogout()
{
    if (m_pageMap.contains(m_currentItemType)) {
        AbstractSettingPage *page = m_pageMap.value(m_currentItemType);
        return page->canAutoLogout();
    } else {
        return true;
    }
}

int ModuleRetrieve::currentItem()
{
    return m_currentItemType;
}

bool ModuleRetrieve::isAddToVisibleList()
{
    return true;
}
