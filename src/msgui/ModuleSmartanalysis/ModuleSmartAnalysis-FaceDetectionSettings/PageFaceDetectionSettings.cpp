#include "PageFaceDetectionSettings.h"
#include "ui_PageFaceDetectionSettings.h"
#include "MsLanguage.h"
#include "TabFaceAdvanced.h"
#include "TabFaceCapture.h"

PageFaceDetectionSettings::PageFaceDetectionSettings(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::FaceDetectionSettings)
{
    ui->setupUi(this);

    ui->tabBar->addTab(GET_TEXT("FACE/141031", "Face Capture"), TAB_FACE_CAPTURE);
    ui->tabBar->addTab(GET_TEXT("PTZCONFIG/36016", "Advanced"), TAB_ADVANCED);
    connect(ui->tabBar, SIGNAL(tabClicked(int)), this, SLOT(onTabClicked(int)));
}

PageFaceDetectionSettings::~PageFaceDetectionSettings()
{
    delete ui;
}

void PageFaceDetectionSettings::initializeData()
{
    AbstractSettingTab::setCurrentChannel(0);

    m_currentPageTab = TAB_FACE_CAPTURE;
    ui->tabBar->setCurrentTab(TAB_FACE_CAPTURE);
}

void PageFaceDetectionSettings::onTabClicked(int index)
{
    m_currentPageTab = static_cast<SETTINGS_TAB>(index);
    AbstractSettingTab *settingTab = m_typeMap.value(m_currentPageTab, nullptr);
    if (!settingTab) {
        switch (m_currentPageTab) {
        case TAB_FACE_CAPTURE:
            settingTab = new TabFaceCapture(this);
            break;
        case TAB_ADVANCED:
            settingTab = new TabFaceAdvanced(this);
            break;
        default:
            break;
        }
        if (settingTab) {
            connect(settingTab, SIGNAL(sig_back()), this, SIGNAL(sig_back()));
            m_typeMap.insert(m_currentPageTab, settingTab);
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
    if (settingTab) {
        ui->gridLayout->addWidget(settingTab, 0, 0);
        settingTab->show();
        settingTab->initializeData();
    }
}
