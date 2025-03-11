#include "LiveviewBottomBar.h"
#include "ui_LiveviewBottomBar.h"
#include "BottomBar.h"
#include "LiveView.h"
#include "LiveViewOccupancyManager.h"
#include "LiveViewTarget.h"
#include "LogWrite.h"
#include "MessageBox.h"
#include "MessageFilter.h"
#include "MsApplication.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "PopupContent.h"
#include "SubControl.h"
#include "UpgradeThread.h"
#include "centralmessage.h"
#include "globalwaitting.h"
#include "mainwindow.h"
#include "msuser.h"
#include "userlogin.h"
#include <QDesktopWidget>

LiveviewBottomBar *LiveviewBottomBar::s_self = nullptr;

LiveviewBottomBar::LiveviewBottomBar(QWidget *parent)
    : MsWidget(parent)
    , ui(new Ui::LiveviewBottomBar)
{
    ui->setupUi(this);

    s_self = this;

    //
    connect(SubControl::instance(), SIGNAL(screenSwitched()), this, SLOT(onScreenSwitched()));

    //
    m_defaultNormalLayoutNames.insert("LAYOUTMODE_1", 0);
    m_defaultNormalLayoutNames.insert("LAYOUTMODE_4", 0);
    m_defaultNormalLayoutNames.insert("LAYOUTMODE_9", 0);
    m_defaultNormalLayoutNames.insert("LAYOUTMODE_12", 0);
    m_defaultNormalLayoutNames.insert("LAYOUTMODE_16", 0);
    m_defaultNormalLayoutNames.insert("LAYOUTMODE_25", 0);
    m_defaultNormalLayoutNames.insert("LAYOUTMODE_32", 0);
    m_defaultNormalLayoutNames.insert("LAYOUTMODE_64", 0);

    //layout
    m_layoutButtonGroup = new QButtonGroup(this);
    m_layoutButtonGroup->setExclusive(false);
    m_layoutButtonGroup->addButton(ui->toolButton_layout1, LAYOUTMODE_1);
    m_layoutButtonGroup->addButton(ui->toolButton_layout4, LAYOUTMODE_4);
    m_layoutButtonGroup->addButton(ui->toolButton_layout9, LAYOUTMODE_9);
    m_layoutButtonGroup->addButton(ui->toolButton_layout12, LAYOUTMODE_12);
    m_layoutButtonGroup->addButton(ui->toolButton_layout16, LAYOUTMODE_16);
    m_layoutButtonGroup->addButton(ui->toolButton_layout25, LAYOUTMODE_25);
    m_layoutButtonGroup->addButton(ui->toolButton_layout32, LAYOUTMODE_32);
    m_layoutButtonGroup->addButton(ui->toolButton_layout64, LAYOUTMODE_64);
    connect(m_layoutButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onLayoutButtonGroupClicked(int)));
    ui->toolButton_layout1->setNormalIcon(":/bottombar/bottombar/layout_1.png");
    ui->toolButton_layout1->setCheckedIcon(":/bottombar/bottombar/layout_1_checked.png");
    ui->toolButton_layout1->setData("LAYOUTMODE_1", Qt::UserRole);
    ui->toolButton_layout4->setNormalIcon(":/bottombar/bottombar/layout_4.png");
    ui->toolButton_layout4->setCheckedIcon(":/bottombar/bottombar/layout_4_checked.png");
    ui->toolButton_layout4->setData("LAYOUTMODE_4", Qt::UserRole);
    ui->toolButton_layout9->setNormalIcon(":/bottombar/bottombar/layout_9.png");
    ui->toolButton_layout9->setCheckedIcon(":/bottombar/bottombar/layout_9_checked.png");
    ui->toolButton_layout9->setData("LAYOUTMODE_9", Qt::UserRole);
    ui->toolButton_layout12->setNormalIcon(":/bottombar/bottombar/layout_12.png");
    ui->toolButton_layout12->setCheckedIcon(":/bottombar/bottombar/layout_12_checked.png");
    ui->toolButton_layout12->setData("LAYOUTMODE_12", Qt::UserRole);
    ui->toolButton_layout16->setNormalIcon(":/bottombar/bottombar/layout_16.png");
    ui->toolButton_layout16->setCheckedIcon(":/bottombar/bottombar/layout_16_checked.png");
    ui->toolButton_layout16->setData("LAYOUTMODE_16", Qt::UserRole);
    ui->toolButton_layout25->setNormalIcon(":/bottombar/bottombar/layout_25.png");
    ui->toolButton_layout25->setCheckedIcon(":/bottombar/bottombar/layout_25_checked.png");
    ui->toolButton_layout25->setData("LAYOUTMODE_25", Qt::UserRole);
    ui->toolButton_layout32->setNormalIcon(":/bottombar/bottombar/layout_32.png");
    ui->toolButton_layout32->setCheckedIcon(":/bottombar/bottombar/layout_32_checked.png");
    ui->toolButton_layout32->setData("LAYOUTMODE_32", Qt::UserRole);
    ui->toolButton_layout64->setNormalIcon(":/bottombar/bottombar/layout_64.png");
    ui->toolButton_layout64->setCheckedIcon(":/bottombar/bottombar/layout_64_checked.png");
    ui->toolButton_layout64->setData("LAYOUTMODE_64", Qt::UserRole);
    connect(LiveView::instance(), SIGNAL(layoutChanged(CustomLayoutKey)), this, SLOT(onLayoutModeChanged(CustomLayoutKey)));
    connect(this, SIGNAL(setLiveviewLayoutMode(int)), LiveView::instance(), SLOT(onSetLayoutMode(int)));
    connect(this, SIGNAL(setVideoRatio(int)), LiveView::instance(), SLOT(onSetVideoRatio(int)));

    m_irregularLayout = new IrregularLayout(this);
    connect(m_irregularLayout, SIGNAL(layoutButtonClicked(int)), this, SLOT(onLayoutButtonGroupClicked(int)));

    //anpr
    ui->toolButton_target->setNormalIcon(":/bottombar/bottombar/targetmode.png");
    ui->toolButton_target->setCheckedIcon(":/bottombar/bottombar/targetmode_checked.png");

    //
    ui->toolButtonOccupancy->setNormalIcon(":/liveview_occupancy/liveview_occupancy/occupancy.png");
    ui->toolButtonOccupancy->setCheckedIcon(":/liveview_occupancy/liveview_occupancy/occupancy_checked.png");

    //
    m_eventNotification = new EventNotification(this);
    ui->toolButton_event->setNormalPixmap(QPixmap(":/bottombar/bottombar/event.png"));
    QList<QPixmap> notificationPixmapList;
    notificationPixmapList.append(QPixmap(":/bottombar/bottombar/event.png"));
    notificationPixmapList.append(QPixmap());
    ui->toolButton_event->setFlashPixmapList(notificationPixmapList);
    ui->toolButton_event->setVisible(false);
    ui->toolButton_event->setFlashInterval(500);

    //
    m_displaySetting = new DisplaySetting(this);
    connect(m_displaySetting, SIGNAL(displayStateChanged(bool)), this, SLOT(onDisplayStateChanged(bool)));
    m_displaySetting->initializeData();
    m_displaySetting->initializePlayMode();

    //
    m_disturbing = new Disturbing(this);

    //
    m_sequence = new Sequence(this);
    connect(m_sequence, SIGNAL(sequenceStateChanged(int, bool)), this, SLOT(onSequenceStateChanged(int, bool)));
    m_sequence->initializeData();

    //
    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();

    //热备模式隐藏Start All Recording、Stop All Recording、target、Occupancy
    if (qMsNvr->isSlaveMode()) {
        ui->toolButton_startRecord->hide();
        ui->toolButton_stopRecord->hide();
        ui->toolButton_target->hide();
        ui->toolButtonOccupancy->hide();
    }

    //anpr
    if (!qMsNvr->isSupportTargetMode()) {
        ui->line->setVisible(false);
        ui->toolButton_target->setVisible(false);
    }

#if defined(_NT98323_) || defined(_NT98633_)
    ui->toolButton_scaleMode->hide();
#endif

    //兼容出现多个焦点问题
    m_buttons = findChildren<MyToolButton *>();
    m_buttons.append(BottomBar::instance()->lockButton());
    for (int i = 0; i < m_buttons.size(); ++i) {
        MyToolButton *button = m_buttons.at(i);
        connect(button, SIGNAL(mouseEnter()), this, SLOT(onToolButtonMouseEnter()));
    }

    //屏蔽大于机型通道数的布局
    //3536g辅屏屏蔽64通道
    int channelCount = qMsNvr->maxChannel();
    if (channelCount < 64 || (qMsNvr->is3536g() && SCREEN_MAIN != SubControl::instance()->currentScreen())) {
        ui->toolButton_layout64->hide();
    }
    if (channelCount < 32) {
        ui->toolButton_layout32->hide();
    }
    if (channelCount < 25) {
        ui->toolButton_layout25->hide();
    }
    if (channelCount < 16) {
        ui->toolButton_layout16->hide();
    }
    if (channelCount < 12) {
        ui->toolButton_layout12->hide();
    }
    if (channelCount < 8) {
        ui->toolButton_layout9->hide();
    }


    ui->toolButtonOnlineUpgrade->hide();
    gMessageFilter.installMessageFilter(RESPONSE_FLAG_GET_ONLINE_UPGRADE_INFO, this);
    m_upgradeThread = new UpgradeThread();
    connect(m_upgradeThread, SIGNAL(downloadFinished(int)), this, SLOT(onDownloadFinished(int)));
    connect(m_upgradeThread, SIGNAL(upgradeFinished(int)), this, SLOT(onUpgradeFinished(int)));

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimerUpdate()));
    m_timer->setInterval(2000);
}

LiveviewBottomBar::~LiveviewBottomBar()
{
    m_upgradeThread->stopThread();
    m_upgradeThread->deleteLater();
    s_self = nullptr;
    delete ui;
}

LiveviewBottomBar *LiveviewBottomBar::instance()
{
    return s_self;
}

void LiveviewBottomBar::onLayoutModeChanged(const CustomLayoutKey &key)
{
    if (key.screen() == SubControl::instance()->currentScreen()) {
        m_irregularLayout->setCurrentLayoutButton(key);
        setCurrentLayoutButton(key);

        //
        if (qMsNvr->is5016()) {

            if (LiveViewTarget::instance() && LiveViewTarget::instance()->isTargetEnable()) {
                m_irregularLayout->setLayout14Visible(false);
                ui->toolButton_layout16->setVisible(false);
            } else {
                m_irregularLayout->setLayout14Visible(true);
                ui->toolButton_layout16->setVisible(true);
            }
        }
    }
}

void LiveviewBottomBar::updateTargetState()
{
    if (LiveViewTarget::instance()) {
        if (LiveViewTarget::instance()->isTargetEnable()) {
            ui->toolButton_target->setChecked(true);
        } else {
            ui->toolButton_target->setChecked(false);
        }
    }
}

void LiveviewBottomBar::updateDisturbState(bool checked)
{
    if (checked) {
        ui->toolButton_disturb->setIcon(QIcon(":/bottombar/bottombar/disturbing_checked.png"));
    } else {
        ui->toolButton_disturb->setIcon(QIcon(":/bottombar/bottombar/disturbing.png"));
    }
}

void LiveviewBottomBar::updateOccupancyState()
{
    if (LiveViewOccupancyManager::instance()->isOccupancyMode()) {
        ui->toolButtonOccupancy->setChecked(true);
    } else {
        ui->toolButtonOccupancy->setChecked(false);
    }
}

void LiveviewBottomBar::updateUpgradedState()
{
    if (!gMsUser.checkBasicPermission(PERM_MODE_SETTINGS, PERM_SETTINGS_NONE) || !canBeUpgraded) {
        ui->toolButtonOnlineUpgrade->hide();
    } else {
        ui->toolButtonOnlineUpgrade->show();
    }
}

QWidget *LiveviewBottomBar::test_randomLayoutButton()
{
    QList<QAbstractButton *> buttonList = m_layoutButtonGroup->buttons();
    int index = qrand() % buttonList.size();
    return buttonList.at(index);
}

void LiveviewBottomBar::getExceptionStatus()
{
    sendMessage(REQUEST_FLAG_GET_EXCEPTION_STATUS, nullptr, 0);
}

void LiveviewBottomBar::dealMessage(MessageReceive *message)
{
    if (message->isAccepted()) {
        return;
    }

    switch (message->type()) {
    case RESPONSE_FLAG_INFOMATION_SHOW:
        ON_RESPONSE_FLAG_INFOMATION_SHOW(message);
        break;
    }
}

void LiveviewBottomBar::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_EXCEPTION_STATUS:
        ON_RESPONSE_FLAG_INFOMATION_SHOW(message);
        break;
    case RESPONSE_FLAG_SET_ALL_RECORD:
        ON_RESPONSE_FLAG_SET_ALL_RECORD(message);
        break;
    }
}

void LiveviewBottomBar::filterMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_ONLINE_UPGRADE_INFO:
        ON_RESPONSE_FLAG_GET_ONLINE_UPGRADE_INFO(message);
        break;
    default:
        break;
    }
}

void LiveviewBottomBar::ON_RESPONSE_FLAG_INFOMATION_SHOW(MessageReceive *message)
{
    if (!message->data) {
        qMsWarning() << "data is null";
        return;
    }
    int showinfo = *((int *)message->data);
    switch (showinfo) {
    case 0:
        ui->toolButton_event->setVisible(false);
        ui->toolButton_event->stopFlash();
        break;
    case 1:
        ui->toolButton_event->setVisible(true);
        ui->toolButton_event->startFlash();
        BottomBar::instance()->setLocked(true);
        break;
    }
}

void LiveviewBottomBar::ON_RESPONSE_FLAG_SET_ALL_RECORD(MessageReceive *message)
{
    Q_UNUSED(message)

    //GlobalWaitting:://closeWait();
}

void LiveviewBottomBar::ON_RESPONSE_FLAG_GET_ONLINE_UPGRADE_INFO(MessageReceive *message)
{
    if (!message->data) {
        canBeUpgraded = false;
        qMsWarning() << "check nvr data is null.";
        return;
    }
    memset(&m_online_upgrade, 0, sizeof(struct resp_check_online_upgrade));
    memcpy(&m_online_upgrade, static_cast<struct resp_check_online_upgrade *>(message->data), sizeof(struct resp_check_online_upgrade));
    QString Description = m_online_upgrade.pDescription;
    int upgrade_state = m_online_upgrade.state;
    QString url = m_online_upgrade.pUrl;
    QString version = m_online_upgrade.pSoftversion;
    qDebug() << QString("check nvr:latest_ver:[%1], state:[%2], url:[%3], pDescription:[%4], size:[%5]")
                    .arg(m_online_upgrade.pSoftversion)
                    .arg(m_online_upgrade.state)
                    .arg(m_online_upgrade.pUrl)
                    .arg(m_online_upgrade.pDescription)
                    .arg(m_online_upgrade.fileSize);
    if (upgrade_state && !version.isEmpty() && !url.isEmpty() && version != qMsNvr->softwareVersion()) {
        canBeUpgraded = true;
    }
    updateUpgradedState();
}

void LiveviewBottomBar::setCurrentLayoutButton(const CustomLayoutKey &key)
{
    QList<QAbstractButton *> buttonList = m_layoutButtonGroup->buttons();
    for (int i = 0; i < buttonList.size(); ++i) {
        MyIconToolButton *button = static_cast<MyIconToolButton *>(buttonList.at(i));
        button->setChecked(false);
    }
    //
    if (key.type() == CustomLayoutKey::DefaultType && m_defaultNormalLayoutNames.contains(key.name())) {
        ui->toolButton_irregularLayout->setIcon(QIcon(":/bottombar/bottombar/arrow.png"));
        //
        for (int i = 0; i < buttonList.size(); ++i) {
            MyIconToolButton *button = static_cast<MyIconToolButton *>(buttonList.at(i));
            const QString &text = button->data(Qt::UserRole).toString();
            if (text == key.name()) {
                button->setChecked(true);
                break;
            }
        }
    } else if (key.type() == CustomLayoutKey::CustomType) {
        ui->toolButton_irregularLayout->setIcon(QIcon(":/bottombar/bottombar/arrow_set.png"));
    } else {
        ui->toolButton_irregularLayout->setIcon(QIcon(":/bottombar/bottombar/arrow_set.png"));
    }
}

int LiveviewBottomBar::screenRight()
{
    return qApp->desktop()->geometry().right();
}

void LiveviewBottomBar::showEvent(QShowEvent *event)
{
    updateUpgradedState();
    onTimerUpdate();
    m_timer->start();
    QWidget::showEvent(event);
}

void LiveviewBottomBar::hideEvent(QHideEvent *event)
{
    m_timer->stop();
    QWidget::hideEvent(event);
}

void LiveviewBottomBar::onLanguageChanged()
{
    ui->toolButton_layout1->setToolTip(GET_TEXT("LIVEVIEW/20000", "Single Screen"));
    ui->toolButton_layout4->setToolTip(GET_TEXT("LIVEVIEW/20002", "4 Screen"));
    ui->toolButton_layout9->setToolTip(GET_TEXT("LIVEVIEW/20005", "9 Screen"));
    ui->toolButton_layout12->setToolTip(GET_TEXT("LIVEVIEW/20006", "12 Screen"));
    ui->toolButton_layout16->setToolTip(GET_TEXT("LIVEVIEW/20009", "16 Screen"));
    ui->toolButton_layout25->setToolTip(GET_TEXT("LIVEVIEW/20029", "25 Screen"));
    ui->toolButton_layout32->setToolTip(GET_TEXT("LIVEVIEW/20010", "32 Screen"));
    ui->toolButton_layout64->setToolTip(GET_TEXT("LIVEVIEW/168004", "64 Screen"));

    ui->toolButton_target->setToolTip(GET_TEXT("TARGETMODE/103206", "Target Mode"));
    ui->toolButtonOccupancy->setToolTip(GET_TEXT("OCCUPANCY/74255", "Occupancy Mode"));

    ui->toolButton_event->setToolTip(GET_TEXT("EXCEPTION/54009", "Event Notification"));
    ui->toolButton_startRecord->setToolTip(GET_TEXT("LIVEVIEW/20062", "Start All Record"));
    ui->toolButton_stopRecord->setToolTip(GET_TEXT("LIVEVIEW/20061", "Stop All Record"));
    ui->toolButton_scaleMode->setToolTip(m_ratio == RATIO_VO_FULL ? GET_TEXT("LIVEVIEW/20063", "Original") : GET_TEXT("LIVEVIEW/20064", "Resize"));
    ui->toolButton_display->setToolTip(GET_TEXT("LIVEVIEW/20065", "Display Settings"));
    ui->toolButton_disturb->setToolTip(GET_TEXT("LIVEVIEW/20103", "Do Not Disturb"));
    ui->toolButton_sequence->setToolTip(GET_TEXT("LIVEVIEW/20013", "Sequence"));
    ui->toolButtonOnlineUpgrade->setToolTip(GET_TEXT("LIVEVIEW/168008", "Newer Version"));

    m_irregularLayout->onLanguageChanged();
}

void LiveviewBottomBar::onToolButtonMouseEnter()
{
    MyToolButton *sendButton = static_cast<MyToolButton *>(sender());
    for (int i = 0; i < m_buttons.size(); ++i) {
        MyToolButton *button = m_buttons.at(i);
        if (button != sendButton) {
            if (button->testAttribute(Qt::WA_UnderMouse)) {
                button->clearUnderMouse();
                button->update();
            }
        }
    }
}

void LiveviewBottomBar::onLayoutButtonGroupClicked(int id)
{
    emit setLiveviewLayoutMode(id);
}

void LiveviewBottomBar::onScreenSwitched()
{
    bool hide64Layout = qMsNvr->maxChannel() < 64 || (qMsNvr->is3536g() && SCREEN_MAIN != SubControl::instance()->currentScreen());
    ui->toolButton_layout64->setVisible(!hide64Layout);

    bool isSequencing = m_sequence->isSequencing(SubControl::instance()->currentScreen());
    if (isSequencing) {
        ui->toolButton_sequence->setIcon(QIcon(":/bottombar/bottombar/sequence_checked.png"));
    } else {
        ui->toolButton_sequence->setIcon(QIcon(":/bottombar/bottombar/sequence.png"));
    }
}

void LiveviewBottomBar::onDisplayStateChanged(bool open)
{
    qDebug() << "LiveviewBottomBar::onDisplayStateChanged, open:" << open;
    if (open) {
        ui->toolButton_display->setIcon(QIcon(":/bottombar/bottombar/displaysetting_checked.png"));
    } else {
        ui->toolButton_display->setIcon(QIcon(":/bottombar/bottombar/displaysetting.png"));
    }
}

void LiveviewBottomBar::onSequenceStateChanged(int screen, bool open)
{
    qDebug() << "LiveviewBottomBar::onSequenceStateChanged, screen:" << screen << ", open:" << open;
    if (screen == SubControl::instance()->currentScreen()) {
        if (open) {
            ui->toolButton_sequence->setIcon(QIcon(":/bottombar/bottombar/sequence_checked.png"));
        } else {
            ui->toolButton_sequence->setIcon(QIcon(":/bottombar/bottombar/sequence.png"));
        }
    }
}

void LiveviewBottomBar::on_toolButton_irregularLayout_clicked()
{
    ui->toolButton_irregularLayout->clearUnderMouse();

    QPoint p = ui->toolButton_irregularLayout->mapToGlobal(QPoint(0, 0));
    int x = p.x() + ui->toolButton_irregularLayout->width() / 2 - 50;
    int y = p.y() - m_irregularLayout->height();
    m_irregularLayout->move(QPoint(x, y));
    m_irregularLayout->show();
}

void LiveviewBottomBar::on_toolButton_target_clicked()
{
    if (LiveViewTarget::instance()) {
        if (LiveViewTarget::instance()->isTargetEnable()) {
            LiveView::instance()->onActionRegularModeClicked(true);
        } else {
            LiveView::instance()->onActionTargetModeClicked(true);
        }
    }
}

void LiveviewBottomBar::on_toolButtonOccupancy_clicked()
{
    LiveViewOccupancyManager *occupancyManager = LiveViewOccupancyManager::instance();

    if (occupancyManager->isOccupancyMode()) {
        occupancyManager->closeOccupancy(LiveViewOccupancyManager::NormalReason);
    } else {
        occupancyManager->showOccupancy(LiveViewOccupancyManager::NormalReason);
    }
    updateOccupancyState();
}

void LiveviewBottomBar::on_toolButton_event_clicked()
{
    QPoint p = ui->toolButton_event->mapToGlobal(QPoint(0, 0));
    int x = p.x() + ui->toolButton_event->width() / 2 - m_eventNotification->width() / 2;
    int y = p.y() - m_eventNotification->height();
    m_eventNotification->setPos(QPoint(x, y));
    PopupContent::instance()->setPopupWidget(m_eventNotification);
    PopupContent::instance()->showPopupWidget();
}

void LiveviewBottomBar::on_toolButton_startRecord_clicked()
{
    if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_RECORD)) {
        ui->toolButton_startRecord->clearUnderMouse();
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }

    if (MsUser::isNeedMenuAuthentication()) {
        UserLogin login(this);
        const int &result = login.execLogin();
        if (result != UserLogin::Accepted) {
            return;
        }
    }

    MessageBox::Result result = MessageBox::question(this, GET_TEXT("LIVEVIEW/20069", "Start all channels continuous record?"), GET_TEXT("LIVEVIEW/20080", "Note: This will overwrite the record schedule."));
    if (result == MessageBox::Yes) {
        struct resp_set_all_record all_record;
        memset(&all_record, 0, sizeof(struct resp_set_all_record));
        all_record.enable = 1;
        for (int i = 0; i < qMsNvr->maxChannel(); ++i) {
            all_record.channel[i] = 1;
        }
        //GlobalWaitting:://showWait();
        sendMessage(REQUEST_FLAG_SET_ALL_RECORD, &all_record, sizeof(struct resp_set_all_record));

        //
        struct log_data log;
        memset(&log, 0, sizeof(struct log_data));
        log.log_data_info.subType = SUB_OP_CONFIG_LOCAL;
        log.log_data_info.parameter_type = SUB_PARAM_RECORD_SCHED;
        snprintf(log.log_data_info.user, sizeof(log.log_data_info.user), "%s", qMsNvr->currentUserName().toStdString().c_str());
        log.log_data_info.chan_no = qMsNvr->maxChannel();

        struct op_lr_rec_start_stop_sche olrsta;
        memset(&olrsta, 0, sizeof(olrsta));
        snprintf(olrsta.user, sizeof(olrsta.user), "%s", qMsNvr->currentUserName().toStdString().c_str());
        olrsta.chanid = qMsNvr->maxChannel();
        msfs_log_pack_detail(&log, OP_REC_START_SCHE, &olrsta, sizeof(struct op_lr_rec_start_stop_user));
        MsWriteLog(log);
    }
}

void LiveviewBottomBar::on_toolButton_stopRecord_clicked()
{
    if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_RECORD)) {
        ui->toolButton_stopRecord->clearUnderMouse();
        ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
        return;
    }

    if (MsUser::isNeedMenuAuthentication()) {
        UserLogin login(this);
        const int &result = login.execLogin();
        if (result != UserLogin::Accepted) {
            return;
        }
    }

    MessageBox::Result result = MessageBox::question(this, GET_TEXT("LIVEVIEW/20068", "Stop all channels continuous record?"), GET_TEXT("LIVEVIEW/20080", "Note: This will overwrite the record schedule."));
    if (result == MessageBox::Yes) {
        struct resp_set_all_record all_record;
        memset(&all_record, 0, sizeof(struct resp_set_all_record));
        all_record.enable = 0;
        for (int i = 0; i < qMsNvr->maxChannel(); ++i) {
            all_record.channel[i] = 1;
        }
        //GlobalWaitting:://showWait();
        sendMessage(REQUEST_FLAG_SET_ALL_RECORD, &all_record, sizeof(struct resp_set_all_record));

        //
        struct log_data log;
        memset(&log, 0, sizeof(struct log_data));
        log.log_data_info.subType = SUB_OP_CONFIG_LOCAL;
        log.log_data_info.parameter_type = SUB_PARAM_RECORD_SCHED;
        snprintf(log.log_data_info.user, sizeof(log.log_data_info.user), "%s", qMsNvr->currentUserName().toStdString().c_str());
        log.log_data_info.chan_no = qMsNvr->maxChannel();

        struct op_lr_rec_start_stop_sche olrsta;
        memset(&olrsta, 0, sizeof(olrsta));
        snprintf(olrsta.user, sizeof(olrsta.user), "%s", qMsNvr->currentUserName().toStdString().c_str());
        olrsta.chanid = qMsNvr->maxChannel();
        msfs_log_pack_detail(&log, OP_REC_START_SCHE, &olrsta, sizeof(struct op_lr_rec_start_stop_user));
        MsWriteLog(log);
    }
}

void LiveviewBottomBar::on_toolButton_scaleMode_clicked()
{
    if (m_ratio == RATIO_VO_FULL) {
        m_ratio = RATIO_VO_AUTO;
        ui->toolButton_scaleMode->setToolTip(GET_TEXT("LIVEVIEW/20064", "Resize"));
        ui->toolButton_scaleMode->setIcon(QIcon(":/bottombar/bottombar/resize.png"));
    } else {
        m_ratio = RATIO_VO_FULL;
        ui->toolButton_scaleMode->setToolTip(GET_TEXT("LIVEVIEW/20063", "Original"));
        ui->toolButton_scaleMode->setIcon(QIcon(":/bottombar/bottombar/original.png"));
    }

    emit setVideoRatio((int)m_ratio);
}

void LiveviewBottomBar::on_toolButton_display_clicked()
{
    QPoint p = ui->toolButton_display->mapToGlobal(QPoint(0, 0));
    int x = p.x() + ui->toolButton_display->width() / 2 - m_displaySetting->width() / 2;
    if (x + m_displaySetting->width() > screenRight()) {
        x = screenRight() - m_displaySetting->width();
    }
    int y = p.y() - m_displaySetting->height();
    m_displaySetting->setPos(QPoint(x, y));
    PopupContent::instance()->setPopupWidget(m_displaySetting);
    PopupContent::instance()->showPopupWidget();
}

void LiveviewBottomBar::on_toolButton_disturb_clicked()
{
    QPoint p = ui->toolButton_disturb->mapToGlobal(QPoint(0, 0));
    int x = p.x() + ui->toolButton_disturb->width() / 2 - m_disturbing->width() / 2;
    if (x + m_disturbing->width() > screenRight()) {
        x = screenRight() - m_disturbing->width();
    }
    int y = p.y() - m_disturbing->height();
    m_disturbing->setPos(QPoint(x, y));
    PopupContent::instance()->setPopupWidget(m_disturbing);
    PopupContent::instance()->showPopupWidget();
}

void LiveviewBottomBar::on_toolButton_sequence_clicked()
{
    QPoint p = ui->toolButton_sequence->mapToGlobal(QPoint(0, 0));
    int x = p.x() + ui->toolButton_sequence->width() / 2 - m_sequence->width() / 2;
    if (x + m_sequence->width() > screenRight()) {
        x = screenRight() - m_sequence->width();
    }
    int y = p.y() - m_sequence->height();
    m_sequence->setPos(QPoint(x, y));
    m_sequence->updateDisplayInfo();
    PopupContent::instance()->setPopupWidget(m_sequence);
    PopupContent::instance()->showPopupWidget();
}

void LiveviewBottomBar::on_toolButtonOnlineUpgrade_clicked()
{
    quint64 maxSize = (84 << 20);
    if (static_cast<quint64>(m_online_upgrade.fileSize) > maxSize) {
        ShowMessageBox(GET_TEXT("CAMERAMAINTENANCE/38014", "Firmware file is too large."));
        return;
    }
    QString softVersion = m_online_upgrade.pSoftversion;
    int result = MessageBox::question(this, GET_TEXT("LIVEVIEW/168009", "Newer version %1 detected. Upgrade?").arg(softVersion), GET_TEXT("LIVEVIEW/168010", "Note: The device will reboot automatically after upgrading."));
    if (result == MessageBox::Yes) {
        m_upgradeThread->getOnlineUpgradeImage(m_online_upgrade.pUrl);
        //MsWaitting::showGlobalWait(LiveView::instance());
    }
}

void LiveviewBottomBar::onDownloadFinished(int result)
{
    qDebug() << "nvr onDownloadFinished" << result;
    if (result != 0) {
        //MsWaitting::closeGlobalWait();
        ShowMessageBox(GET_TEXT("UPGRADE/75028", "Download Failed."));
    } else {
        //调用升级镜像前，先REQUEST_FLAG_UPGRADE_SYSTEM_INIT这个请求
        sendMessageOnly(REQUEST_FLAG_UPGRADE_SYSTEM_INIT, nullptr, 0);
        m_upgradeThread->startOnlineUpgrade();
    }
}

void LiveviewBottomBar::onUpgradeFinished(int result)
{
    qDebug() << "nvr online upgrade result:" << result;
    if (result < 0) {
        //MsWaitting::closeGlobalWait();
        ShowMessageBox(GET_TEXT("UPGRADE/75010", "Upgrade Failed."));
    } else {
        qMsDebug() << "onUpgradeFinished success";
        qMsNvr->reboot();
        qMsApp->setAboutToReboot(true);
    }
}

void LiveviewBottomBar::onTimerUpdate()
{
    DISARMING_S disarm;
    memset(&disarm, 0, sizeof(DISARMING_S));
    read_disarming(SQLITE_FILE_NAME, &disarm);
    ui->toolButton_disturb->setEnabled(!disarm.linkageAction);
    if (disarm.linkageAction && m_disturbing->isVisible()) {
        PopupContent::instance()->closePopupWidget(BasePopup::CloseNormal);
    }
}
