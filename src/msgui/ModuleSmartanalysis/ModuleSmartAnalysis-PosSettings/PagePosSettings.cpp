#include "PagePosSettings.h"
#include "ui_PagePosSettings.h"
#include "ActionPos.h"
#include "DynamicDisplayData.h"
#include "EffectiveTimePos.h"
#include "EventLoop.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsMessage.h"
#include "MsWaitting.h"
#include "PosConnectionModeSettings.h"
#include "PosDisplayData.h"
#include "PosPrivacySettings.h"
#include "PosTextEdit.h"



PagePosSettings::PagePosSettings(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::PosSettings)
{
    ui->setupUi(this);

    //
    for (int i = 0; i < MAX_POS_CLIENT; ++i) {
        ui->comboBoxPosNo->addItem(QString::number(i + 1), i);
    }

    ui->comboBoxPos->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBoxPos->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);
    //
    ui->lineEditPosName->setCheckMode(MyLineEdit::EmptyCheck);
    ui->lineEditPosName->setMaxLength(32);

    ui->comboBoxPosProtocol->addItem(GET_TEXT("POS/130004", "General"), POS_COMMON);
    ui->comboBoxPosProtocol->setEnabled(false);

    ui->comboBoxConnectionMode->addItem("TCP", 0);
    ui->comboBoxConnectionMode->addItem("UDP", 1);

    ui->comboBoxLiveViewDisplay->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBoxLiveViewDisplay->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);

    ui->comboBoxDisplayChannel->addItem(GET_TEXT("ACTION/56010", "Select Channel"), -1);
    for (int i = 0; i < qMsNvr->maxChannel(); ++i) {
        ui->comboBoxDisplayChannel->addItem(QString::number(i + 1), i);
    }

    ui->comboBoxCharacterEncoding->addItem(GET_TEXT("POS/130012", "Unicode(UTF-8)"), 0);
    ui->comboBoxCharacterEncoding->addItem("Windows 1250", 1);

    ui->comboBoxFontSize->addItem(GET_TEXT("POS/130014", "Small"), 0);
    ui->comboBoxFontSize->addItem(GET_TEXT("POS/130015", "Medium"), 1);
    ui->comboBoxFontSize->addItem(GET_TEXT("POS/130016", "Large"), 2);

    const auto &colorList = gPosDisplayData.colorList();
    for (int i = 0; i < colorList.size(); ++i) {
        const auto &colorInfo = colorList.at(i);
        ui->comboBoxFontColor->addItem(colorInfo.name, colorInfo.index);
        ui->comboBoxFontColor->setItemData(i, colorInfo.color, Qt::DecorationRole);
    }

    ui->comboBoxOverlayMode->addItem(GET_TEXT("POS/130019", "Page"), 0);
    ui->comboBoxOverlayMode->addItem(GET_TEXT("POS/130020", "Roll"), 1);
    //
    ui->lineEditClearScreenTime->setRange(5, 3600);
    ui->lineEditDisplayTime->setRange(5, 3600);
    ui->lineEditTimeout->setRange(5, 3600);
    //
    ui->widgetMessage->hide();

    //
    m_drawRegion = new GraphicsPosDisplayRegion();
    ui->video->addGraphicsItem(m_drawRegion);
    connect(m_drawRegion, SIGNAL(mouseDragging()), this, SLOT(onSceneMouseDragging()));

    //
    onLanguageChanged();
}

PagePosSettings::~PagePosSettings()
{
    delete ui;
}

void PagePosSettings::initializeData()
{
    m_currentPosNo = -1;
    if (m_action) {
        m_action->clearCache();
    }
    if (m_effectiveTime) {
        m_effectiveTime->clearCache();
    }

    memset(m_currentPosConfigs, 0, sizeof(m_currentPosConfigs));
    read_pos_settings(SQLITE_FILE_NAME, m_currentPosConfigs);

    memcpy(m_lastPosConfigs, m_currentPosConfigs, sizeof(m_lastPosConfigs));

    ui->comboBoxPosNo->setCurrentIndex(0);
}

void PagePosSettings::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_SET_POS_SETTING:
        ON_RESPONSE_FLAG_SET_POS_SETTING(message);
        break;
    }
}

void PagePosSettings::ON_RESPONSE_FLAG_SET_POS_SETTING(MessageReceive *message)
{
    Q_UNUSED(message)

    gEventLoopExit(0);
}

bool PagePosSettings::saveCurrentData()
{
    Db_POS_CONFIG &config = m_currentPosConfigs[m_currentPosNo];
    bool enable = ui->comboBoxPos->currentIntData();
    if (enable) {
        bool valid = ui->lineEditPosName->checkValid();
        valid &= ui->lineEditClearScreenTime->checkValid();
        valid &= ui->lineEditDisplayTime->checkValid();
        valid &= ui->lineEditTimeout->checkValid();
        if (!valid) {
            return false;
        }
        int result = 0;
        if (QString(config.ip).isEmpty()) {
            result = 1;
        } else if ((config.nvrPort < 1024) || (config.nvrPort > 65535)) {
            result = 2;
        } else {
            const Db_POS_CONFIG &current = m_currentPosConfigs[m_currentPosNo];
            const Db_POS_CONFIG &last = m_lastPosConfigs[m_currentPosNo];
            if (current.nvrPort != last.nvrPort) {
                if (qMsNvr->isPortUsed(current.nvrPort)) {
                    result = 3;
                }
            }
        }
        if (result != 0) {
            qMsDebug() << "result:" << result;
            MessageBox::information(this, GET_TEXT("POS/130046", "Invalid connection mode settings."));
            return false;
        }
    }
    //
    m_drawRegion->getPosItemRegion(m_currentPosNo, config.startX, config.startY, config.areaWidth, config.areaHeight);
    //
    config.enable = enable;
    snprintf(config.name, sizeof(config.name), ui->lineEditPosName->text().toLocal8Bit().data());
    config.protocol = ui->comboBoxPosProtocol->currentIntData();
    config.connectionMode = ui->comboBoxConnectionMode->currentIntData();
    config.liveviewDisplay = ui->comboBoxLiveViewDisplay->currentIntData();
    config.displayChannel = ui->comboBoxDisplayChannel->currentIntData();
    config.characterEncodeing = ui->comboBoxCharacterEncoding->currentIntData();
    config.fontSize = ui->comboBoxFontSize->currentIntData();
    config.fontColor = ui->comboBoxFontColor->currentIntData();
    config.overlayMode = ui->comboBoxOverlayMode->currentIntData();
    config.displayTime = ui->lineEditDisplayTime->text().toInt();
    config.clearTime = ui->lineEditClearScreenTime->text().toInt();
    config.timeout = ui->lineEditTimeout->text().toInt();
    return true;
}

void PagePosSettings::onLanguageChanged()
{
    ui->labelPosNo->setText(GET_TEXT("POS/130001", "POS No."));
    ui->labelPos->setText(GET_TEXT("POS/130000", "POS"));
    ui->labelPosName->setText(GET_TEXT("POS/130002", "POS Name"));
    ui->labelPosProtocol->setText(GET_TEXT("POS/130003", "POS Protocol"));
    ui->labelConnectionMode->setText(GET_TEXT("POS/130005", "Connection Mode"));
    ui->labelConnectionModeSettings->setText(GET_TEXT("POS/130007", "Connection Mode Settings"));
    ui->pushButtonConnectionModeSettingsEdit->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->labelLiveViewDisplay->setText(GET_TEXT("POS/130031", "Live View Display"));
    ui->labelDisplayChannel->setText(GET_TEXT("POS/130008", "Display Channel"));
    ui->labelDisplayRegion->setText(GET_TEXT("POS/130009", "Display Region"));
    ui->pushButtonDisplayRegionReset->setText(GET_TEXT("COMMON/1057", "Reset"));
    ui->labelCharacterEncoding->setText(GET_TEXT("POS/130010", "Character Encoding"));
    ui->labelFontSize->setText(GET_TEXT("POS/130013", "Font Size"));
    ui->labelFontColor->setText(GET_TEXT("POS/130017", "Font Color"));
    ui->labelOverlayMode->setText(GET_TEXT("POS/130018", "Overlay Mode"));
    ui->labelClearScreenTime->setText(GET_TEXT("POS/130045", "Clear Screen Time"));
    ui->labelDisplayTime->setText(GET_TEXT("POS/130021", "Display Time"));
    ui->labelTimeout->setText(GET_TEXT("POS/130022", "Timeout"));
    ui->labelPrivacySettings->setText(GET_TEXT("POS/130023", "Privacy Settings"));
    ui->pushButtonPrivacySettingsEdit->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->labelEffectiveTime->setText(GET_TEXT("ALARMOUT/53003", "Effective Time"));
    ui->pushButtonEffectiveTimeEdit->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->labelAction->setText(GET_TEXT("VIDEOLOSS/50002", "Action"));
    ui->pushButtonActionEdit->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->pushButtonApply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->labelNote->setText(GET_TEXT("POS/130039", "Please configure record channel in Action interface if needed."));
}

void PagePosSettings::onSceneMouseDragging()
{
    QRect rc = m_drawRegion->posGlobalGeometry();
    if (rc.isValid()) {
        rc.moveTopLeft(mapFromGlobal(rc.topLeft()));
    }
}

void PagePosSettings::on_comboBoxPosNo_indexSet(int index)
{
    int posNo = ui->comboBoxPosNo->itemData(index).toInt();
    if (posNo != m_currentPosNo) {
        ui->video->clearPos();
    }

    if (m_currentPosNo != -1) {
        if (!saveCurrentData()) {
            ui->comboBoxPosNo->editCurrentIndexFromData(m_currentPosNo);
            return;
        }
    }
    m_currentPosNo = posNo;

    const Db_POS_CONFIG &config = m_currentPosConfigs[m_currentPosNo];
    //
    ui->lineEditPosName->setText(config.name);
    ui->comboBoxPos->setCurrentIndexFromData(config.enable);
    ui->comboBoxPosProtocol->setCurrentIndexFromData(POS_COMMON);
    ui->comboBoxConnectionMode->setCurrentIndexFromData(config.connectionMode);
    ui->comboBoxLiveViewDisplay->setCurrentIndexFromData(config.liveviewDisplay);
    //
    QMap<int, int> channelCountMap;
    for (int i = 0; i < MAX_POS_CLIENT; ++i) {
        const auto &c = m_currentPosConfigs[i];
        channelCountMap[c.displayChannel]++;
    }
    ui->comboBoxDisplayChannel->clear();
    ui->comboBoxDisplayChannel->addItem(GET_TEXT("ACTION/56010", "Select Channel"), -1);
    for (int i = 0; i < qMsNvr->maxChannel(); ++i) {
        const auto count = channelCountMap.value(i);
        if (count < 4 || (count == 4 && i == config.displayChannel)) {
            ui->comboBoxDisplayChannel->addItem(QString::number(i + 1), i);
        }
    }
    ui->comboBoxDisplayChannel->setCurrentIndexFromData(config.displayChannel);
    //
    ui->comboBoxCharacterEncoding->setCurrentIndexFromData(config.characterEncodeing);
    ui->comboBoxFontSize->setCurrentIndexFromData(config.fontSize);
    ui->comboBoxFontColor->setCurrentIndexFromData(config.fontColor);
    ui->comboBoxOverlayMode->setCurrentIndexFromData(config.overlayMode);
    ui->lineEditClearScreenTime->setText(QString::number(config.clearTime));
    ui->lineEditDisplayTime->setText(QString::number(config.displayTime));
    ui->lineEditTimeout->setText(QString::number(config.timeout));
}

void PagePosSettings::on_comboBoxPos_indexSet(int index)
{
    int posNo = ui->comboBoxPosNo->currentData().toInt();
    Db_POS_CONFIG &config = m_currentPosConfigs[posNo];
    config.enable = index;

    QMetaObject::invokeMethod(this, "updateEnableState", Qt::QueuedConnection);
}

void PagePosSettings::on_comboBoxPosProtocol_indexSet(int index)
{
    Q_UNUSED(index)

    int protocol = ui->comboBoxPosProtocol->currentData().toInt();
    switch (protocol) {
    case POS_COMMON:
        ui->labelClearScreenTime->hide();
        ui->widgetClearScreenTime->hide();
        ui->labelDisplayTime->show();
        ui->widgetDisplayTime->show();
        ui->labelTimeout->show();
        ui->widgetTimeout->show();
        break;
    case POS_FORCOM:
        ui->labelClearScreenTime->show();
        ui->widgetClearScreenTime->show();
        ui->labelDisplayTime->hide();
        ui->widgetDisplayTime->hide();
        ui->labelTimeout->hide();
        ui->widgetTimeout->hide();
        break;
    default:
        break;
    }
}

void PagePosSettings::on_pushButtonConnectionModeSettingsEdit_clicked()
{
    ui->pushButtonConnectionModeSettingsEdit->clearUnderMouse();

    int index = ui->comboBoxPosNo->currentData().toInt();

    PosConnectionModeSettings setting(this);
    setting.setConfig(index, m_currentPosConfigs, m_lastPosConfigs);
    setting.exec();
}

void PagePosSettings::on_comboBoxDisplayChannel_indexSet(int index)
{
    int channel = ui->comboBoxDisplayChannel->itemData(index).toInt();
    if (channel < 0) {
        ui->video->stopVideo();
    } else {
        ui->video->playVideo(channel);
    }

    m_drawRegion->clearAll();

    if (m_currentPosNo >= 0 && m_currentPosNo < MAX_POS_CLIENT) {
        auto &config = m_currentPosConfigs[m_currentPosNo];
        config.displayChannel = channel;
    }

    //找出来所有display channel相同的pos
    for (int i = 0; i < MAX_POS_CLIENT; ++i) {
        const auto &config = m_currentPosConfigs[i];
        if (config.id == m_currentPosNo || (channel >= 0 && config.displayChannel == channel && config.enable)) {
            m_drawRegion->setPosItemRegion(config.id, config.startX, config.startY, config.areaWidth, config.areaHeight);
        }
    }
    m_drawRegion->setCurrentPos(m_currentPosNo);
}

void PagePosSettings::on_pushButtonDisplayRegionReset_clicked()
{
    m_drawRegion->setPosItemRegion(m_currentPosNo, 0, 0, 1000, 1000);
}

void PagePosSettings::on_pushButtonPrivacySettingsEdit_clicked()
{
    ui->pushButtonPrivacySettingsEdit->clearUnderMouse();

    int index = ui->comboBoxPosNo->currentData().toInt();
    Db_POS_CONFIG &config = m_currentPosConfigs[index];
    PosPrivacySettings setting(this);
    setting.setConfig(&config);
    setting.exec();
}

void PagePosSettings::on_pushButtonEffectiveTimeEdit_clicked()
{
    int index = ui->comboBoxPosNo->currentData().toInt();

    if (!m_effectiveTime) {
        m_effectiveTime = new EffectiveTimePos(this);
    }
    m_effectiveTime->showEffectiveTime(-1, index);
}

void PagePosSettings::on_pushButtonActionEdit_clicked()
{
    if (!m_action) {
        m_action = new ActionPos(this);
    }
    m_action->showAction(m_currentPosNo);
}

void PagePosSettings::updateEnableState()
{
    int posNo = ui->comboBoxPosNo->currentData().toInt();
    const Db_POS_CONFIG &config = m_currentPosConfigs[posNo];

    ui->lineEditPosName->setEnabled(config.enable);
    ui->comboBoxConnectionMode->setEnabled(config.enable);
    ui->pushButtonConnectionModeSettingsEdit->setEnabled(config.enable);
    ui->comboBoxLiveViewDisplay->setEnabled(config.enable);
    ui->comboBoxDisplayChannel->setEnabled(config.enable);
    ui->pushButtonDisplayRegionReset->setEnabled(config.enable);
    ui->comboBoxCharacterEncoding->setEnabled(config.enable);
    ui->comboBoxFontSize->setEnabled(config.enable);
    ui->comboBoxFontColor->setEnabled(config.enable);
    ui->comboBoxOverlayMode->setEnabled(config.enable);
    ui->lineEditClearScreenTime->setEnabled(config.enable);
    ui->lineEditDisplayTime->setEnabled(config.enable);
    ui->lineEditTimeout->setEnabled(config.enable);
    ui->pushButtonPrivacySettingsEdit->setEnabled(config.enable);
    ui->pushButtonEffectiveTimeEdit->setEnabled(config.enable);
    ui->pushButtonActionEdit->setEnabled(config.enable);

    m_drawRegion->setEnabled(posNo, config.enable);
}

void PagePosSettings::on_pushButtonApply_clicked()
{
    if (!saveCurrentData()) {
        return;
    }

    //防止web端也在配置，这里再检查一次displayChannel
    Db_POS_CONFIG posConfigs[MAX_POS_CLIENT];
    memset(posConfigs, 0, sizeof(posConfigs));
    read_pos_settings(SQLITE_FILE_NAME, posConfigs);
    QMap<int, int> channelCountMap;
    for (int i = 0; i < MAX_POS_CLIENT; ++i) {
        const auto &c = posConfigs[i];
        channelCountMap[c.displayChannel]++;
    }
    for (auto iter = channelCountMap.constBegin(); iter != channelCountMap.constEnd(); ++iter) {
        const auto &c = iter.key();
        const auto &v = iter.value();
        if (c >= 0 && v > 4) {
            MessageBox::information(this, GET_TEXT("POS/130038", "Failed to save because the display channel is occupied."));
            return;
        }
    }

    //
    //showWait();
    for (int i = 0; i < MAX_POS_CLIENT; ++i) {
        write_pos_setting(SQLITE_FILE_NAME, &m_currentPosConfigs[i]);
        if (m_effectiveTime) {
            m_effectiveTime->saveEffectiveTime(i);
        }
        if (m_action) {
            m_action->saveAction(i);
        }
        sendMessage(REQUEST_FLAG_SET_POS_SETTING, &i, sizeof(i));
        gEventLoopExec();
    }
    //closeWait();

    memcpy(m_lastPosConfigs, m_currentPosConfigs, sizeof(m_lastPosConfigs));
}

void PagePosSettings::on_pushButtonBack_clicked()
{
    back();
}
