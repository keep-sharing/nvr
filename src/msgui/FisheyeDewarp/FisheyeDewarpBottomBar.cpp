#include "FisheyeDewarpBottomBar.h"
#include "ui_FisheyeDewarpBottomBar.h"
#include "centralmessage.h"
#include "globalwaitting.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include <QTimer>
#include <QtDebug>

FisheyeDewarpBottomBar *FisheyeDewarpBottomBar::s_self;

FisheyeDewarpBottomBar::FisheyeDewarpBottomBar(QWidget *parent)
    : MsWidget(parent)
    , ui(new Ui::FisheyeDewarpBottomBar)
{
    ui->setupUi(this);

    s_self = this;

    ui->toolButton_fish_panel->setStateIcon(MyStateToolButton::StateNormal, ":/fisheye2/fisheye2/fisheye_client-side_dewarping_white.png");
    ui->toolButton_fish_panel->setStateIcon(MyStateToolButton::StateChecked, ":/fisheye2/fisheye2/fisheye_client-side_dewarping_blue.png");
    ui->toolButton_fish_panel->setStateIcon(MyStateToolButton::StateDisabled, ":/fisheye2/fisheye2/fisheye_client-side_dewarping_hover.png");

    ui->toolButton_fish_track->setNormalIcon(":/fisheye2/fisheye2/fish_track_off_white.png");
    ui->toolButton_fish_track->setCheckedIcon(":/fisheye2/fisheye2/fish_track_on_blue.png");
    ui->toolButton_fish_track->setDisabledIcon(":/fisheye2/fisheye2/fish_track_off_hover.png");

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

FisheyeDewarpBottomBar::~FisheyeDewarpBottomBar()
{
    s_self = nullptr;
    delete ui;
}

FisheyeDewarpBottomBar *FisheyeDewarpBottomBar::instance()
{
    return s_self;
}

void FisheyeDewarpBottomBar::initializeData(int channel)
{
    m_channel = channel;
    ui->toolButton_fish_panel->setState(MyStateToolButton::StateNormal);

    QMetaObject::invokeMethod(this, "onInitializeData", Qt::QueuedConnection);
}

void FisheyeDewarpBottomBar::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_FISHEYE_MODE:
        ON_RESPONSE_FLAG_GET_FISHEYE_MODE(message);
        break;
    case RESPONSE_FLAG_GET_AUTO_TRACKING:
        ON_RESPONSE_FLAG_GET_AUTO_TRACKING(message);
        break;
    }
}

void FisheyeDewarpBottomBar::ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message)
{
    struct resp_fishmode_param *fishmode_param = (struct resp_fishmode_param *)message->data;
    memset(&m_fishmode_param, 0, sizeof(m_fishmode_param));
    if (fishmode_param) {
        memcpy(&m_fishmode_param, fishmode_param, sizeof(m_fishmode_param));
    }

    m_eventLoop.exit();
}

void FisheyeDewarpBottomBar::ON_RESPONSE_FLAG_GET_AUTO_TRACKING(MessageReceive *message)
{
    ms_auto_tracking *auto_tracking = (ms_auto_tracking *)message->data;
    memset(&m_auto_tracking, 0, sizeof(m_auto_tracking));
    if (auto_tracking) {
        memcpy(&m_auto_tracking, auto_tracking, sizeof(m_auto_tracking));
    }

    m_eventLoop.exit();
}

void FisheyeDewarpBottomBar::onFisheyePanelButtonStateChanged(int state)
{
    switch (state) {
    case 0:
        ui->toolButton_fish_panel->setState(MyStateToolButton::StateNormal);
        break;
    case 1:
        ui->toolButton_fish_panel->setState(MyStateToolButton::StateChecked);
        break;
    case -1:
        break;
    }
}

void FisheyeDewarpBottomBar::onLanguageChanged()
{
    ui->toolButton_fish_panel->setToolTip(GET_TEXT("FISHEYE/12007", "NVR Dewarping"));
    ui->toolButton_fish_track->setToolTip(GET_TEXT("PTZDIALOG/21028", "Fisheye Auto Tracking"));
}

void FisheyeDewarpBottomBar::onInitializeData()
{
    //GlobalWaitting:://showWait();
    //鱼眼信息
    sendMessage(REQUEST_FLAG_GET_FISHEYE_MODE, &m_channel, sizeof(int));
    //m_eventLoop.exec();
    sendMessage(REQUEST_FLAG_GET_AUTO_TRACKING, &m_channel, sizeof(int));
    //m_eventLoop.exec();
    //
    //GlobalWaitting:://closeWait();

    //仅针对鱼眼（硬解+顶装模式+带R模式）该按钮可点
    m_fisheyeTrackButtonEnable = true;
    if (m_fishmode_param.fishmount != MsQt::FISHEYE_INSTALL_CEILING) {
        m_fisheyeTrackButtonEnable = false;
    }
    switch (m_fishmode_param.fishdisplay) {
    case MsQt::FISHEYE_DISPLAY_1O3R:
    case MsQt::FISHEYE_DISPLAY_1P3R:
    case MsQt::FISHEYE_DISPLAY_1O1P3R:
    case MsQt::FISHEYE_DISPLAY_4R:
        break;
    default:
        m_fisheyeTrackButtonEnable = false;
        break;
    }

    ui->toolButton_fish_track->setChecked(m_fisheyeTrackButtonEnable && m_auto_tracking.enable && m_auto_tracking.show);

    if (ui->toolButton_fish_track->isChecked()) {
        on_toolButton_fish_track_clicked(true);
    }

    QTimer::singleShot(500, this, SLOT(on_toolButton_fish_panel_clicked()));
}

void FisheyeDewarpBottomBar::on_toolButton_fish_track_clicked(bool checked)
{
    if (!m_fisheyeTrackButtonEnable) {
        ui->toolButton_fish_track->clearUnderMouse();
        ui->toolButton_fish_track->setChecked(false);
        ShowMessageBox(GET_TEXT("FISHEYE/12009", "Only available when on-board dewarping mode with R and on-board installation mode is ceiling."));
        return;
    }

    qDebug() << "====FisheyeDewarpBottomBar::on_toolButton_fish_track_clicked====";
    qDebug() << "----chnid:" << m_auto_tracking.chanid;
    qDebug() << "----enable:" << checked;
    qDebug() << "----show:" << checked;

    m_auto_tracking.enable = checked;
    m_auto_tracking.show = checked;
    sendMessage(REQUEST_FLAG_SET_AUTO_TRACKING, &m_auto_tracking, sizeof(ms_auto_tracking));

    //
    LiveView::instance()->setFisheyeTrackEnabled(m_auto_tracking.chanid, checked);

    //
    emit closeFisheyePanel();
}

void FisheyeDewarpBottomBar::on_toolButton_fish_panel_clicked()
{
    //避免进入鱼眼模式后快速退出
    if (isVisible()) {
        QPoint p = ui->toolButton_fish_panel->mapToGlobal(QPoint(0, 0));
        int x = p.x() + ui->toolButton_fish_panel->width() / 2;
        emit fisheyePanelButtonClicked(x, p.y());
    }
}

void FisheyeDewarpBottomBar::on_toolButton_closeFisheye_clicked()
{
    emit closeButtonClicked();
}
