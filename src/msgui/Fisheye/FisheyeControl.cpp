#include "FisheyeControl.h"
#include "ui_FisheyeControl.h"
#include "FisheyeBottomBar.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "ptzcontrol.h"
#include <QMouseEvent>

FisheyeControl *FisheyeControl::s_fisheyeControl = nullptr;

FisheyeControl::FisheyeControl(QWidget *parent)
    : BaseDialog(parent)
    , ui(new Ui::FisheyeControl)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Widget);
    s_fisheyeControl = this;
    installEventFilter(this);

    connect(ui->fisheyeLayout->fisheyeBar(), SIGNAL(buttonClicked(int)), this, SLOT(onFisheyeButtonClicked(int)));
}

FisheyeControl::~FisheyeControl()
{
    s_fisheyeControl = nullptr;
    m_ptzControl = nullptr;
    delete ui;
}

FisheyeControl *FisheyeControl::instance()
{
    return s_fisheyeControl;
}

void FisheyeControl::showFisheyeBar()
{
    ui->fisheyeLayout->setFisheyeBarVisible(true);
}

void FisheyeControl::closePtz3DForSwitchScreen()
{
    done(2);
}

int FisheyeControl::currentFisheyeStream()
{
    return ui->fisheyeLayout->currentFisheyeStream();
}

int FisheyeControl::currentFisheyeMode()
{
    return ui->fisheyeLayout->layoutMode();
}

int FisheyeControl::currentFisheyeMount() const
{
    return m_fishmode_param.fishmount;
}

int FisheyeControl::currentFisheyeDisplay() const
{
    return m_fishmode_param.fishdisplay;
}

bool FisheyeControl::isTrackEnable() const
{
    return m_auto_tracking.enable;
}

bool FisheyeControl::isTrackShow() const
{
    return m_auto_tracking.show;
}

void FisheyeControl::setPtzControl(PtzControl *ptz)
{
    m_ptzControl = ptz;
}

void FisheyeControl::showFisheyeControl(int channel)
{
    m_channel = channel;
    ui->fisheyeLayout->setFisheyeBarVisible(false);
    //鱼眼信息
    sendMessage(REQUEST_FLAG_GET_FISHEYE_MODE, (void *)&channel, sizeof(int));
}

int FisheyeControl::currentChannel() const
{
    return m_channel;
}

void FisheyeControl::setFisheyeMode(int mount, int display)
{
    if (mount >= 0) {
        m_fishmode_param.fishmount = mount;
    }
    if (display >= 0) {
        m_fishmode_param.fishdisplay = display;
        //
        ui->fisheyeLayout->setLayoutMode(display);
    }

    struct resp_fishmode_param fishmode_param;
    memset(&fishmode_param, 0, sizeof(resp_fishmode_param));
    if (m_fishmode_param.fishmount == MsQt::FISHEYE_INSTALL_WALL && m_fishmode_param.fishdisplay == MsQt::FISHEYE_DISPLAY_2P) {
        m_fishmode_param.fishdisplay = MsQt::FISHEYE_DISPLAY_1O;
    }
    fishmode_param.chnid = m_channel;
    fishmode_param.fishcorrect = m_fishmode_param.fishcorrect;
    fishmode_param.fishmount = m_fishmode_param.fishmount;
    fishmode_param.fishdisplay = m_fishmode_param.fishdisplay;
    sendMessage(REQUEST_FLAG_SET_FISHEYE_MODE, (void *)&fishmode_param, sizeof(struct resp_fishmode_param));
}

void FisheyeControl::setFisheyeAutoTrack(bool enable)
{
    m_auto_tracking.enable = enable;
    m_auto_tracking.show = enable;
    qDebug() << "====FisheyeControl::setFisheyeAutoTrack====";
    qDebug() << "----chnid:" << m_auto_tracking.chanid;
    qDebug() << "----enable:" << enable;
    qDebug() << "----show:" << enable;
    sendMessage(REQUEST_FLAG_SET_AUTO_TRACKING, &m_auto_tracking, sizeof(ms_auto_tracking));

    int display = currentFisheyeDisplay();
    switch (display) {
    case MsQt::FISHEYE_DISPLAY_1O3R:
        LiveView::instance()->setFisheyeTrackEnabled(m_auto_tracking.chanid, enable);
        break;
    default:
        LiveView::instance()->setFisheyeTrackEnabled(m_auto_tracking.chanid, false);
        break;
    }
}

void FisheyeControl::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_FISHEYE_MODE:
        ON_RESPONSE_FLAG_GET_FISHEYE_MODE(message);
        break;
    case RESPONSE_FLAG_GET_AUTO_TRACKING:
        ON_RESPONSE_FLAG_GET_AUTO_TRACKING(message);
        break;
    default:
        break;
    }
}

void FisheyeControl::ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message)
{
    memset(&m_fishmode_param, 0, sizeof(resp_fishmode_param));

    struct resp_fishmode_param *fishmode_param = (struct resp_fishmode_param *)message->data;
    if (!fishmode_param) {
        qMsWarning() << message;
    } else {
        if (fishmode_param->chnid == m_channel) {
            memcpy(&m_fishmode_param, fishmode_param, sizeof(resp_fishmode_param));
            ui->fisheyeLayout->setLayoutMode(fishmode_param->fishdisplay);
        }
    }

    //
    sendMessage(REQUEST_FLAG_GET_AUTO_TRACKING, (void *)&m_channel, sizeof(int));
}

void FisheyeControl::ON_RESPONSE_FLAG_GET_AUTO_TRACKING(MessageReceive *message)
{
    ms_auto_tracking *auto_tracking = (ms_auto_tracking *)message->data;
    memset(&m_auto_tracking, 0, sizeof(m_auto_tracking));
    if (auto_tracking) {
        memcpy(&m_auto_tracking, auto_tracking, sizeof(m_auto_tracking));
    }

    //
    FisheyeBottomBar::instance()->updateFisheyeButtonState();
}

bool FisheyeControl::eventFilter(QObject *obj, QEvent *evt)
{
    switch (evt->type()) {
    case QEvent::ContextMenu:
        return true;
        break;
    default:
        break;
    }

    return BaseDialog::eventFilter(obj, evt);
}

void FisheyeControl::showEvent(QShowEvent *)
{
    showMaximized();
}

void FisheyeControl::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        m_pressedPoint = event->pos();
    } else if (event->button() == Qt::RightButton) {
        close();
    }
}

void FisheyeControl::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    m_pressed = false;
}

void FisheyeControl::onFisheyeButtonClicked(int mode)
{
    switch (mode) {
    case FisheyeBar::Mode_PTZ: {
        const QRect &rc = ui->fisheyeLayout->selectedGlobalRect();
        int result = m_ptzControl->waitForGetPTZSupport(m_channel, rc, PtzControl::MODE_FISHEYE);
        switch (result) {
        case PtzControl::STATE_NONSUPPORT:
            ShowMessageBox(GET_TEXT("PTZCONFIG/36013", "This channel does not support this function."));
            return;
        case PtzControl::STATE_PTZ_NORMAL:
            m_ptzControl->setBundleFisheyeStream(FisheyeLayout::instance()->currentFisheyeStream());
            m_ptzControl->execPtz();
            showFisheyeBar();
            break;
        case PtzControl::STATE_PTZ_3D:
            break;
        }
        break;
    }
    default:
        break;
    }
}
