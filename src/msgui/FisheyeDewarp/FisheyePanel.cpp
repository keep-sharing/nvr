#include "FisheyePanel.h"
#include "ui_FisheyePanel.h"
#include "BottomBar.h"
#include "LiveView.h"
#include "LiveViewSub.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "SubControl.h"
#include "centralmessage.h"
#include <QEvent>
#include <QtDebug>

QMap<int, FisheyeKey> FisheyePanel::s_controlChannelMap;
QMap<FisheyeKey, PanelState> FisheyePanel::s_panelStateMap;
int FisheyePanel::s_vapiWinId = 0;
FisheyeKey FisheyePanel::s_currentFisheyeKey;
int FisheyePanel::s_controlMode = -1;

QDebug operator<<(QDebug dbg, const FisheyeKey &f)
{
    dbg.nospace() << QString("FisheyeKey(mode: %1, channel: %2, sid: %3, screen: %4").arg(f.mode).arg(f.channel).arg(f.sid).arg(f.screen);

    return dbg.space();
}

FisheyePanel::FisheyePanel(QWidget *parent)
    : MsWidget(parent)
    , ui(new Ui::FisheyePanel)
{
    ui->setupUi(this);

    //
    ui->toolButton_enable->setNormalIcon(":/fisheye2/fisheye2/fisheye_client-side_dewarping_white.png");
    ui->toolButton_enable->setCheckedIcon(":/fisheye2/fisheye2/fisheye_client-side_dewarping_blue.png");

    //
    m_installButtonGroup = new QButtonGroup(this);
    m_installButtonGroup->setExclusive(false);
    m_installButtonGroup->addButton(ui->toolButton_install_ceiling, Ceilling);
    m_installButtonGroup->addButton(ui->toolButton_install_wall, wall);
    m_installButtonGroup->addButton(ui->toolButton_install_flat, flat);
    connect(m_installButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onInstallButtonGroupClicked(int)));

    ui->toolButton_install_ceiling->setNormalIcon(":/fisheye2/fisheye2/Ceil_white.png");
    ui->toolButton_install_ceiling->setCheckedIcon(":/fisheye2/fisheye2/Ceil_blue.png");
    ui->toolButton_install_ceiling->setDisabledIcon(":/fisheye2/fisheye2/Ceil_hover.png");
    ui->toolButton_install_wall->setNormalIcon(":/fisheye2/fisheye2/Wall_white.png");
    ui->toolButton_install_wall->setCheckedIcon(":/fisheye2/fisheye2/Wall_blue.png");
    ui->toolButton_install_wall->setDisabledIcon(":/fisheye2/fisheye2/Wall_hover.png");
    ui->toolButton_install_flat->setNormalIcon(":/fisheye2/fisheye2/Flat_white.png");
    ui->toolButton_install_flat->setCheckedIcon(":/fisheye2/fisheye2/Flat_blue.png");
    ui->toolButton_install_flat->setDisabledIcon(":/fisheye2/fisheye2/Flat_hover.png");

    //
    m_displayButtonGroup = new QButtonGroup(this);
    m_displayButtonGroup->setExclusive(false);
    m_displayButtonGroup->addButton(ui->toolButton_1O, FISH_MODE_1O);
    m_displayButtonGroup->addButton(ui->toolButton_1P, FISH_MODE_1P);
    m_displayButtonGroup->addButton(ui->toolButton_2P, FISH_MODE_2P);
    m_displayButtonGroup->addButton(ui->toolButton_4R, FISH_MODE_4R);
    m_displayButtonGroup->addButton(ui->toolButton_1O3R, FISH_MODE_1O3R);
    m_displayButtonGroup->addButton(ui->toolButton_1P1R, FISH_MODE_1P1R);
    m_displayButtonGroup->addButton(ui->toolButton_1P4R, FISH_MODE_1P4R);
    m_displayButtonGroup->addButton(ui->toolButton_1P6R, FISH_MODE_1P6R);
    m_displayButtonGroup->addButton(ui->toolButton_1O8R, FISH_MODE_1O8R);
    connect(m_displayButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onDisplayButtonGroupClicked(int)));

    ui->toolButton_1O->setNormalIcon(":/fisheye2/fisheye2/1O_white.png");
    ui->toolButton_1O->setCheckedIcon(":/fisheye2/fisheye2/1O_blue.png");
    ui->toolButton_1O->setDisabledIcon(":/fisheye2/fisheye2/1O_hover.png");
    ui->toolButton_1P->setNormalIcon(":/fisheye2/fisheye2/1P_white.png");
    ui->toolButton_1P->setCheckedIcon(":/fisheye2/fisheye2/1P_blue.png");
    ui->toolButton_1P->setDisabledIcon(":/fisheye2/fisheye2/1P_hover.png");
    ui->toolButton_2P->setNormalIcon(":/fisheye2/fisheye2/2P_white.png");
    ui->toolButton_2P->setCheckedIcon(":/fisheye2/fisheye2/2P_blue.png");
    ui->toolButton_2P->setDisabledIcon(":/fisheye2/fisheye2/2P_hover.png");
    ui->toolButton_4R->setNormalIcon(":/fisheye2/fisheye2/4R_white.png");
    ui->toolButton_4R->setCheckedIcon(":/fisheye2/fisheye2/4R_blue.png");
    ui->toolButton_4R->setDisabledIcon(":/fisheye2/fisheye2/4R_hover.png");
    ui->toolButton_1O3R->setNormalIcon(":/fisheye2/fisheye2/1O3R_white.png");
    ui->toolButton_1O3R->setCheckedIcon(":/fisheye2/fisheye2/1O3R_blue.png");
    ui->toolButton_1O3R->setDisabledIcon(":/fisheye2/fisheye2/1O3R_hover.png");
    ui->toolButton_1P1R->setNormalIcon(":/fisheye2/fisheye2/1P1R_white.png");
    ui->toolButton_1P1R->setCheckedIcon(":/fisheye2/fisheye2/1P1R_blue.png");
    ui->toolButton_1P1R->setDisabledIcon(":/fisheye2/fisheye2/1P1R_hover.png");
    ui->toolButton_1P4R->setNormalIcon(":/fisheye2/fisheye2/1P4R_white.png");
    ui->toolButton_1P4R->setCheckedIcon(":/fisheye2/fisheye2/1P4R_blue.png");
    ui->toolButton_1P4R->setDisabledIcon(":/fisheye2/fisheye2/1P4R_hover.png");
    ui->toolButton_1P6R->setNormalIcon(":/fisheye2/fisheye2/1P6R_white.png");
    ui->toolButton_1P6R->setCheckedIcon(":/fisheye2/fisheye2/1P6R_blue.png");
    ui->toolButton_1P6R->setDisabledIcon(":/fisheye2/fisheye2/1P6R_hover.png");
    ui->toolButton_1O8R->setNormalIcon(":/fisheye2/fisheye2/1O8R_white.png");
    ui->toolButton_1O8R->setCheckedIcon(":/fisheye2/fisheye2/1O8R_blue.png");
    ui->toolButton_1O8R->setDisabledIcon(":/fisheye2/fisheye2/1O8R_hover.png");

    m_displayButtonList.append(ui->toolButton_1O);
    m_displayButtonList.append(ui->toolButton_1P);
    m_displayButtonList.append(ui->toolButton_2P);
    m_displayButtonList.append(ui->toolButton_4R);
    m_displayButtonList.append(ui->toolButton_1O3R);
    m_displayButtonList.append(ui->toolButton_1P1R);
    m_displayButtonList.append(ui->toolButton_1P4R);
    m_displayButtonList.append(ui->toolButton_1P6R);
    m_displayButtonList.append(ui->toolButton_1O8R);

    ui->toolButton_enable->setChecked(false);

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();

    installEventFilter(this);
}

FisheyePanel::~FisheyePanel()
{
    delete ui;
}

FisheyeKey FisheyePanel::fisheyeChannel()
{
    return s_controlChannelMap.value(s_controlMode);
}

FisheyeKey FisheyePanel::fisheyeChannel(int mode)
{
    return s_controlChannelMap.value(mode);
}

void FisheyePanel::setFisheyeChannel(FisheyeKey key)
{
    s_controlChannelMap.insert(s_controlMode, key);
}

void FisheyePanel::setFisheyeChannel(int mode, FisheyeKey key)
{
    s_controlChannelMap.insert(mode, key);
}

bool FisheyePanel::hasFisheyeChannel()
{
    for (auto iter = s_controlChannelMap.constBegin(); iter != s_controlChannelMap.constEnd(); ++iter) {
        FisheyeKey fisheyeKey = iter.value();
        if (fisheyeKey.channel >= 0) {
            if (fisheyeKey != s_currentFisheyeKey) {
                qDebug() << "FisheyePanel::hasFisheyeChannel," << fisheyeKey;
                return true;
            }
        }
    }
    return false;
}

void FisheyePanel::clearFisheyeChannel()
{
    qDebug() << "FisheyePanel::clearFisheyeChannel";
    s_controlChannelMap.clear();
}

void FisheyePanel::clearFisheyeChannel(int mode)
{
    s_controlChannelMap.remove(mode);
}

int FisheyePanel::fisheyeDisplayMode(FisheyeKey key)
{
    return s_panelStateMap.value(key).display;
}

PanelState FisheyePanel::panelState()
{
    return s_panelStateMap.value(s_currentFisheyeKey);
}

void FisheyePanel::initializePanel()
{
    if (fisheyeChannel() == s_currentFisheyeKey) {
        ui->toolButton_enable->setChecked(true);

        const PanelState &state = s_panelStateMap.value(fisheyeChannel());

        qMsDebug() << QString("install: %1, display: %2").arg(FisheyeInstallModeString_SoftDewarp(state.installation)).arg(FisheyeDisplayModeString_SoftDewarp(state.display));
        QAbstractButton *displayButton = m_displayButtonGroup->button(state.display);
        setDisplayButtonChecked(displayButton);

        QAbstractButton *installationButton = m_installButtonGroup->button(state.installation);
        setInstallButtonChecked(installationButton);

        emit dewarpStateChanged(1);
    } else {
        ui->toolButton_enable->setChecked(false);
        clearInstallButtonChecked();
        clearDisplayButtonChecked();

        emit dewarpStateChanged(0);
    }
}

void FisheyePanel::closeDewarp()
{
    qDebug() << "FisheyePanel::closeDewarp";
    qMsNvr->fisheyeHandleLock();
    if (qMsNvr->s_fisheyeHandle) {
        req_fisheye_mode fisheye_mode;
        memset(&fisheye_mode, 0, sizeof(fisheye_mode));
        fisheye_mode.winid = s_vapiWinId;
        fisheye_mode.enFish = FISH_MODE_NONE;
        qDebug() << QString("REQUEST_FLAG_SET_VAPI_FISHEYE_MODE, %1, %2").arg(VapiWinIdString(s_vapiWinId)).arg(FisheyeDisplayModeString_SoftDewarp(FISH_MODE_NONE));
        sendMessageOnly(REQUEST_FLAG_SET_VAPI_FISHEYE_MODE, (void *)&fisheye_mode, sizeof(fisheye_mode));
    }
    qMsNvr->fisheyeHandleUnlock();

    FisheyePanel::setFisheyeChannel(FisheyeKey());
    ui->toolButton_enable->setChecked(false);

    emit dewarpStateChanged(0);
}

bool FisheyePanel::isDewarpEnabled() const
{
    return ui->toolButton_enable->isChecked();
}

bool FisheyePanel::eventFilter(QObject *obj, QEvent *evt)
{
    switch (evt->type()) {
    case QEvent::ContextMenu:
    case QEvent::Wheel:
        return true;
        break;
    default:
        break;
    }
    return QWidget::eventFilter(obj, evt);
}

void FisheyePanel::clearState()
{
    s_panelStateMap.remove(s_currentFisheyeKey);
}

void FisheyePanel::clearState(FisheyeKey key)
{
    s_panelStateMap.remove(key);
}

void FisheyePanel::saveInstallationState(int state)
{
    qMsDebug() << FisheyeInstallModeString_SoftDewarp(state);
    s_panelStateMap[s_currentFisheyeKey].installation = state;
}

void FisheyePanel::saveDisplayState(int state)
{
    qMsDebug() << FisheyeDisplayModeString_SoftDewarp(state);
    s_panelStateMap[s_currentFisheyeKey].display = state;
}

void FisheyePanel::setInstallMode(int mode)
{
    qMsNvr->fisheyeHandleLock();
    switch (mode) {
    case Ceilling:
    case wall:
    case flat:
        if (qMsNvr->s_fisheyeHandle) {
            qDebug() << "FisheyePlayer_SetInstallModel," << FisheyeInstallModeString_SoftDewarp(mode);
            FisheyePlayer_SetInstallModel(qMsNvr->s_fisheyeHandle, static_cast<Fisheye_InstallModel>(mode));
        }
        break;
    default:
        break;
    }
    qMsNvr->fisheyeHandleUnlock();
}

void FisheyePanel::setDisplayMode(int installMode, int &displayMode)
{
    //2020-08-10, 壁装相关模式的兼容（1P、1P1R、1P4R） ==> （1W、1W1R、1W4R）
    switch (installMode) {
    case wall:
        switch (displayMode) {
        case FISH_MODE_1P:
            displayMode = FISH_MODE_1W;
            break;
        case FISH_MODE_1P1R:
            displayMode = FISH_MODE_1W1R;
            break;
        case FISH_MODE_1P4R:
            displayMode = FISH_MODE_1W4R;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    req_fisheye_mode fisheye_mode;
    memset(&fisheye_mode, 0, sizeof(fisheye_mode));
    fisheye_mode.winid = s_vapiWinId;
    fisheye_mode.enFish = static_cast<FISH_E>(displayMode);
    qDebug() << QString("REQUEST_FLAG_SET_VAPI_FISHEYE_MODE, %1, %2, %3").arg(VapiWinIdString(s_vapiWinId)).arg(FisheyeInstallModeString_SoftDewarp(installMode)).arg(FisheyeDisplayModeString_SoftDewarp(displayMode));
    sendMessageOnly(REQUEST_FLAG_SET_VAPI_FISHEYE_MODE, (void *)&fisheye_mode, sizeof(fisheye_mode));
}

void FisheyePanel::resetDisplayLayout(int installMode)
{
    for (int i = 0; i < m_displayButtonList.size(); ++i) {
        QToolButton *button = m_displayButtonList.at(i);
        ui->gridLayout_display->removeWidget(button);
        //BUG 临时解决直接点击1P导致1O出现焦点问题
        if (button != ui->toolButton_1O) {
            button->hide();
        }
    }

    int row = 0;
    int column = 0;
    for (int i = 0; i < m_displayButtonList.size(); ++i) {
        QToolButton *button = m_displayButtonList.at(i);
        if (installMode == wall) {
            if (button != ui->toolButton_2P && button != ui->toolButton_1P6R && button != ui->toolButton_1O8R) {
                ui->gridLayout_display->addWidget(button, row, column);
                button->show();
                column++;
                if (column > 2) {
                    column = 0;
                    row++;
                }
            }
        } else {
            ui->gridLayout_display->addWidget(button, row, column);
            button->show();
            column++;
            if (column > 2) {
                column = 0;
                row++;
            }
        }
    }
}

void FisheyePanel::setInstallButtonChecked(QAbstractButton *button)
{
    qMsDebug() << button;
    if (!button) {
        return;
    }
    button->setChecked(true);
    QList<QAbstractButton *> buttonList = m_installButtonGroup->buttons();
    for (int i = 0; i < buttonList.size(); ++i) {
        QAbstractButton *tempButton = buttonList.at(i);
        if (tempButton != button) {
            tempButton->setChecked(false);
        }
    }
}

void FisheyePanel::clearInstallButtonChecked()
{
    QList<QAbstractButton *> buttonList = m_installButtonGroup->buttons();
    for (int i = 0; i < buttonList.size(); ++i) {
        QAbstractButton *tempButton = buttonList.at(i);
        tempButton->setChecked(false);
    }
}

void FisheyePanel::setDisplayButtonChecked(QAbstractButton *button)
{
    qMsDebug() << button;
    if (!button) {
        return;
    }
    button->setChecked(true);
    QList<QAbstractButton *> buttonList = m_displayButtonGroup->buttons();
    for (int i = 0; i < buttonList.size(); ++i) {
        QAbstractButton *tempButton = buttonList.at(i);
        if (tempButton != button) {
            tempButton->setChecked(false);
        }
    }
}

void FisheyePanel::clearDisplayButtonChecked()
{
    QList<QAbstractButton *> buttonList = m_displayButtonGroup->buttons();
    for (int i = 0; i < buttonList.size(); ++i) {
        QAbstractButton *tempButton = buttonList.at(i);
        tempButton->setChecked(false);
    }
}

void FisheyePanel::enterDewarpState()
{
    qMsDebug() << "";
    //
    setFisheyeChannel(s_currentFisheyeKey);
    const PanelState &state = s_panelStateMap.value(s_currentFisheyeKey);

    QAbstractButton *displayButton = m_displayButtonGroup->button(state.display);
    setDisplayButtonChecked(displayButton);
    int displayMode = state.display;
    setDisplayMode(state.installation, displayMode);
    saveDisplayState(displayMode);

    //这里鱼眼句柄可能还没生成，默认安装模式在鱼眼句柄回调函数里设置
    QAbstractButton *installationButton = m_installButtonGroup->button(state.installation);
    setInstallButtonChecked(installationButton);
    resetDisplayLayout(state.installation);

    //
    emit dewarpStateChanged(1);
}

void FisheyePanel::closeOtherScreenFisheye()
{
    SCREEN_E currentScreen = SubControl::instance()->currentScreen();
    switch (currentScreen) {
    case SCREEN_MAIN:
        if (LiveViewSub::instance() && LiveViewSub::instance()->isVisible()) {
            LiveView::instance()->resetSubScreenLayout();
        }
        break;
    case SCREEN_SUB:
        if (LiveViewSub::instance() && LiveViewSub::instance()->isVisible()) {
            LiveView::instance()->resetMainScreenLayout();
        }
        break;
    default:
        break;
    }
}

void FisheyePanel::onLanguageChanged()
{
    ui->label_enable->setText(GET_TEXT("FISHEYE/12000", "Enable NVR Dewarping"));
    ui->toolButton_enable->setToolTip(GET_TEXT("FISHEYE/12000", "Enable NVR Dewarping"));
    ui->label_installMode->setText(GET_TEXT("FISHEYE/12001", "Installation Mode"));
    ui->toolButton_install_ceiling->setToolTip(GET_TEXT("FISHEYE/12002", "Ceiling"));
    ui->toolButton_install_wall->setToolTip(GET_TEXT("FISHEYE/12003", "Wall"));
    ui->toolButton_install_flat->setToolTip(GET_TEXT("FISHEYE/12004", "Flat"));
    ui->label_displayMode->setText(GET_TEXT("FISHEYE/12005", "Display Mode"));
    ui->toolButton_1O->setToolTip("1O");
    ui->toolButton_1P->setToolTip("1P");
    ui->toolButton_2P->setToolTip("2P");
    ui->toolButton_4R->setToolTip("4R");
    ui->toolButton_1O3R->setToolTip("1O3R");
    ui->toolButton_1P1R->setToolTip("1P1R");
    ui->toolButton_1P4R->setToolTip("1P4R");
    ui->toolButton_1P6R->setToolTip("1P6R");
    ui->toolButton_1O8R->setToolTip("1O8R");
}

void FisheyePanel::onInstallButtonGroupClicked(int id)
{
    qMsDebug() << "";
    QAbstractButton *button = m_installButtonGroup->button(id);
    if (hasFisheyeChannel()) {
        button->setAttribute(Qt::WA_UnderMouse, false);
        int result = MessageBox::question(this, GET_TEXT("FISHEYE/12008", "NVR Dewarping of the other channel will be disabled, continue?"));
        if (result == MessageBox::Cancel) {
            button->setChecked(false);
            return;
        } else {
            clearFisheyeChannel();
            clearState(fisheyeChannel());

            closeOtherScreenFisheye();
        }
    }
    //
    setInstallButtonChecked(button);
    //
    resetDisplayLayout(id);
    QAbstractButton *displayButton = m_displayButtonGroup->button(FISH_MODE_1O);
    setDisplayButtonChecked(displayButton);
    //
    setInstallMode(id);
    saveInstallationState(id);
    //
    int displayMode = FISH_MODE_1O;
    if (!ui->toolButton_enable->isChecked()) {
        ui->toolButton_enable->setChecked(true);
        enterDewarpState();
    } else {
        setDisplayMode(id, displayMode);
    }
    saveDisplayState(displayMode);
}

void FisheyePanel::onDisplayButtonGroupClicked(int id)
{
    qMsDebug() << "";
    QAbstractButton *button = m_displayButtonGroup->button(id);
    if (hasFisheyeChannel()) {
        button->setAttribute(Qt::WA_UnderMouse, false);
        int result = MessageBox::question(this, GET_TEXT("FISHEYE/12008", "NVR Dewarping of the other channel will be disabled, continue?"));
        if (result == MessageBox::Cancel) {
            button->setChecked(false);
            return;
        } else {
            clearFisheyeChannel();
            clearState(fisheyeChannel());

            closeOtherScreenFisheye();
        }
    }
    //
    setDisplayButtonChecked(button);
    //
    if (!ui->toolButton_enable->isChecked()) {
        ui->toolButton_enable->setChecked(true);
        saveDisplayState(id);
        enterDewarpState();
    } else {
        const PanelState &state = s_panelStateMap.value(s_currentFisheyeKey);
        setDisplayMode(state.installation, id);
        saveDisplayState(id);
    }
}

void FisheyePanel::on_toolButton_enable_clicked(bool checked)
{
    if (checked) {
        if (hasFisheyeChannel()) {
            ui->toolButton_enable->clearUnderMouse();
            int result = MessageBox::question(this, GET_TEXT("FISHEYE/12008", "NVR Dewarping of the other channel will be disabled, continue?"));
            if (result == MessageBox::Cancel) {
                ui->toolButton_enable->setChecked(false);
                return;
            } else {
                clearFisheyeChannel();
                clearState(fisheyeChannel());

                closeOtherScreenFisheye();
            }
        }

        enterDewarpState();
    } else {
        closeDewarp();
        clearState();
        resetDisplayLayout(Ceilling);
        clearInstallButtonChecked();
        clearDisplayButtonChecked();
    }
}
