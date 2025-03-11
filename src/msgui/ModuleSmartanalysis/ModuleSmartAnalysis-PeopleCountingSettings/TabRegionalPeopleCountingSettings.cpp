#include "TabRegionalPeopleCountingSettings.h"
#include "ui_TabRegionalPeopleCountingSettings.h"
#include "ActionRegionPeopleCounting.h"
#include "DrawSceneObjectSize.h"
#include "DynamicDisplayData.h"
#include "EffectiveTime.h"
#include "EffectiveTimePeopleCounting.h"
#include "EventLoop.h"
#include "FaceData.h"
#include "GraphicsMultiRegionScene.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsMessage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "VideoLineCrossing.h"
#include "channelcopydialog.h"

TabRegionalPeopleCountingSettings::TabRegionalPeopleCountingSettings(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::TabRegionalPeopleCountingSettings)
{
    ui->setupUi(this);

    m_regionScene = new GraphicsMultiRegionScene(this);
    m_regionScene->setAlarmable(false);
    connect(m_regionScene, SIGNAL(conflicted()), this, SLOT(onDrawRegionConflicted()), Qt::QueuedConnection);
    ui->commonVideo->setDrawScene(m_regionScene);

    ui->widgetButtonGroup->setCount(qMsNvr->maxChannel());
    connect(ui->widgetButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onChannelButtonClicked(int)));

    ui->comboBoxRegion->beginEdit();
    ui->comboBoxRegion->clear();
    ui->comboBoxRegion->addItem("1", 0);
    ui->comboBoxRegion->addItem("2", 1);
    ui->comboBoxRegion->addItem("3", 2);
    ui->comboBoxRegion->addItem("4", 3);
    ui->comboBoxRegion->endEdit();

    ui->checkableLineEditMaxStay->setRange(1, 60);
    ui->checkableLineEditMinStay->setRange(1, 60);
    ui->checkableLineEditMaxLength->setRange(1, 1800);

    ui->sliderSensitivity->setRange(1, 10);

    m_regionInfo = new MsIpcRegionalPeople;

    //object
    m_objScene = new DrawSceneObjectSize(this);
    connect(m_objScene, SIGNAL(objectSizeChanged(QRect, MsQtVca::ObjectSizeType)), this, SLOT(onSceneObjectSizeChanged(QRect, MsQtVca::ObjectSizeType)));
    ui->radioButtonMinLayout->setAutoExclusive(false);
    ui->radioButtonMaxLayout->setAutoExclusive(false);
    ui->widgetMinLineBox->setWidthRange(1, 320);
    ui->widgetMinLineBox->setHeightRange(1, 240);
    connect(ui->widgetMinLineBox, SIGNAL(widthChange()), this, SLOT(onLineEditMinWidthEditingFinished()));
    connect(ui->widgetMinLineBox, SIGNAL(heightChange()), this, SLOT(onLineEditMinHeightEditingFinished()));

    ui->widgetMaxLineBox->setWidthRange(1, 320);
    ui->widgetMaxLineBox->setHeightRange(1, 240);
    connect(ui->widgetMaxLineBox, SIGNAL(widthChange()), this, SLOT(onLineEditMaxWidthEditingFinished()));
    connect(ui->widgetMaxLineBox, SIGNAL(heightChange()), this, SLOT(onLineEditMaxHeightEditingFinished()));

    //
    connect(&gDynamicDisplayData, SIGNAL(dynamicDataChanged(int, int)), this, SLOT(updateDynamicDisplay(int, int)));
    onLanguageChanged();
}

TabRegionalPeopleCountingSettings::~TabRegionalPeopleCountingSettings()
{
    delete m_regionInfo;
    delete ui;
}

void TabRegionalPeopleCountingSettings::initializeData()
{
    ui->widgetButtonGroup->setCurrentIndex(0);
}

void TabRegionalPeopleCountingSettings::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPC_REGIONAL_PEOPLE:
        ON_RESPONSE_FLAG_GET_IPC_REGIONAL_PEOPLE(message);
        break;
    case RESPONSE_FLAG_SET_IPC_REGIONAL_PEOPLE:
        ON_RESPONSE_FLAG_SET_IPC_REGIONAL_PEOPLE(message);
        break;
    case RESPONSE_FLAG_UPDATE_REGIONAL_ACTION:
        ON_RESPONSE_FLAG_UPDATE_REGIONAL_ACTION(message);
        break;
    }
}

void TabRegionalPeopleCountingSettings::ON_RESPONSE_FLAG_GET_IPC_REGIONAL_PEOPLE(MessageReceive *message)
{
    MsIpcRegionalPeople *ipc_regional_people = static_cast<MsIpcRegionalPeople *>(message->data);
    memcpy(m_regionInfo, ipc_regional_people, sizeof(MsIpcRegionalPeople));

    gEventLoopExit(0);
}

void TabRegionalPeopleCountingSettings::ON_RESPONSE_FLAG_SET_IPC_REGIONAL_PEOPLE(MessageReceive *message)
{
    Q_UNUSED(message)
    clearCopyInfo();
    gEventLoopExit(0);
}

void TabRegionalPeopleCountingSettings::ON_RESPONSE_FLAG_UPDATE_REGIONAL_ACTION(MessageReceive *message)
{
    Q_UNUSED(message)
    gEventLoopExit(0);
}

int TabRegionalPeopleCountingSettings::currentRegionIndex() const
{
    return ui->comboBoxRegion->currentData().toInt();
}

void TabRegionalPeopleCountingSettings::updateEnableState()
{
    bool isEnable = ui->checkBoxEnable->isEnabled() && ui->checkBoxEnable->isChecked();
    ui->pushButtonObjectSizeEdit->setEnabled(isEnable);
    ui->pushButtonObjectSizeReset->setEnabled(isEnable);
    ui->pushButtonEditAction->setEnabled(isEnable);
    ui->pushButtonRegionalPeopleCountingSchedule->setEnabled(isEnable);
    ui->widgetSize->setEnabled(isEnable && ui->pushButtonObjectSizeEdit->buttonState() == PushButtonEditState::StateEditing);
    ui->pushButtonApply->setEnabled(ui->checkBoxEnable->isEnabled());
    ui->pushButtonCopy->setEnabled(ui->checkBoxEnable->isEnabled());
    ui->commonVideo->setDrawViewVisible(ui->pushButtonObjectSizeEdit->buttonState() == PushButtonEditState::StateEditing);

    //TODO 中间3个按钮的状态更新暂时没写

    ui->sliderSensitivity->setEnabled(isEnable);
}

void TabRegionalPeopleCountingSettings::clearSettings()
{
}

bool TabRegionalPeopleCountingSettings::saveRegionInfo(int index)
{
    MsIpcRegionalArea &regional = m_regionInfo->regional[index];
    //
    regional.enable = ui->checkBoxEnable->isChecked();
    regional.maxStayEnable = ui->checkableLineEditMaxStay->isChecked();
    regional.minStayEnable = ui->checkableLineEditMinStay->isChecked();
    regional.maxStayTimeEnable = ui->checkableLineEditMaxLength->isChecked();

    regional.maxStayValue = ui->checkableLineEditMaxStay->value();
    regional.minStayValue = ui->checkableLineEditMinStay->value();
    regional.maxStayTimeValue = ui->checkableLineEditMaxLength->value();

    if (ui->checkBoxEnable->isChecked()) {
        bool valid = ui->widgetMinLineBox->checkValid();
        valid = ui->widgetMaxLineBox->checkValid() && valid;
        valid = ui->checkableLineEditMaxStay->checkValid() && valid;
        valid = ui->checkableLineEditMinStay->checkValid() && valid;
        valid = ui->checkableLineEditMaxLength->checkValid() && valid;
        if (!valid) {
            return false;
        }
        if (regional.minStayValue > regional.maxStayValue) {
            MessageBox::information(this, GET_TEXT("REGIONAL_PEOPLECOUNTING/103306", "Min.Stay cannot be more than Max. Stay."));
            return false;
        }
    }
    m_regionScene->getRegion(m_regionInfo->regional, index);
    return true;
}

void TabRegionalPeopleCountingSettings::showDebugInfo(const QString &title)
{
    QString text;
    text += QString("\n%1\n").arg(title);
    text += QString("result: %1\n").arg(m_regionInfo->result);
    text += QString("chnid: %1\n").arg(m_regionInfo->chnid);
    text += QString("sensitivity: %1\n").arg(m_regionInfo->sensitivity);
    text += QString("minObj: (%1,%2 %3x%4)\n").arg(m_regionInfo->minObjX).arg(m_regionInfo->minObjY).arg(m_regionInfo->minObjWidth).arg(m_regionInfo->minObjHeight);
    text += QString("maxObj: (%1,%2 %3x%4)\n").arg(m_regionInfo->maxObjX).arg(m_regionInfo->maxObjY).arg(m_regionInfo->maxObjWidth).arg(m_regionInfo->maxObjHeight);
    for (int i = 0; i < MAX_LEN_4; ++i) {
        const MsIpcRegionalArea &region = m_regionInfo->regional[i];
        text += QString("--region: %1--\n").arg(i + 1);
        text += QString("enable: %1\n").arg(region.enable);
        text += QString("polygonX: %1\n").arg(region.polygonX);
        text += QString("polygonY: %1\n").arg(region.polygonY);
        text += QString("maxStayEnable: %1, maxStayValue: %2\n").arg(region.maxStayEnable).arg(region.maxStayValue);
        text += QString("minStayEnable: %1, minStayValue: %2\n").arg(region.minStayEnable).arg(region.minStayValue);
        text += QString("maxStayTimeEnable: %1, maxStayTimeValue: %2\n").arg(region.maxStayTimeEnable).arg(region.maxStayTimeValue);
    }
    qMsDebug() << qPrintable(text);
}

void TabRegionalPeopleCountingSettings::clearCopyInfo()
{
    for (int i = 0; i < MAX_LEN_65; i++) {
        m_regionInfo->copyChn[i] = '0';
    }
    m_copyFlags = 0;
}

void TabRegionalPeopleCountingSettings::onLanguageChanged()
{
    ui->labelRegionNumber->setText(GET_TEXT("REGIONAL_PEOPLECOUNTING/103311", "Region No."));
    ui->labelRegionalEnable->setText(GET_TEXT("REGIONAL_PEOPLECOUNTING/103313", "Regional People Counting"));
    ui->labelSetRegion->setText(GET_TEXT("REGIONAL_PEOPLECOUNTING/103312", "Region"));
    ui->labelObjectSizeLimits->setText(GET_TEXT("SMARTEVENT/55063", "Object Size Limits"));
    ui->labelMaxStay->setText(GET_TEXT("REGIONAL_PEOPLECOUNTING/103303", "Max. Stay"));
    ui->labelMinStay->setText(GET_TEXT("REGIONAL_PEOPLECOUNTING/103304", "Min. Stay"));
    ui->labelMaxLength->setText(GET_TEXT("REGIONAL_PEOPLECOUNTING/103305", "Max. Length of Stay"));
    ui->labelSensitivity->setText(GET_TEXT("MOTION/51003", "Sensitivity"));
    ui->labelAction->setText(GET_TEXT("VIDEOLOSS/50002", "Action"));
    ui->labelRegionalPeopleCountingSchedule->setText(GET_TEXT("REGIONAL_PEOPLECOUNTING/158005", "Regional People Counting Schedule"));
    ui->pushButtonEditAction->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->pushButtonRegionalPeopleCountingSchedule->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->pushButtonRegionSetAll->setText(GET_TEXT("MOTION/51004", "Set All"));
    ui->pushButtonRegionDelete->setText(GET_TEXT("CHANNELMANAGE/30023", "Delete"));
    ui->pushButtonObjectSizeReset->setText(GET_TEXT("COMMON/1057", "Reset"));
    ui->pushButtonApply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->checkBoxEnable->setText(GET_TEXT("COMMON/1009", "Enable"));

    ui->pushButtonCopy->setText(GET_TEXT("COMMON/1005", "Copy"));
}

void TabRegionalPeopleCountingSettings::onChannelButtonClicked(int index)
{
    MsWaittingContainer waitting(MsWaitting::instance());

    if (m_action) {
        m_action->clearCache();
    }
    if (m_effectiveTime) {
        m_effectiveTime->clearCache();
    }

    ui->widgetMinLineBox->setValid(true);
    ui->widgetMaxLineBox->setValid(true);

    m_currentChannel = index;
    m_copyFlags = 0;
    ui->commonVideo->playVideo(m_currentChannel);

    m_currentRegion = -1;
    ui->checkableLineEditMaxStay->clearCheck();
    ui->checkableLineEditMinStay->clearCheck();
    ui->checkableLineEditMaxLength->clearCheck();
    MsCameraVersion cameraVersion = MsCameraVersion::fromChannel(m_currentChannel);
    if ((cameraVersion >= MsCameraVersion(8, 3, 9) && qMsNvr->isNT(m_currentChannel))
    || cameraVersion >= MsCameraVersion(8, 4)) {
        m_objectSizeType = DrawSceneObjectSize::After_8_0_3_r9;
        ui->widgetMinLineBox->setWidthRange(8, 608);
        ui->widgetMinLineBox->setHeightRange(8, 352);
        ui->widgetMaxLineBox->setWidthRange(8, 608);
        ui->widgetMaxLineBox->setHeightRange(8, 352);
        ui->radioButtonMinLayout->setText(GET_TEXT("STATUS/177014", "Minimum Size(%1x%2~%3x%4)").arg(8).arg(8).arg(608).arg(352));
        ui->radioButtonMaxLayout->setText(GET_TEXT("STATUS/177015", "Maximum Size(%1x%2~%3x%4)").arg(8).arg(8).arg(608).arg(352));
    } else {
        m_objectSizeType = DrawSceneObjectSize::Before_8_0_3_r9;
        ui->widgetMinLineBox->setWidthRange(1, 320);
        ui->widgetMinLineBox->setHeightRange(1, 240);
        ui->widgetMaxLineBox->setWidthRange(1, 320);
        ui->widgetMaxLineBox->setHeightRange(1, 240);
        ui->radioButtonMinLayout->setText(GET_TEXT("SMARTEVENT/55052", "Minimum Size(1x1~%1x%2)").arg(320).arg(240));
        ui->radioButtonMaxLayout->setText(GET_TEXT("SMARTEVENT/55053", "Maximum Size(1x1~%1x%2)").arg(320).arg(240));
    }

    memset(m_regionInfo, 0, sizeof(MsIpcRegionalPeople));
    sendMessage(REQUEST_FLAG_GET_IPC_REGIONAL_PEOPLE, &m_currentChannel, sizeof(m_currentChannel));
    gEventLoopExec();
    showDebugInfo("====get data====");

    do {
        if (m_regionInfo->result == -1) {
            ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
            break;
        }
        if (m_regionInfo->result == -2) {
            ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
            break;
        }
        ui->widgetMessage->hideMessage();
        ui->widgetSettings->setEnabled(true);
        ui->pushButtonApply->setEnabled(true);
        ui->pushButtonCopy->setEnabled(true);

        ui->widgetMinLineBox->setWidthValue(m_regionInfo->minObjWidth);
        ui->widgetMinLineBox->setHeightValue(m_regionInfo->minObjHeight);
        ui->widgetMaxLineBox->setWidthValue(m_regionInfo->maxObjWidth);
        ui->widgetMaxLineBox->setHeightValue(m_regionInfo->maxObjHeight);

        m_regionScene->setRegions(m_regionInfo->regional, MAX_LEN_4);
        m_regionScene->clearAllEditState();
        ui->comboBoxRegion->setCurrentIndex(0);
        ui->sliderSensitivity->setValue(m_regionInfo->sensitivity);
        return;
    } while (0);

    m_regionScene->clearAll();
    ui->comboBoxRegion->setCurrentIndex(0);
    ui->sliderSensitivity->setValue(1);
    ui->pushButtonRegionEdit->setButtonState(PushButtonEditState::StateFinished);
    ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateFinished);
    ui->widgetSettings->setEnabled(false);
    ui->pushButtonApply->setEnabled(false);
    ui->pushButtonCopy->setEnabled(false);
}

void TabRegionalPeopleCountingSettings::onSceneObjectSizeChanged(const QRect &rc, MsQtVca::ObjectSizeType sizeType)
{
    switch (sizeType) {
    case MsQtVca::MinSize:
        m_regionInfo->minObjX = rc.x();
        m_regionInfo->minObjY = rc.y();
        m_regionInfo->minObjWidth = rc.width();
        m_regionInfo->minObjHeight = rc.height();

        ui->widgetMinLineBox->setWidthValue(rc.width());
        ui->widgetMinLineBox->setHeightValue(rc.height());
        break;
    case MsQtVca::MaxSize:
        m_regionInfo->maxObjX = rc.x();
        m_regionInfo->maxObjY = rc.y();
        m_regionInfo->maxObjWidth = rc.width();
        m_regionInfo->maxObjHeight = rc.height();

        ui->widgetMaxLineBox->setWidthValue(rc.width());
        ui->widgetMaxLineBox->setHeightValue(rc.height());
        break;
    default:
        break;
    }
}

void TabRegionalPeopleCountingSettings::on_radioButtonMinLayout_clicked(bool checked)
{
    if (!checked) {
        ui->radioButtonMinLayout->setChecked(true);
        return;
    } else {
        ui->radioButtonMaxLayout->setChecked(false);
    }
    m_objScene->showMinimumObjectSize(m_regionInfo, m_objectSizeType);
}

void TabRegionalPeopleCountingSettings::on_radioButtonMaxLayout_clicked(bool checked)
{
    if (!checked) {
        ui->radioButtonMaxLayout->setChecked(true);
    } else {
        ui->radioButtonMinLayout->setChecked(false);
    }
    m_objScene->showMaximumObjectSize(m_regionInfo, m_objectSizeType);
}

void TabRegionalPeopleCountingSettings::onLineEditMinWidthEditingFinished()
{
    int value = ui->widgetMinLineBox->widthValue();
    if (value < 1) {
        return;
    }
    m_regionInfo->minObjWidth = value;
    if (ui->radioButtonMinLayout->isChecked()) {
        m_objScene->showMinimumObjectSize(m_regionInfo, m_objectSizeType);
    }
}

void TabRegionalPeopleCountingSettings::onLineEditMinHeightEditingFinished()
{
    int value = ui->widgetMinLineBox->heightValue();
    if (value < 1) {
        return;
    }
    m_regionInfo->minObjHeight = value;
    if (ui->radioButtonMinLayout->isChecked()) {
        m_objScene->showMinimumObjectSize(m_regionInfo, m_objectSizeType);
    }
}

void TabRegionalPeopleCountingSettings::onLineEditMaxWidthEditingFinished()
{
    int value = ui->widgetMaxLineBox->widthValue();
    if (value < 1) {
        return;
    }
    m_regionInfo->maxObjHeight = value;
    if (ui->radioButtonMaxLayout->isChecked()) {
        m_objScene->showMaximumObjectSize(m_regionInfo, m_objectSizeType);
    }
}

void TabRegionalPeopleCountingSettings::onLineEditMaxHeightEditingFinished()
{
    int value = ui->widgetMaxLineBox->heightValue();
    if (value < 1) {
        return;
    }
    m_regionInfo->maxObjWidth = value;
    if (ui->radioButtonMaxLayout->isChecked()) {
        m_objScene->showMaximumObjectSize(m_regionInfo, m_objectSizeType);
    }
}

void TabRegionalPeopleCountingSettings::on_pushButtonObjectSizeEdit_clicked()
{
    switch (ui->pushButtonObjectSizeEdit->buttonState()) {
    case PushButtonEditState::StateFinished:
        ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateEditing);
        break;
    case PushButtonEditState::StateEditing:
        ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateFinished);
        break;
    }
}

void TabRegionalPeopleCountingSettings::on_pushButtonObjectSizeEdit_stateSet(PushButtonEditState::State state)
{
    switch (state) {
    case PushButtonEditState::StateFinished:
        ui->pushButtonObjectSizeReset->hide();
        ui->widgetSize->setEnabled(false);
        ui->radioButtonMinLayout->setChecked(false);
        ui->radioButtonMaxLayout->setChecked(false);
        ui->commonVideo->setDrawScene(m_regionScene);
        break;
    case PushButtonEditState::StateEditing:
        ui->pushButtonObjectSizeReset->show();
        ui->widgetSize->setEnabled(true);
        ui->radioButtonMinLayout->setChecked(true);
        ui->radioButtonMaxLayout->setChecked(false);
        ui->commonVideo->setDrawScene(m_objScene);
        m_objScene->showMinimumObjectSize(m_regionInfo, m_objectSizeType);

        ui->pushButtonRegionEdit->setButtonState(PushButtonEditState::StateFinished);
        break;
    }
}

void TabRegionalPeopleCountingSettings::on_pushButtonObjectSizeReset_clicked()
{
    int minw,minh,maxw,maxh;
    if (m_objectSizeType == DrawSceneObjectSize::Before_8_0_3_r9) {
        minw = 3;
        minh = 3;
        maxw = 320;
        maxh = 240;
    } else {
        minw = 8;
        minh = 8;
        maxw = 608;
        maxh = 352;
    }
    m_regionInfo->minObjX = 0;
    m_regionInfo->minObjY = 0;
    m_regionInfo->minObjWidth = minw;
    m_regionInfo->minObjHeight = minh;
    m_regionInfo->maxObjX = 0;
    m_regionInfo->maxObjY = 0;
    m_regionInfo->maxObjWidth = maxw;
    m_regionInfo->maxObjHeight = maxh;

    ui->widgetMinLineBox->setWidthValue(minw);
    ui->widgetMinLineBox->setHeightValue(minh);
    ui->widgetMaxLineBox->setWidthValue(maxw);
    ui->widgetMaxLineBox->setHeightValue(maxh);

    if (ui->radioButtonMinLayout->isChecked()) {
        m_objScene->showMinimumObjectSize(m_regionInfo, m_objectSizeType);
    }
    if (ui->radioButtonMaxLayout->isChecked()) {
        m_objScene->showMaximumObjectSize(m_regionInfo, m_objectSizeType);
    }
}

void TabRegionalPeopleCountingSettings::onDrawRegionConflicted()
{
    MessageBox::information(this, GET_TEXT("SMARTEVENT/55066", "The boundaries of the area cannot intersect. Please redraw."));
    m_regionScene->clearPolygon();
}

void TabRegionalPeopleCountingSettings::on_comboBoxRegion_indexSet(int index)
{
    ui->pushButtonRegionEdit->setButtonState(PushButtonEditState::StateFinished);
    ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateFinished);

    if (m_currentRegion >= 0) {
        if (!saveRegionInfo(m_currentRegion)) {
            ui->comboBoxRegion->beginEdit();
            ui->comboBoxRegion->setCurrentIndex(m_currentRegion);
            ui->comboBoxRegion->endEdit();
            return;
        }
    }
    m_currentRegion = index;
    m_regionScene->setCurrentIndex(index);

    MsIpcRegionalArea &regional = m_regionInfo->regional[index];
    ui->checkBoxEnable->setChecked(regional.enable);
    ui->checkableLineEditMaxStay->setChecked(regional.maxStayEnable);
    ui->checkableLineEditMaxStay->setValue(regional.maxStayValue);
    ui->checkableLineEditMinStay->setChecked(regional.minStayEnable);
    ui->checkableLineEditMinStay->setValue(regional.minStayValue);
    ui->checkableLineEditMaxLength->setChecked(regional.maxStayTimeEnable);
    ui->checkableLineEditMaxLength->setValue(regional.maxStayTimeValue);
}

void TabRegionalPeopleCountingSettings::on_checkBoxEnable_checkStateSet(int state)
{
    bool checked = state == Qt::Checked;
    if (!checked) {
        ui->pushButtonRegionEdit->setButtonState(PushButtonEditState::StateFinished);
        ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateFinished);
    }
    ui->pushButtonEditAction->setEnabled(checked);
    ui->pushButtonRegionalPeopleCountingSchedule->setEnabled(checked);
    ui->pushButtonRegionEdit->setEnabled(checked);
    ui->pushButtonObjectSizeEdit->setEnabled(checked);
    ui->checkableLineEditMaxStay->setEnabled(checked);
    ui->checkableLineEditMinStay->setEnabled(checked);
    ui->checkableLineEditMaxLength->setEnabled(checked);
    ui->sliderSensitivity->setEnabled(checked);
}

void TabRegionalPeopleCountingSettings::on_pushButtonRegionEdit_clicked()
{
    switch (ui->pushButtonRegionEdit->buttonState()) {
    case PushButtonEditState::StateFinished:
        ui->pushButtonRegionEdit->setButtonState(PushButtonEditState::StateEditing);
        break;
    case PushButtonEditState::StateEditing:
        ui->pushButtonRegionEdit->setButtonState(PushButtonEditState::StateFinished);
        break;
    }
}

void TabRegionalPeopleCountingSettings::on_pushButtonRegionEdit_stateSet(PushButtonEditState::State state)
{
    switch (state) {
    case PushButtonEditState::StateFinished:
        ui->pushButtonRegionSetAll->hide();
        ui->pushButtonRegionDelete->hide();
        m_regionScene->setEditable(false);
        break;
    case PushButtonEditState::StateEditing:
        ui->pushButtonRegionSetAll->show();
        ui->pushButtonRegionDelete->show();
        m_regionScene->setEditable(true);

        ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateFinished);
        break;
    }
}

void TabRegionalPeopleCountingSettings::on_pushButtonRegionSetAll_clicked()
{
    m_regionScene->selectAll();
}

void TabRegionalPeopleCountingSettings::on_pushButtonRegionDelete_clicked()
{
    m_regionScene->clearPolygon();
}

void TabRegionalPeopleCountingSettings::on_pushButtonEditAction_clicked()
{
    ui->pushButtonEditAction->clearUnderMouse();

    if (!m_action) {
        m_action = new ActionRegionPeopleCounting(this);
    }
    m_action->showAction(m_currentChannel, m_currentRegion);
}

void TabRegionalPeopleCountingSettings::updateDynamicDisplay(int type, int channel)
{
    Q_UNUSED(type)
    Q_UNUSED(channel)
}

void TabRegionalPeopleCountingSettings::on_pushButtonApply_clicked()
{
    //showWait();
    return;
    gFaceData.getFaceConfig(m_currentChannel);
    if (ui->checkBoxEnable->isChecked() && gFaceData.isFaceConflict()) {
        const int &result = MessageBox::question(this, GET_TEXT("FACE/141053", "Face Detection will be disabled, continue?"));
        if (result == MessageBox::Yes) {
            gFaceData.setFaceDisable();
        } else {
            //closeWait();
            clearCopyInfo();
            return;
        }
    }
    //closeWait();
    if (!saveRegionInfo(m_currentRegion)) {
        clearCopyInfo();
        return;
    }
    ui->pushButtonRegionEdit->setButtonState(PushButtonEditState::StateFinished);
    ui->pushButtonObjectSizeEdit->setButtonState(PushButtonEditState::StateFinished);

    if (ui->checkBoxEnable->isChecked()) {
        int minWidth = ui->widgetMinLineBox->widthValue();
        int minHeight = ui->widgetMinLineBox->heightValue();
        int maxWidth = ui->widgetMaxLineBox->widthValue();
        int maxHeight = ui->widgetMaxLineBox->heightValue();
        if (minWidth > maxWidth || minHeight > maxHeight) {
            MessageBox::information(this, GET_TEXT("SMARTEVENT/55010", "Minimum size must be smaller than Maximum size."));
            clearCopyInfo();
            return;
        }
    }

    m_regionScene->clearAllEditState();
    m_regionInfo->sensitivity = ui->sliderSensitivity->value();
    showDebugInfo("====set data====");

    MsWaittingContainer showWait;
    if (m_action && m_action->saveAction()) {
        //这里不用发送更新，REQUEST_FLAG_SET_IPC_REGIONAL_PEOPLE会一起更新Action
        //sendMessage(REQUEST_FLAG_UPDATE_REGIONAL_ACTION, &m_currentChannel, sizeof(m_currentChannel));
    }
    //判断是否需要copy action
    if (m_copyFlags != 0) {
        if (!m_action) {
            m_action = new ActionRegionPeopleCounting(this);
        }
        m_action->copyAction(m_currentChannel, m_copyFlags);
    }
    sendMessage(REQUEST_FLAG_SET_IPC_REGIONAL_PEOPLE, m_regionInfo, sizeof(MsIpcRegionalPeople));
    gEventLoopExec();
}

void TabRegionalPeopleCountingSettings::on_pushButtonBack_clicked()
{
    back();
}

void TabRegionalPeopleCountingSettings::on_pushButtonCopy_clicked()
{
    ChannelCopyDialog copy(this);
    copy.setCurrentChannel(m_currentChannel);
    int result = copy.exec();
    if (result == ChannelCopyDialog::Accepted) {
        QList<int> copyList = copy.checkedList(false);
        m_copyFlags = copy.checkedFlags(false);
        for (int i = 0; i < MAX_LEN_65; i++) {
            if (copyList.contains(i)) {
                m_regionInfo->copyChn[i] = '1';
            } else {
                m_regionInfo->copyChn[i] = '0';
            }
        }
        on_pushButtonApply_clicked();
    }
}

void TabRegionalPeopleCountingSettings::on_pushButtonRegionalPeopleCountingSchedule_clicked()
{
    if (!m_effectiveTime) {
        m_effectiveTime = new EffectiveTimePeopleCounting(RegionalPeopleCountingSettingsMode, this);
    }
    m_effectiveTime->setSchedule(m_regionInfo->sche.schedule_day);
    m_effectiveTime->showEffectiveTime(m_currentChannel);
    int result = m_effectiveTime->exec();
    if (result == QDialog::Accepted) {
        m_effectiveTime->getSchedule(m_regionInfo->sche.schedule_day);
    }
    ui->pushButtonRegionalPeopleCountingSchedule->clearUnderMouse();
}
