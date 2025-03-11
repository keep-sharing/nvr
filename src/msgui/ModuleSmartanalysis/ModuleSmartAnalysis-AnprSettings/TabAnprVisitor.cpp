#include "TabAnprVisitor.h"
#include "ui_TabAnprVisitor.h"
#include "ActionAnpr.h"
#include "EffectiveTimeAnpr.h"
#include "EventLoop.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "MessageBox.h"
#include "MsLanguage.h"
#include "MsWaitting.h"

TabAnprVisitor::TabAnprVisitor(QWidget *parent)
    : AnprBasePage(parent)
    , ui(new Ui::AnprVisitorPage)
{
    ui->setupUi(this);

    onLanguageChanged();
}

TabAnprVisitor::~TabAnprVisitor()
{
    delete ui;
}

void TabAnprVisitor::initializeData(int channel)
{
    m_currentChannel = channel;
    if (isChannelConnected()) {
        ui->widgetMessage->hide();
    } else {
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
        showEnable(false);
        return;
    }
    //showWait();
    sendMessage(REQUEST_FLAG_GET_LPR_SUPPORT, &m_currentChannel, sizeof(m_currentChannel));
    //m_eventLoop.exec();

    if (!isAnprSupport()) {
        return;
    }
    if (!m_anprEvent) {
        m_anprEvent = new smart_event;
    }
    memset(m_anprEvent, 0, sizeof(smart_event));
    read_anpr_event(SQLITE_FILE_NAME, m_anprEvent, m_currentChannel, ANPR_VISTOR);

    ui->checkBox_enable->setChecked(m_anprEvent->enable);

    if (m_action) {
        m_action->clearCache();
    }
    if (m_effectiveTime) {
        m_effectiveTime->clearCache();
    }
}

void TabAnprVisitor::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_LPR_SUPPORT:
        ON_RESPONSE_FLAG_GET_LPR_SUPPORT(message);
        break;
    case RESPONSE_FLAG_SET_ANPR_EVENT:
        ON_RESPONSE_FLAG_SET_ANPR_EVENT(message);
        break;
    }
}

void TabAnprVisitor::ON_RESPONSE_FLAG_GET_LPR_SUPPORT(MessageReceive *message)
{
    ms_lpr_support_info *lpr_support_info = (ms_lpr_support_info *)message->data;
    if (lpr_support_info) {
        qDebug() << QString("AnprSetting::ON_RESPONSE_FLAG_GET_LPR_SUPPORT, channel: %1, support: %2, version: %3")
                        .arg(lpr_support_info->chanid)
                        .arg(lpr_support_info->vca_support)
                        .arg(lpr_support_info->vca_version);
    } else {
        qWarning() << QString("AnprSetting::ON_RESPONSE_FLAG_GET_LPR_SUPPORT, data is null.");
    }

    AnprBasePage::setAnprSupportInfo(lpr_support_info);

    //closeWait();
    if (isAnprSupport()) {
        showEnable(true);
        ui->widgetMessage->hide();
    } else {
        showEnable(false);
        if (isChannelConnected()) {
            ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
        }
    }
    m_eventLoop.exit();
}

void TabAnprVisitor::ON_RESPONSE_FLAG_SET_ANPR_EVENT(MessageReceive *message)
{
    Q_UNUSED(message)
    gEventLoopExit(0);
}

void TabAnprVisitor::showEnable(bool enable)
{
    ui->checkBox_enable->setEnabled(enable);
    on_checkBox_enable_stateChanged(enable);
    ui->pushButton_apply->setEnabled(enable);
}

void TabAnprVisitor::onLanguageChanged()
{
    ui->label_enable->setText(GET_TEXT("ANPR/103009", "Visitor Mode"));
    ui->label_effectiveTime->setText(GET_TEXT("SMARTEVENT/55015", "Effective Time"));
    ui->label_action->setText(GET_TEXT("VIDEOLOSS/50002", "Action"));

    ui->checkBox_enable->setText(GET_TEXT("COMMON/1009", "Enable"));
    ui->pushButton_effectiveTime->setText(GET_TEXT("ANPR/103031", "Edit"));
    ui->pushButton_action->setText(GET_TEXT("ANPR/103031", "Edit"));
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));
}

void TabAnprVisitor::on_pushButton_effectiveTime_clicked()
{
    if (!m_effectiveTime) {
        m_effectiveTime = new EffectiveTimeAnpr(this);
    }
    m_effectiveTime->showEffectiveTime(m_currentChannel, ANPR_VISTOR);
    m_effectiveTime->show();
}

void TabAnprVisitor::on_pushButton_action_clicked()
{
    if (!m_action) {
        m_action = new ActionAnpr(this);
    }
    m_action->showAction(m_currentChannel, ANPR_VISTOR);
}

void TabAnprVisitor::on_pushButton_apply_clicked()
{
    MsWaittingContainer waitting(MsWaitting::instance());

    if (m_effectiveTime) {
        m_effectiveTime->saveEffectiveTime(ANPR_VISTOR);
    }
    if (m_action) {
        m_action->saveAction();
    }

    read_anpr_event(SQLITE_FILE_NAME, m_anprEvent, m_currentChannel, ANPR_VISTOR);
    m_anprEvent->enable = ui->checkBox_enable->isChecked();
    write_anpr_event(SQLITE_FILE_NAME, m_anprEvent, ANPR_VISTOR);

    req_anpr_event anpr_event;
    anpr_event.chnid = m_currentChannel;
    anpr_event.modeType = ANPR_VISTOR;
    Q_UNUSED(anpr_event)
    gEventLoopExec();
}

void TabAnprVisitor::on_pushButton_back_clicked()
{
    back();
}

void TabAnprVisitor::on_checkBox_enable_stateChanged(int arg1)
{
    Q_UNUSED(arg1);
    bool isEnable = ui->checkBox_enable->isEnabled() && ui->checkBox_enable->isChecked();
    ui->pushButton_action->setEnabled(isEnable);
    ui->pushButton_effectiveTime->setEnabled(isEnable);
}
