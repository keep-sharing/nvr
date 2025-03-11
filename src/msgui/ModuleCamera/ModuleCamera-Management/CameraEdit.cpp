#include "CameraEdit.h"
#include "ui_CameraEdit.h"
#include "CameraEditTabParameters.h"
#include "CameraEditTabParametersOld.h"
#include "CameraEditTabSettings.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include <QFile>

CameraEdit::CameraEdit(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::CameraEdit)
{
    ui->setupUi(this);

    QFile file(":/style/style/settingstyle.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(file.readAll());
    }
    file.close();

    ui->tabBar->addTab(GET_TEXT("SMARTEVENT/55009", "Settings"), TabSettings);
    ui->tabBar->addTab(GET_TEXT("CAMERAEDIT/31001", "Parameters"), TabParametersOld);
    connect(ui->tabBar, SIGNAL(tabClicked(int)), this, SLOT(onTabBarClicked(int)));

    onLanguageChanged();
}

CameraEdit::~CameraEdit()
{
    delete ui;
}

void CameraEdit::showEdit(int channel)
{
    show();
    //
    m_channel = channel;
    ui->tabBar->setCurrentTab(TabSettings);
}

void CameraEdit::showEdit(int channel, const resq_get_ipcdev &ipcdev)
{
    show();
    //
    memset(&m_ipcdev, 0, sizeof(m_ipcdev));
    memcpy(&m_ipcdev, &ipcdev, sizeof(m_ipcdev));

    m_channel = channel;
    ui->tabBar->setCurrentTab(TabSettings);
}

void CameraEdit::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPC_FRAME_RESOLUTION:
        ON_RESPONSE_FLAG_GET_IPC_FRAME_RESOLUTION(message);
        break;
    default:
        break;
    }
}

void CameraEdit::ON_RESPONSE_FLAG_GET_IPC_FRAME_RESOLUTION(MessageReceive *message)
{
    m_jsonData.clear();
    if (message->data && message->header.size > 0) {
        m_jsonData = QByteArray((char *)message->data, message->header.size);
    }
    m_eventLoop.exit();
}

void CameraEdit::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("CAMERAEDIT/31000", "Camera Edit"));
    ui->tabBar->setTabText(TabSettings, GET_TEXT("SMARTEVENT/55009", "Settings"));
    ui->tabBar->setTabText(TabParametersOld, GET_TEXT("CAMERAEDIT/31001", "Parameters"));
}

void CameraEdit::onTabBarClicked(int index)
{
    m_currentTab = index;
    AbstractSettingTab *settingTab = nullptr;

    if (m_currentTab == TabParametersOld || m_currentTab == TabParameters) {
        sendMessage(REQUEST_FLAG_GET_IPC_FRAME_RESOLUTION, &m_channel, sizeof(int));
        //m_eventLoop.exec();
        if (m_jsonData.isEmpty()) {
            m_currentTab = TabParametersOld;
        } else {
            m_currentTab = TabParameters;
        }
    }

    settingTab = m_tabsMap.value(m_currentTab, nullptr);
    switch (m_currentTab) {
    case TabSettings:
        if (!settingTab) {
            settingTab = new CameraEditTabSettings(this);
            m_tabsMap.insert(m_currentTab, settingTab);
        }
        break;
    case TabParameters:
        if (!settingTab) {
            settingTab = new CameraEditTabParameters(this);
            m_tabsMap.insert(m_currentTab, settingTab);
        }
        break;
    case TabParametersOld:
        //MsWaitting::showGlobalWait();
        if (!settingTab) {
            settingTab = new CameraEditTabParametersOld(this);
            m_tabsMap.insert(m_currentTab, settingTab);
        }
        break;
    default:
        break;
    }

    //
    QLayoutItem *item = ui->gridLayoutContent->itemAtPosition(0, 0);
    if (item) {
        QWidget *widget = item->widget();
        if (widget) {
            widget->hide();
        }
        ui->gridLayoutContent->removeItem(item);
        delete item;
    }
    //
    if (settingTab) {
        ui->gridLayoutContent->addWidget(settingTab, 0, 0);
        settingTab->show();
        settingTab->initializeData();
    }
}
