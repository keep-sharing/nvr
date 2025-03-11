#include "ActionAbstract.h"
#include "ui_ActionAbstract.h"
#include "ActionPageAbstract.h"
#include "ActionPageAlarmOutput.h"
#include "ActionPageAudibleWarning.h"
#include "ActionPageEmailLinkage.h"
#include "ActionPageEventPopup.h"
#include "ActionPageHTTPNotification.h"
#include "ActionPageOthers.h"
#include "ActionPagePtzAction.h"
#include "ActionPageWhiteLed.h"
#include "MsLanguage.h"
#include "MyButtonGroup.h"
#include "MyDebug.h"

extern "C" {
#include "msdb.h"
}

ActionAbstract::ActionAbstract(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::ActionAbstract)
{
    ui->setupUi(this);

    onLanguageChanged();
    m_buttonGroup = new MyButtonGroup(this);
    m_buttonGroup->addButton(ui->pushButtonAudibleWarning, TabAudibleWarning);
    m_buttonGroup->addButton(ui->pushButtonEmailLinkage, TabEmailLinkage);
    m_buttonGroup->addButton(ui->pushButtonEventPopup, TabEventPopup);
    m_buttonGroup->addButton(ui->pushButtonPtzAction, TabPtzAction);
    m_buttonGroup->addButton(ui->pushButtonAlarmOutput, TabAlarmOutput);
    m_buttonGroup->addButton(ui->pushButtonWhiteLed, TabWhiteLed);
    m_buttonGroup->addButton(ui->pushButtonOthers, TabOthers);
    m_buttonGroup->addButton(ui->pushButtonHTTP, TabHTTP);
    connect(m_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onTabButtonClicked(int)));

    resize(1210, 880);
}

ActionAbstract::~ActionAbstract()
{
    delete ui;
}

void ActionAbstract::showAction()
{
    initialize();
    m_buttonGroup->setCurrentId(TabAudibleWarning);
    show();
}

bool ActionAbstract::hasCache() const
{
    return m_cache;
}

void ActionAbstract::setCached()
{
    m_cache = true;
}

void ActionAbstract::clearCache()
{
    m_cache = false;
    clearPageCache();
}

void ActionAbstract::processMessage(MessageReceive *message)
{
    ActionPageAbstract *page = nullptr;
    if (m_pageMap.contains(m_currentTab)) {
        page = m_pageMap.value(m_currentTab);
    }
    if (page) {
        page->dealMessage(message);
    }
}

void ActionAbstract::initialize()
{
    Tabs tabs = actionTabs();
    ui->pushButtonAudibleWarning->setVisible(tabs & TabAudibleWarning);
    ui->pushButtonEmailLinkage->setVisible(tabs & TabEmailLinkage);
    ui->pushButtonEventPopup->setVisible(tabs & TabEventPopup);
    ui->pushButtonPtzAction->setVisible(tabs & TabPtzAction);
    ui->pushButtonAlarmOutput->setVisible(tabs & TabAlarmOutput);
    ui->pushButtonWhiteLed->setVisible(tabs & TabWhiteLed);
    ui->pushButtonOthers->setVisible(tabs & TabOthers);
    ui->pushButtonHTTP->setVisible(tabs & TabHTTP);
}

int ActionAbstract::channel() const
{
    return m_channel;
}

void ActionAbstract::clearPageCache()
{
    for (auto iter = m_pageMap.constBegin(); iter != m_pageMap.constEnd(); ++iter) {
        ActionPageAbstract *page = iter.value();
        page->clearCache();
    }
}

ActionAbstract::Tabs ActionAbstract::actionTabs()
{
    return Tabs(TabAudibleWarning | TabEmailLinkage | TabEventPopup | TabPtzAction | TabAlarmOutput | TabWhiteLed | TabOthers| TabHTTP);
}

bool ActionAbstract::hasTriggerChannelsSnapshot()
{
    return false;
}

char *ActionAbstract::emailLinkageTriggerChannelsSnapshot()
{
    return nullptr;
}

int ActionAbstract::emailLinkageTriggerChannelsSnapshotArraySize()
{
    return MAX_LEN_65;
}

bool ActionAbstract::hasTriggerChannelEventPopup()
{
    return false;
}

int *ActionAbstract::eventPopupTriggerLayout()
{
    return nullptr;
}

int *ActionAbstract::eventPopupTriggerChannels()
{
    return nullptr;
}

int ActionAbstract::ptzActionsArraySize()
{
    return MAX_CAMERA;
}

int ActionAbstract::whiteLedParamsArraySize()
{
    return MAX_CAMERA;
}

bool ActionAbstract::isNoteVisible() const
{
    return true;
}

void ActionAbstract::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("VIDEOLOSS/50002", "Action"));
    ui->pushButtonAudibleWarning->setText(GET_TEXT("VIDEOLOSS/50009", "Audible Warning"));
    ui->pushButtonEmailLinkage->setText(GET_TEXT("VIDEOLOSS/50010", "Email Linkage"));
    ui->pushButtonEventPopup->setText(GET_TEXT("SYSTEMGENERAL/70041", "Event Popup"));
    ui->pushButtonPtzAction->setText(GET_TEXT("VIDEOLOSS/50014", "PTZ Action"));
    ui->pushButtonAlarmOutput->setText(GET_TEXT("ALARMOUT/53000", "Alarm Output"));
    ui->pushButtonWhiteLed->setText(GET_TEXT("WHITELED/105000", "White LED"));
    ui->pushButtonOthers->setText(GET_TEXT("VIDEOLOSS/50011", "Others"));
    ui->pushButtonOk->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButtonCancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
    ui->pushButtonHTTP->setText(GET_TEXT("ACTION/153000", "HTTP Notification"));
}

void ActionAbstract::onTabButtonClicked(int id)
{
    m_currentTab = static_cast<Tab>(id);
    ActionPageAbstract *page = m_pageMap.value(m_currentTab, nullptr);
    if (!page) {
        switch (m_currentTab) {
        case TabAlarmOutput:
            page = new ActionPageAlarmOutput(this);
            break;
        case TabAudibleWarning:
            page = new ActionPageAudibleWarning(this);
            break;
        case TabEmailLinkage:
            page = new ActionPageEmailLinkage(this);
            break;
        case TabEventPopup:
            page = new ActionPageEventPopup(this);
            break;
        case TabOthers:
            page = new ActionPageOthers(this);
            break;
        case TabPtzAction:
            page = new ActionPagePtzAction(this);
            break;
        case TabWhiteLed:
            page = new ActionPageWhiteLed(this);
            break;
        case TabHTTP:
            page = new ActionPageHTTPNotification(this);
            break;
        default:
            break;
        }
        if (page) {
            connect(this, SIGNAL(cancelClicked()), page, SLOT(onCancelClicked()));
            m_pageMap.insert(m_currentTab, page);
        }
    }
    if (!page) {
        qMsWarning() << "invalid tab:" << m_currentTab;
        return;
    }
    QLayoutItem *item = ui->gridLayoutContainer->itemAtPosition(0, 0);
    if (item) {
        QWidget *widget = item->widget();
        if (widget) {
            widget->hide();
        }
        ui->gridLayoutContainer->removeItem(item);
        delete item;
    }
    ui->gridLayoutContainer->addWidget(page, 0, 0);
    page->show();
}

void ActionAbstract::on_pushButtonOk_clicked()
{
    for (auto iter = m_pageMap.constBegin(); iter != m_pageMap.constEnd(); ++iter) {
        ActionPageAbstract *page = iter.value();
        if (page->saveData() != 0) {
            return;
        }
    }
    accept();
}

void ActionAbstract::on_pushButtonCancel_clicked()
{
    emit cancelClicked();
    reject();
}
