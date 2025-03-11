#include "ptzbottombar.h"
#include "ui_ptzbottombar.h"
#include "MessageBox.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "centralmessage.h"
#include "msuser.h"
#include "ptz3dcontrol.h"
#include <QtDebug>
PtzBottomBar *PtzBottomBar::s_self = nullptr;

PtzBottomBar::PtzBottomBar(QWidget *parent)
    : MsWidget(parent)
    , ui(new Ui::PtzBottomBar)
{
    ui->setupUi(this);

    s_self = this;

    ui->toolButton_3d->setNormalIcon(":/ptz/ptz/3D_white.png");
    ui->toolButton_3d->setCheckedIcon(":/ptz/ptz/3D_blue.png");
    ui->toolButton_manualTrack->setNormalIcon(":/ptz/ptz/manual_track_white.png");
    ui->toolButton_manualTrack->setCheckedIcon(":/ptz/ptz/manual_track_blue.png");
    ui->toolButton_manualTrack->setDisabledIcon(":/ptz/ptz/manual_track_disable.png");
    ui->toolButton_ptz->setNormalIcon(":/ptz/ptz/ptz_white.png");
    ui->toolButton_ptz->setCheckedIcon(":/ptz/ptz/ptz_blue.png");

    ui->toolButtonOneTouchPatrol->setNormalIcon(":/ptz/ptz/1key_white.png");
    ui->toolButtonOneTouchPatrol->setCheckedIcon(":/ptz/ptz/1key_blue.png");
    ui->toolButtonDehumidifying->setNormalIcon(":/ptz/ptz/defog_white.png");
    ui->toolButtonDehumidifying->setCheckedIcon(":/ptz/ptz/defog_blue.png");
    ui->toolButtonAutoHome->setNormalIcon(":/ptz/ptz/position_white.png");
    ui->toolButtonAutoHome->setCheckedIcon(":/ptz/ptz/position_blue.png");
    ui->toolButtonWiper->setNormalIcon(":/ptz/ptz/wiper_white.png");
    ui->toolButtonWiper->setCheckedIcon(":/ptz/ptz/wiper_blue.png");

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

PtzBottomBar::~PtzBottomBar()
{
    s_self = nullptr;
    if (statusTimer) {
        statusTimer->stop();
        statusTimer->deleteLater();
        statusTimer = nullptr;
    }
    delete ui;
}

void PtzBottomBar::hideEvent(QHideEvent *)
{
    stopStatusTimer();
}

void PtzBottomBar::showEvent(QShowEvent *)
{
    onStatusTimeout();
    startStatusTimer();
}

PtzBottomBar *PtzBottomBar::instance()
{
    return s_self;
}

void PtzBottomBar::initializeData(int channel)
{
    m_channel = channel;
    if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_PTZCONTROL)) {
        ui->toolButton_3d->setChecked(false);
        on_toolButton_3d_clicked(false);
    } else {
        ui->toolButton_3d->setChecked(true);
        on_toolButton_3d_clicked(true);
    }
    ui->toolButton_manualTrack->setChecked(false);
    ui->toolButton_ptz->setChecked(true);
    on_toolButton_ptz_clicked(true);

    sendMessage(REQUEST_FLAG_GET_AUTO_TRACKING, &m_channel, sizeof(int));

    onStatusTimeout();
    startStatusTimer();
}

void PtzBottomBar::setPtzButtonChecked(bool checked)
{
    ui->toolButton_ptz->setChecked(checked);
    on_toolButton_ptz_clicked(checked);
}

void PtzBottomBar::setPtzManualTrackButtonEnabled(bool enabled)
{
    ui->toolButton_manualTrack->setEnabled(enabled);
    if (enabled) {
        ui->toolButton_manualTrack->setEnabled(true);
    } else {
        ui->toolButton_manualTrack->setEnabled(false);
    }
}

void PtzBottomBar::setPtzManualTrackButtonChecked(bool checked)
{
    ui->toolButton_manualTrack->setChecked(checked);
    //
    if (checked) {
        if (ui->toolButton_3d->isChecked()) {
            ui->toolButton_3d->setChecked(false);
            if (PTZ3DControl::instance()) {
                PTZ3DControl::instance()->set3DEnable(false);
            }
        }
    }
    ui->toolButton_manualTrack->setChecked(checked);
}

void PtzBottomBar::stopStatusTimer()
{
    if (statusTimer) {
        statusTimer->stop();
    }
}

void PtzBottomBar::startStatusTimer()
{
    if (!statusTimer) {
        statusTimer = new QTimer(this);
        connect(statusTimer, SIGNAL(timeout()), this, SLOT(onStatusTimeout()));
    }
    statusTimer->start(1000);
}

void PtzBottomBar::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_AUTO_TRACKING:
        ON_RESPONSE_FLAG_GET_AUTO_TRACKING(message);
        break;
    case RESPONSE_FLAG_GET_IPC_PTZ_PANEL_STATUS:
        ON_RESPONSE_FLAG_GET_IPC_PTZ_PANEL_STATUS(message);
        break;
    }
}

void PtzBottomBar::ON_RESPONSE_FLAG_GET_AUTO_TRACKING(MessageReceive *message)
{
    qDebug() << "====PtzBottomBar::ON_RESPONSE_FLAG_GET_AUTO_TRACKING====";
    ms_auto_tracking *auto_tracking = (ms_auto_tracking *)message->data;
    memset(&m_auto_tracking, 0, sizeof(m_auto_tracking));
    if (auto_tracking) {
        memcpy(&m_auto_tracking, auto_tracking, sizeof(m_auto_tracking));

        qDebug() << "----chanid:" << auto_tracking->chanid;
        qDebug() << "----enable:" << auto_tracking->enable;
        qDebug() << "----show:" << auto_tracking->show;
        qDebug() << "----manual_tracking:" << auto_tracking->manual_tracking;

        setPtzManualTrackButtonEnabled(auto_tracking->enable);
        setPtzManualTrackButtonChecked(auto_tracking->manual_tracking);
    }
}

void PtzBottomBar::ON_RESPONSE_FLAG_GET_IPC_PTZ_PANEL_STATUS(MessageReceive *message)
{
    IpcPtzPanelStatus *status = static_cast<IpcPtzPanelStatus *>(message->data);
    bool supportJson = false;
    if (status) {
        supportJson = true;
    }
    ui->toolButtonOneTouchPatrol->setVisible(supportJson);
    ui->toolButtonDehumidifying->setVisible(supportJson && status->defogSupport);
    ui->toolButtonAutoHome->setVisible(supportJson);
    ui->toolButtonAutoIris->setVisible(supportJson);
    ui->toolButtonWiper->setVisible(supportJson && status->wiperSupport);
    if (supportJson) {
        ui->toolButtonOneTouchPatrol->setChecked(status->onePatrol);
        ui->toolButtonDehumidifying->setChecked(status->manualDefog);
        ui->toolButtonAutoHome->setChecked(status->autoHome);
        ui->toolButtonAutoIris->setChecked(status->irisMode);
        if (status->irisMode) {
            ui->toolButtonAutoIris->setNormalIcon(":/ptz/ptz/autoiris_blue.png");
            ui->toolButtonAutoIris->setCheckedIcon(":/ptz/ptz/autoiris_blue.png");
        } else {
            ui->toolButtonAutoIris->setNormalIcon(":/ptz/ptz/autoiris_white.png");
            ui->toolButtonAutoIris->setCheckedIcon(":/ptz/ptz/autoiris_white.png");
        }
        ui->toolButtonWiper->setChecked(status->manualWiper);
    }
}

void PtzBottomBar::onLanguageChanged()
{
    ui->toolButton_3d->setToolTip(GET_TEXT("LIVEVIEW/20101", "3D Positioning"));
    ui->toolButton_3d->setIsNeedToolTip(true);
    ui->toolButton_ptz->setToolTip(GET_TEXT("LIVEVIEW/168000", "Operators"));
    ui->toolButton_ptz->setIsNeedToolTip(true);
    ui->toolButton_manualTrack->setToolTip(GET_TEXT("PTZDIALOG/21029", "PTZ Manual Tracking"));
    ui->toolButton_manualTrack->setIsNeedToolTip(true);
    ui->toolButton_closePtz3D->setToolTip(GET_TEXT("PLAYBACK/80032", "Exit"));

    ui->toolButtonOneTouchPatrol->setToolTip(GET_TEXT("LIVEVIEW/168001", "One-touch patrol"));
    ui->toolButtonOneTouchPatrol->setIsNeedToolTip(true);
    ui->toolButtonDehumidifying->setToolTip(GET_TEXT("LIVEVIEW/168002", "Dehumidifying"));
    ui->toolButtonDehumidifying->setIsNeedToolTip(true);
    ui->toolButtonAutoHome->setToolTip(GET_TEXT("PTZCONFIG/166000", "Auto Home"));
    ui->toolButtonAutoHome->setIsNeedToolTip(true);
    ui->toolButtonAutoIris->setToolTip(GET_TEXT("LIVEVIEW/168003", "Auto Iris"));
    ui->toolButtonAutoIris->setIsNeedToolTip(true);
    ui->toolButtonWiper->setToolTip(GET_TEXT("PTZCONFIG/166005", "Wiper"));
    ui->toolButtonWiper->setIsNeedToolTip(true);
}

void PtzBottomBar::onStatusTimeout()
{
    sendMessage(REQUEST_FLAG_GET_IPC_PTZ_PANEL_STATUS, &m_channel, sizeof(int));
}

void PtzBottomBar::on_toolButton_3d_clicked(bool checked)
{
    if (checked) {
        if (!gMsUser.checkBasicPermission(PERM_MODE_LIVE, PERM_LIVE_PTZCONTROL)) {
            ui->toolButton_3d->clearUnderMouse();
            ui->toolButton_3d->setChecked(false);
            ShowMessageBox(GET_TEXT("COMMON/1011", "Insufficient User Permissions."));
            return;
        }
        if (ui->toolButton_manualTrack->isChecked()) {
            ui->toolButton_manualTrack->setChecked(false);
            PTZ3DControl::instance()->setManualTrackEnabled(false);
        }
    }
    ui->toolButton_3d->setChecked(checked);
    if (PTZ3DControl::instance()) {
        PTZ3DControl::instance()->set3DEnable(checked);
    }
}

void PtzBottomBar::on_toolButton_manualTrack_clicked(bool checked)
{
    if (checked) {
        if (!m_auto_tracking.enable) {
            ui->toolButton_manualTrack->setChecked(false);
            ShowMessageBox(GET_TEXT("PTZCONFIG/36061", "Please enable Auto Tracking first."));
            return;
        }
        //
        if (ui->toolButton_3d->isChecked()) {
            ui->toolButton_3d->setChecked(false);
            if (PTZ3DControl::instance()) {
                PTZ3DControl::instance()->set3DEnable(false);
            }
        }
    }
    ui->toolButton_manualTrack->setChecked(checked);
    PTZ3DControl::instance()->setManualTrackEnabled(checked);
}

void PtzBottomBar::on_toolButton_ptz_clicked(bool checked)
{
    ui->toolButton_ptz->setChecked(checked);
    PTZ3DControl::instance()->setPtzEnable(checked);
}

void PtzBottomBar::on_toolButton_closePtz3D_clicked()
{
    PTZ3DControl::instance()->closePtz3D(PTZ3DControl::ExitForRightButton);
}

void PtzBottomBar::on_toolButtonOneTouchPatrol_clicked()
{
    PTZ3DControl::instance()->ptzPanelControl(IPC_PTZ_CONTORL_ONE_TOUCH_PATROL, ui->toolButtonOneTouchPatrol->isChecked());
}

void PtzBottomBar::on_toolButtonDehumidifying_clicked()
{
    PTZ3DControl::instance()->ptzPanelControl(IPC_PTZ_CONTORL_DEHUMIDIFYING, ui->toolButtonDehumidifying->isChecked());
}

void PtzBottomBar::on_toolButtonAutoHome_clicked()
{
    PTZ3DControl::instance()->ptzPanelControl(IPC_PTZ_CONTORL_AUTO_HOME, ui->toolButtonAutoHome->isChecked());
}

void PtzBottomBar::on_toolButtonWiper_clicked()
{
    PTZ3DControl::instance()->ptzPanelControl(IPC_PTZ_CONTORL_WIPER, ui->toolButtonWiper->isChecked());
}
