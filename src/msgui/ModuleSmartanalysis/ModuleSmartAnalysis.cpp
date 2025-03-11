#include "ModuleSmartAnalysis.h"
#include "ui_ModuleSmartAnalysis.h"
#include "AbstractSettingPage.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "PageAnprSearch.h"
#include "PageAnprSettings.h"
#include "PageFaceDetectionSearch.h"
#include "PageFaceDetectionSettings.h"
#include "PageHeatMapSearch.h"
#include "PageHeatMapSetting.h"
#include "PagePeopleCountingSearch.h"
#include "PagePeopleCountingSettings.h"
#include "PagePosSearch.h"
#include "PagePosSettings.h"

ModuleSmartAnalysis::ModuleSmartAnalysis(QWidget *parent)
    : BaseSetting(TypeLayout, parent)
    , ui(new Ui::SmartAnalysisSetting)
{
    ui->setupUi(this);
}

ModuleSmartAnalysis::~ModuleSmartAnalysis()
{
    delete ui;
}

NetworkResult ModuleSmartAnalysis::dealNetworkCommond(const QString &commond)
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

QList<SettingItemInfo> ModuleSmartAnalysis::itemList()
{
    QList<SettingItemInfo> list;

    SettingItemInfo itemANalysisSearch(GET_TEXT("SMARTANALYSIS/54101", "Analysis Search"), ItemAnalysisSearch, PERM_MODE_SMART);
    bool anprSearchVisible = !qMsNvr->isSlaveMode() && qMsNvr->isSupportTargetMode();
    itemANalysisSearch.subItems.append(SettingItemInfo(GET_TEXT("ANPR/103005", "ANPR"), ItemAnalysisSearch_Anpr, PERM_MODE_SMART, PERM_SMART_NONE, anprSearchVisible));
    if (qMsNvr->isSupportFaceDetection()) {
        itemANalysisSearch.subItems.append(SettingItemInfo(GET_TEXT("FACE/141000", "Face Detection"), itemAnalysisSearch_FaceDetection, PERM_MODE_SMART, PERM_SMART_NONE));
    }
    itemANalysisSearch.subItems.append(SettingItemInfo(GET_TEXT("SMARTEVENT/55008", "People Counting"), ItemAnalysisSearch_PeopleCounting, PERM_MODE_SMART, PERM_SMART_NONE));
    itemANalysisSearch.subItems.append(SettingItemInfo(GET_TEXT("HEATMAP/104000", "Heat Map"), ItemAnalysisSearch_Heatmap, PERM_MODE_SMART, PERM_SMART_NONE));
    itemANalysisSearch.subItems.append(SettingItemInfo("POS", ItemAnalysisSearch_Pos, PERM_MODE_SMART, PERM_SMART_NONE));
    list.append(itemANalysisSearch);

    SettingItemInfo itemAnalysisSettings(GET_TEXT("SMARTANALYSIS/54100", "Analysis Settings"), ItemAnalysisSettings, PERM_MODE_SMART);
    bool anprSettingVisible = !qMsNvr->isSlaveMode() && qMsNvr->isSupportTargetMode();
    itemAnalysisSettings.subItems.append(SettingItemInfo(GET_TEXT("ANPR/103005", "ANPR"), ItemAnalysisSettings_Anpr, PERM_MODE_SMART, PERM_SMART_NONE, anprSettingVisible));
    if (qMsNvr->isSupportFaceDetection()) {
        itemAnalysisSettings.subItems.append(SettingItemInfo(GET_TEXT("FACE/141000", "Face Detection"), itemAnalysisSettings_FaceDetection, PERM_MODE_SMART, PERM_SMART_NONE));
    }
    itemAnalysisSettings.subItems.append(SettingItemInfo(GET_TEXT("SMARTEVENT/55008", "People Counting"), ItemAnalysisSettings_PeopleCounting, PERM_MODE_SMART, PERM_SMART_NONE));
    itemAnalysisSettings.subItems.append(SettingItemInfo(GET_TEXT("HEATMAP/104000", "Heat Map"), ItemAnalysisSettings_Heatmap, PERM_MODE_SMART, PERM_SMART_NONE));
    itemAnalysisSettings.subItems.append(SettingItemInfo("POS", ItemAnalysisSettings_Pos, PERM_MODE_SMART, PERM_SMART_NONE));
    list.append(itemAnalysisSettings);

    return list;
}

void ModuleSmartAnalysis::setCurrentItem(int item_id)
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
        case ItemAnalysisSettings_Anpr:
            page = new PageAnprSettings(this);
            break;
        case itemAnalysisSettings_FaceDetection:
            page = new PageFaceDetectionSettings(this);
            break;
        case ItemAnalysisSettings_PeopleCounting:
            page = new PagePeopleCountingSettings(this);
            break;
        case ItemAnalysisSettings_Heatmap:
            page = new PageHeatMapSetting(this);
            break;
        case ItemAnalysisSettings_Pos:
            page = new PagePosSettings(this);
            break;
        case ItemAnalysisSearch_Anpr:
            page = new PageAnprSearch(this);
            break;
        case itemAnalysisSearch_FaceDetection:
            page = new PageFaceDetectionSearch(this);
            break;
        case ItemAnalysisSearch_PeopleCounting:
            page = new PagePeopleCountingSearch(this);
            break;
        case ItemAnalysisSearch_Heatmap:
            page = new PageHeatMapSearch(this);
            break;
        case ItemAnalysisSearch_Pos:
            page = new PagePosSearch(this);
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
        page->show();
        page->initializeData();
    }
}

int ModuleSmartAnalysis::currentItem()
{
    return m_currentItemType;
}

bool ModuleSmartAnalysis::canAutoLogout()
{
    if (m_pageMap.contains(m_currentItemType)) {
        AbstractSettingPage *page = m_pageMap.value(m_currentItemType);
        return page->canAutoLogout();
    } else {
        return true;
    }
}

void ModuleSmartAnalysis::closeSetting()
{
    AbstractSettingPage *page = nullptr;
    if (m_pageMap.contains(m_currentItemType)) {
        page = m_pageMap.value(m_currentItemType);
    }
    if (page) {
        page->closePage();
    }

    BaseSetting::closeSetting();
}

void ModuleSmartAnalysis::closeCurrentPage()
{
    AbstractSettingPage *page = nullptr;
    if (m_pageMap.contains(m_currentItemType)) {
        page = m_pageMap.value(m_currentItemType);
    }
    if (page) {
        page->closePage();
    }
}

bool ModuleSmartAnalysis::isAddToVisibleList()
{
    return true;
}
