#include "smartevent.h"
#include "ui_smartevent.h"
#include "AdvancedMotionDetection.h"
#include "HumanDetection.h"
#include "LineCrossing.h"
#include "Loitering.h"
#include "MessageBox.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "RegionEntrance.h"
#include "RegionExiting.h"
#include "TabPeopleCountingSettings.h"
#include "TamperDetection.h"
#include "objectleftremoved.h"

SmartEvent::SmartEvent(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::SmartEvent)
{
    ui->setupUi(this);

    ui->tabBar->setHSpacing(25);
    ui->tabBar->addTab(GET_TEXT("SMARTEVENT/55001", "Region Entrance"), Item_RegionEntrance);
    ui->tabBar->addTab(GET_TEXT("SMARTEVENT/55002", "Region Exiting"), Item_RegionExiting);
    ui->tabBar->addTab(GET_TEXT("SMARTEVENT/55003", "Advanced Motion Detection"), Item_AdvancedMotionDetection);
    ui->tabBar->addTab(GET_TEXT("SMARTEVENT/55004", "Tamper Detection"), Item_TamperDeterction);
    ui->tabBar->addTab(GET_TEXT("SMARTEVENT/55005", "Line Crossing"), Item_LineCrossing);
    ui->tabBar->addTab(GET_TEXT("SMARTEVENT/55006", "Loitering"), Item_Loitering);
    ui->tabBar->addTab(GET_TEXT("SMARTEVENT/55007", "Human Detection"), Item_HumanDetection);
    ui->tabBar->addTab(GET_TEXT("SMARTEVENT/55055", "Object Left/Removed"), Item_ObjectLeftRemoved);
    connect(ui->tabBar, SIGNAL(tabClicked(int)), this, SLOT(onTabBarClicked(int)));

    ui->widgetMessage->hide();

    onLanguageChanged();
}

SmartEvent::~SmartEvent()
{
    delete ui;
}

void SmartEvent::initializeData()
{
    BaseSmartWidget::setCurrentChannel(0);
    ui->tabBar->setCurrentTab(0);
}

void SmartEvent::dealMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void SmartEvent::setApplyButtonEnable(bool enable)
{
    ui->pushButton_apply->setEnabled(enable);
    ui->pushButtonCopy->setEnabled(enable);
}

void SmartEvent::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
}

void SmartEvent::onLanguageChanged()
{
    ui->pushButtonCopy->setText(GET_TEXT("COMMON/1005", "Copy"));
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
}

void SmartEvent::onTabBarClicked(int index)
{
    //
    if (m_pageMap.contains(m_currentItemType)) {
        BaseSmartWidget *page = m_pageMap.value(m_currentItemType);
        ui->gridLayout->removeWidget(page);
        page->hide();
    }

    //
    m_currentItemType = static_cast<SmartItem>(index);
    BaseSmartWidget *page = nullptr;
    if (m_pageMap.contains(m_currentItemType)) {
        page = m_pageMap.value(m_currentItemType);
    } else {
        switch (m_currentItemType) {
        case Item_RegionEntrance:
            page = new RegionEntrance(this);
            break;
        case Item_RegionExiting:
            page = new RegionExiting(this);
            break;
        case Item_AdvancedMotionDetection:
            page = new AdvancedMotionDetection(this);
            break;
        case Item_TamperDeterction:
            page = new TamperDetection(this);
            break;
        case Item_LineCrossing:
            page = new LineCrossing(this);
            break;
        case Item_Loitering:
            page = new Loitering(this);
            break;
        case Item_HumanDetection:
            page = new HumanDetection(this);
            break;
        case Item_ObjectLeftRemoved:
            page = new ObjectLeftRemoved(this);
            break;
        default:
            break;
        }
        if (page) {
            m_pageMap.insert(m_currentItemType, page);
            connect(page, SIGNAL(showMessage(QString)), this, SLOT(onShowMessage(QString)));
            connect(page, SIGNAL(hideMessage()), this, SLOT(onHideMessage()));
        }
    }
    if (page) {
        ui->gridLayout->addWidget(page);
        page->show();
        page->initializeData(BaseSmartWidget::currentChannel());
    }
}

void SmartEvent::on_pushButton_apply_clicked()
{
    if (m_pageMap.contains(m_currentItemType)) {
        BaseSmartWidget *page = m_pageMap.value(m_currentItemType);
        page->saveData();
    }
}

void SmartEvent::on_pushButton_back_clicked()
{
    emit sig_back();
}
void SmartEvent::onShowMessage(QString message)
{
    ui->widgetMessage->showMessage(message);
}
void SmartEvent::onHideMessage()
{
    ui->widgetMessage->hide();
}

void SmartEvent::on_pushButtonCopy_clicked()
{
    if (m_pageMap.contains(m_currentItemType)) {
        BaseSmartWidget *page = m_pageMap.value(m_currentItemType);
        page->copyData();
    }
}
