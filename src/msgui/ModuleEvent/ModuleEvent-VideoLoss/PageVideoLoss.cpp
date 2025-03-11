#include "PageVideoLoss.h"
#include "ui_PageVideoLoss.h"
#include "ActionVideoLoss.h"
#include "centralmessage.h"
#include "channelcopydialog.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include <QtDebug>

PageVideoLoss::PageVideoLoss(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::PageVideoLoss)
{
    ui->setupUi(this);
    ui->buttonGroup_channel->setCount(qMsNvr->maxChannel());
    connect(ui->buttonGroup_channel, SIGNAL(buttonClicked(int)), this, SLOT(onButtonGroupClicked(int)));

    onLanguageChanged();
}

PageVideoLoss::~PageVideoLoss()
{
    if (m_videoloss) {
        delete m_videoloss;
    }
    if (m_videolossScheduleAudible) {
        delete m_videolossScheduleAudible;
    }
    if (m_videolossScheduleEmail) {
        delete m_videolossScheduleEmail;
    }
    if (m_videoloaddSchedulePTZ) {
        delete m_videoloaddSchedulePTZ;
    }
    if (m_videolossSchedulePopup) {
        delete m_videolossSchedulePopup;
    }
    if (m_ptzActionParams) {
        delete[] m_ptzActionParams;
    }

    delete ui;
}

void PageVideoLoss::initializeData()
{
    ui->buttonGroup_channel->setCurrentIndex(0);
}

void PageVideoLoss::processMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void PageVideoLoss::saveChannelSetting(int channel)
{
    Q_UNUSED(channel)
}

void PageVideoLoss::onLanguageChanged()
{
    ui->label_action->setText(GET_TEXT("VIDEOLOSS/50002", "Action"));
    ui->pushButton_actionEdit->setText(GET_TEXT("COMMON/1019", "Edit"));

    ui->pushButton_copy->setText(GET_TEXT("COMMON/1005", "Copy"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButtonApply->setText(GET_TEXT("COMMON/1003", "Apply"));
}

void PageVideoLoss::onButtonGroupClicked(int index)
{
    m_currentChannel = index;
    ui->video->playVideo(index);
    if (m_action) {
        m_action->clearCache();
    }

    if (LiveView::instance()->isChannelConnected(m_currentChannel)) {
        ui->widgetMessage->hide();
    } else {
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
        return;
    }
}

void PageVideoLoss::on_pushButton_actionEdit_clicked()
{
    if (!m_action) {
        m_action = new ActionVideoLoss(this);
    }
    m_action->showAction(m_currentChannel);
}

void PageVideoLoss::on_pushButton_copy_clicked()
{
    ChannelCopyDialog copy(this);
    copy.setCurrentChannel(m_currentChannel);
    int result = copy.exec();
    if (result == ChannelCopyDialog::Accepted) {
        QList<int> copyList = copy.checkedList(false);
        quint64 copyFlags = copy.checkedFlags(false);

        //showWait();
        //先保存当前通道
        if (m_action) {
            m_action->saveAction();
        }
        saveChannelSetting(m_currentChannel);

        //
        video_loss *currentVideoLoss = new video_loss;
        memset(currentVideoLoss, 0, sizeof(video_loss));
        read_video_lost(SQLITE_FILE_NAME, currentVideoLoss, m_currentChannel);
        copy_video_losts(SQLITE_FILE_NAME, currentVideoLoss, copyFlags);
        delete currentVideoLoss;
        //audible
        video_loss_schedule *currentActionAudible = new video_loss_schedule;
        memset(currentActionAudible, 0, sizeof(video_loss_schedule));
        read_videoloss_audible_schedule(SQLITE_FILE_NAME, currentActionAudible, m_currentChannel);
        copy_videoloss_audible_schedules(SQLITE_FILE_NAME, currentActionAudible, copyFlags);
        delete currentActionAudible;
        //email
        video_loss_schedule *currentActionEmail = new video_loss_schedule;
        memset(currentActionEmail, 0, sizeof(video_loss_schedule));
        read_videoloss_email_schedule(SQLITE_FILE_NAME, currentActionEmail, m_currentChannel);
        copy_videoloss_email_schedules(SQLITE_FILE_NAME, currentActionEmail, copyFlags);
        delete currentActionEmail;
        //event popup
        video_loss_schedule *currentActionPopup = new video_loss_schedule;
        memset(currentActionPopup, 0, sizeof(video_loss_schedule));
        read_videoloss_popup_schedule(SQLITE_FILE_NAME, currentActionPopup, m_currentChannel);
        copy_videoloss_popup_schedules(SQLITE_FILE_NAME, currentActionPopup, copyFlags);
        delete currentActionPopup;
        //ptz action schedule
        video_loss_schedule *currentActionPtz = new video_loss_schedule;
        memset(currentActionPtz, 0, sizeof(video_loss_schedule));
        read_videoloss_ptz_schedule(SQLITE_FILE_NAME, currentActionPtz, m_currentChannel);
        copy_videoloss_ptz_schedules(SQLITE_FILE_NAME, currentActionPtz, copyFlags);
        delete currentActionPtz;
        //ptz action
        ptz_action_params *ptzActionArray = new ptz_action_params[MAX_CAMERA];
        int ptzItemCount = 0;
        memset(ptzActionArray, 0x0, sizeof(ptz_action_params) * MAX_CAMERA);
        read_ptz_params(SQLITE_FILE_NAME, ptzActionArray, VIDEOLOSS, m_currentChannel, &ptzItemCount);
        copy_ptz_params_all(SQLITE_FILE_NAME, ptzActionArray, VIDEOLOSS, copyFlags);
        delete[] ptzActionArray;
        //led schedule
        SMART_SCHEDULE *scheduleLed = new SMART_SCHEDULE;
        memset(scheduleLed, 0, sizeof(SMART_SCHEDULE));
        WLED_INFO led_info_schedule;
        led_info_schedule.chnid = m_currentChannel;
        snprintf(led_info_schedule.pDbTable, sizeof(led_info_schedule.pDbTable), "%s", VDL_WLED_ESCHE);
        read_whiteled_effective_schedule(SQLITE_FILE_NAME, scheduleLed, &led_info_schedule);
        copy_whiteled_effective_schedules(SQLITE_FILE_NAME, scheduleLed, &led_info_schedule, copyFlags);
        delete scheduleLed;
        //led params
        WHITE_LED_PARAMS *ledParamsArray = new WHITE_LED_PARAMS[MAX_CAMERA];
        memset(ledParamsArray, 0, sizeof(WHITE_LED_PARAMS) * MAX_CAMERA);
        WLED_INFO led_info_params;
        led_info_params.chnid = m_currentChannel;
        snprintf(led_info_params.pDbTable, sizeof(led_info_params.pDbTable), "%s", VDL_WLED_PARAMS);
        int led_params_count = 0;
        read_whiteled_params(SQLITE_FILE_NAME, ledParamsArray, &led_info_params, &led_params_count);
        copy_whiteled_params(SQLITE_FILE_NAME, VDL_WLED_PARAMS, ledParamsArray, copyFlags);
        delete[] ledParamsArray;

        //http
        copy_http_notification_action(VIDEOLOSS, m_currentChannel, copyFlags);
        //
        for (int i = 0; i < copyList.size(); ++i) {
            int channel = copyList.at(i);
            if (channel != m_currentChannel) {
                saveChannelSetting(channel);
            }
        }
        //closeWait();
    }
}

void PageVideoLoss::on_pushButton_back_clicked()
{
    emit sig_back();
}

void PageVideoLoss::on_pushButtonApply_clicked()
{
    //showWait();
    return;
    if (m_action) {
        m_action->saveAction();
    }
    saveChannelSetting(m_currentChannel);
    //显示等待，避免频繁点击
    QEventLoop eventLoop;
    QTimer::singleShot(500, &eventLoop, SLOT(quit()));
    eventLoop.exec();
    //closeWait();
}
