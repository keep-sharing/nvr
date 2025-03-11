#include "FisheyeBottomBar.h"
#include "ui_FisheyeBottomBar.h"
#include "FisheyeControl.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "centralmessage.h"

FisheyeBottomBar *FisheyeBottomBar::s_self = nullptr;

FisheyeBottomBar::FisheyeBottomBar(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FisheyeBottomBar)
{
    ui->setupUi(this);

    s_self = this;

    //
    m_mountGroup = new QButtonGroup(this);
    m_mountGroup->addButton(ui->toolButton_ceil, MsQt::FISHEYE_INSTALL_CEILING);
    m_mountGroup->addButton(ui->toolButton_wall, MsQt::FISHEYE_INSTALL_WALL);
    m_mountGroup->addButton(ui->toolButton_flat, MsQt::FISHEYE_INSTALL_FLAT);
    connect(m_mountGroup, SIGNAL(buttonClicked(int)), this, SLOT(onMountGroupClicked(int)));
    ui->toolButton_ceil->setNormalIcon(":/fisheye2/fisheye2/Ceil_white.png");
    ui->toolButton_ceil->setCheckedIcon(":/fisheye2/fisheye2/Ceil_blue.png");
    ui->toolButton_wall->setNormalIcon(":/fisheye2/fisheye2/Wall_white.png");
    ui->toolButton_wall->setCheckedIcon(":/fisheye2/fisheye2/Wall_blue.png");
    ui->toolButton_flat->setNormalIcon(":/fisheye2/fisheye2/Flat_white.png");
    ui->toolButton_flat->setCheckedIcon(":/fisheye2/fisheye2/Flat_blue.png");

    //
    m_displayGroup = new QButtonGroup(this);
    m_displayGroup->addButton(ui->toolButton_1O, MsQt::FISHEYE_DISPLAY_1O);
    m_displayGroup->addButton(ui->toolButton_1P, MsQt::FISHEYE_DISPLAY_1P);
    m_displayGroup->addButton(ui->toolButton_2P, MsQt::FISHEYE_DISPLAY_2P);
    m_displayGroup->addButton(ui->toolButton_4R, MsQt::FISHEYE_DISPLAY_4R);
    m_displayGroup->addButton(ui->toolButton_1O3R, MsQt::FISHEYE_DISPLAY_1O3R);
    m_displayGroup->addButton(ui->toolButton_1P3R, MsQt::FISHEYE_DISPLAY_1P3R);
    connect(m_displayGroup, SIGNAL(buttonClicked(int)), this, SLOT(onDisplayGroupClicked(int)));
    ui->toolButton_1O->setNormalIcon(":/fisheye2/fisheye2/1O_white.png");
    ui->toolButton_1O->setCheckedIcon(":/fisheye2/fisheye2/1O_blue.png");
    ui->toolButton_1P->setNormalIcon(":/fisheye2/fisheye2/1P_white.png");
    ui->toolButton_1P->setCheckedIcon(":/fisheye2/fisheye2/1P_blue.png");
    ui->toolButton_2P->setNormalIcon(":/fisheye2/fisheye2/2P_white.png");
    ui->toolButton_2P->setCheckedIcon(":/fisheye2/fisheye2/2P_blue.png");
    ui->toolButton_4R->setNormalIcon(":/fisheye2/fisheye2/4R_white.png");
    ui->toolButton_4R->setCheckedIcon(":/fisheye2/fisheye2/4R_blue.png");
    ui->toolButton_1O3R->setNormalIcon(":/fisheye2/fisheye2/1O3R_white.png");
    ui->toolButton_1O3R->setCheckedIcon(":/fisheye2/fisheye2/1O3R_blue.png");
    ui->toolButton_1P3R->setNormalIcon(":/fisheye2/fisheye2/1P3R_white.png");
    ui->toolButton_1P3R->setCheckedIcon(":/fisheye2/fisheye2/1P3R_blue.png");

    ui->toolButton_fish_track->setNormalIcon(":/fisheye2/fisheye2/fish_track_off_white.png");
    ui->toolButton_fish_track->setCheckedIcon(":/fisheye2/fisheye2/fish_track_on_blue.png");
    ui->toolButton_fish_track->setDisabledIcon(":/fisheye2/fisheye2/fish_track_off_hover.png");
}

FisheyeBottomBar::~FisheyeBottomBar()
{
    s_self = nullptr;
    delete ui;
}

FisheyeBottomBar *FisheyeBottomBar::instance()
{
    return s_self;
}

void FisheyeBottomBar::updateFisheyeButtonState()
{
    int fisheyeMount = FisheyeControl::instance()->currentFisheyeMount();
    int fisheyeDisplay = FisheyeControl::instance()->currentFisheyeDisplay();
    setFishMount(fisheyeMount);
    setFishDisplay(fisheyeDisplay);

    //
    bool trackEnable = true;
    if (fisheyeMount != MsQt::FISHEYE_INSTALL_CEILING) {
        trackEnable = false;
    }
    //
    switch (fisheyeDisplay) {
    case MsQt::FISHEYE_DISPLAY_4R:
    case MsQt::FISHEYE_DISPLAY_1O3R:
    case MsQt::FISHEYE_DISPLAY_1P3R:
        break;
    default:
        trackEnable = false;
        break;
    }
    //
    MsCameraVersion version = qMsNvr->cameraVersion(FisheyeControl::instance()->currentChannel());
    if (version < MsCameraVersion(7, 73)) {
        trackEnable = false;
    }

    setFisheyeAutoTrackButtonEnabled(trackEnable);
    if (trackEnable) {
        setFisheyeAutoTrackButtonChecked(FisheyeControl::instance()->isTrackEnable() && FisheyeControl::instance()->isTrackShow());
    }

    //
    if (FisheyeControl::instance()) {
        FisheyeControl::instance()->setFisheyeAutoTrack(ui->toolButton_fish_track->isChecked());
    }
}

void FisheyeBottomBar::setFishMount(int mode)
{
    // wall has not 2p mode
    switch (mode) {
    case MsQt::FISHEYE_INSTALL_CEILING:
        ui->toolButton_ceil->setChecked(true);
        ui->toolButton_2P->show();
        break;
    case MsQt::FISHEYE_INSTALL_WALL:
        ui->toolButton_wall->setChecked(true);
        ui->toolButton_2P->hide();
        break;
    case MsQt::FISHEYE_INSTALL_FLAT:
        ui->toolButton_flat->setChecked(true);
        ui->toolButton_2P->show();
        break;
    }
}

void FisheyeBottomBar::setFishDisplay(int mode)
{
    switch (mode) {
    case MsQt::FISHEYE_DISPLAY_1O:
        ui->toolButton_1O->setChecked(true);
        break;
    case MsQt::FISHEYE_DISPLAY_1P:
        ui->toolButton_1P->setChecked(true);
        break;
    case MsQt::FISHEYE_DISPLAY_2P:
        ui->toolButton_2P->setChecked(true);
        break;
    case MsQt::FISHEYE_DISPLAY_4R:
        ui->toolButton_4R->setChecked(true);
        break;
    case MsQt::FISHEYE_DISPLAY_1O3R:
        ui->toolButton_1O3R->setChecked(true);
        break;
    case MsQt::FISHEYE_DISPLAY_1P3R:
        ui->toolButton_1P3R->setChecked(true);
        break;
    }
}

void FisheyeBottomBar::setFisheyeAutoTrackButtonEnabled(bool enabled)
{
    if (!enabled) {
        ui->toolButton_fish_track->setChecked(false);
    }
    ui->toolButton_fish_track->setEnabled(enabled);
}

void FisheyeBottomBar::setFisheyeAutoTrackButtonChecked(bool checked)
{
    ui->toolButton_fish_track->setChecked(checked);
}

void FisheyeBottomBar::setFisheyeMode(int mount, int display)
{
    int fisheyeMount = FisheyeControl::instance()->currentFisheyeMount();
    int fisheyeDisplay = FisheyeControl::instance()->currentFisheyeDisplay();

    int newMount = -1;
    int newDisplay = -1;
    if (mount >= 0 && mount != fisheyeMount) {
        newMount = mount;
    }
    if (display >= 0 && display != fisheyeDisplay) {
        newDisplay = display;
    }

    if ((newMount >= 0 || newDisplay >= 0) && LiveView::instance()->isChannelConnected(FisheyeControl::instance()->currentChannel())) {
        const int &result = MessageBox::question(this, GET_TEXT("LIVEVIEW/20095", "Switch to another mode?"));
        if (result == MessageBox::Cancel) {
            updateFisheyeButtonState();
            return;
        }
    } else {
        updateFisheyeButtonState();
        return;
    }

    //
    FisheyeControl::instance()->setFisheyeMode(newMount, newDisplay);
    updateFisheyeButtonState();
}

void FisheyeBottomBar::on_toolButton_fish_track_clicked(bool checked)
{
    if (FisheyeControl::instance()) {
        FisheyeControl::instance()->setFisheyeAutoTrack(checked);
    }
}

void FisheyeBottomBar::on_toolButton_close_clicked()
{
    qDebug() << "FisheyeBottomBar::on_toolButton_close_clicked";
    FisheyeControl::instance()->close();
}

void FisheyeBottomBar::onLanguageChanged()
{
    //
    ui->toolButton_ceil->setToolTip(GET_TEXT("LIVEVIEW/20086", "Ceiling"));
    ui->toolButton_wall->setToolTip(GET_TEXT("LIVEVIEW/20087", "Wall"));
    ui->toolButton_flat->setToolTip(GET_TEXT("LIVEVIEW/20088", "Flat"));
    ui->toolButton_1O->setToolTip(GET_TEXT("LIVEVIEW/20089", "1O"));
    ui->toolButton_1P->setToolTip(GET_TEXT("LIVEVIEW/20090", "1P"));
    ui->toolButton_2P->setToolTip(GET_TEXT("LIVEVIEW/20091", "2P"));
    ui->toolButton_4R->setToolTip(GET_TEXT("LIVEVIEW/20092", "4R"));
    ui->toolButton_1O3R->setToolTip(GET_TEXT("LIVEVIEW/20093", "1O3R"));
    ui->toolButton_1P3R->setToolTip(GET_TEXT("LIVEVIEW/20094", "1P3R"));
    ui->toolButton_fish_track->setToolTip(GET_TEXT("PTZDIALOG/21028", "Fisheye Auto Tracking"));
    ui->toolButton_close->setToolTip(GET_TEXT("PTZDIALOG/21005", "Close"));
}

void FisheyeBottomBar::onMountGroupClicked(int id)
{
    MyIconToolButton *button = static_cast<MyIconToolButton *>(m_mountGroup->button(id));
    if (button) {
        button->clearUnderMouse();
    }
    setFisheyeMode(id, -1);
}

void FisheyeBottomBar::onDisplayGroupClicked(int id)
{
    MyIconToolButton *button = static_cast<MyIconToolButton *>(m_displayGroup->button(id));
    if (button) {
        button->clearUnderMouse();
    }
    setFisheyeMode(-1, id);
}
