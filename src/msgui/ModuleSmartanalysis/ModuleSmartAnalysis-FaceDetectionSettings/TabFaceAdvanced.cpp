#include "TabFaceAdvanced.h"
#include "ui_TabFaceAdvanced.h"
#include "FaceAttributeRecognitionSettings.h"
#include "LiveView.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "centralmessage.h"

TabFaceAdvanced::TabFaceAdvanced(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::TabFaceAdvanced)
{
    ui->setupUi(this);
    ui->widgetButtonGroup->setCount(qMsNvr->maxChannel());
    connect(ui->widgetButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onChannelButtonClicked(int)));

    ui->comboBoxFacePrivacy->beginEdit();
    ui->comboBoxFacePrivacy->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);
    ui->comboBoxFacePrivacy->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBoxFacePrivacy->endEdit();

    hideMessage();
    m_faceSetting = new FaceAttributeRecognitionSettings(this);

    onLanguageChanged();
}

TabFaceAdvanced::~TabFaceAdvanced()
{
    delete ui;
}

void TabFaceAdvanced::initializeData()
{
    m_toolButtonClicked = false;
    ui->widgetButtonGroup->setCurrentIndex(currentChannel());
}

void TabFaceAdvanced::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_FACE_SUPPORT:
        ON_RESPONSE_FLAG_GET_FACE_SUPPORT(message);
        break;
    case RESPONSE_FLAG_GET_FACE_CONFIG:
        ON_RESPONSE_FLAG_GET_FACE_CONFIG(message);
        break;
    case RESPONSE_FLAG_SET_FACE_CONFIG:
        ON_RESPONSE_FLAG_SET_FACE_CONFIG(message);
        break;
    default:
        break;
    }
}

void TabFaceAdvanced::ON_RESPONSE_FLAG_GET_FACE_SUPPORT(MessageReceive *message)
{
    if (message->data) {
        m_isSupported = *(static_cast<bool *>(message->data));
    }
    m_eventLoop.exit();
}

void TabFaceAdvanced::ON_RESPONSE_FLAG_GET_FACE_CONFIG(MessageReceive *message)
{
    memset(&m_faceConfig, 0, sizeof(MsFaceConfig));
    struct MsFaceConfig *faceConfig = static_cast<MsFaceConfig *>(message->data);
    if (!faceConfig) {
        qWarning() << "data is null.";
        return;
    }
    memcpy(&m_faceConfig, faceConfig, sizeof(MsFaceConfig));
    qDebug() << QString("FaceCapturePage::ON_RESPONSE_FLAG_GET_FACE_CONFIG, channel: %1, enable: %2, min size: %3, captureMode: %4,")
                    .arg(m_faceConfig.chnId)
                    .arg(m_faceConfig.enable)
                    .arg(m_faceConfig.minPixel)
                    .arg(m_faceConfig.captureMode);
    ui->comboBoxFacePrivacy->setCurrentIndexFromData(m_faceConfig.mosaicEnable);
    if (m_faceConfig.captureMode != 0) {
        m_faceSetting->setNote(FaceAttributeRecognitionSettings::TypeModeError);
        m_faceSetting->setIsEnabled(false);
    } else if (m_faceConfig.mosaicEnable) {
        m_faceSetting->setNote(FaceAttributeRecognitionSettings::TypeFacePrivacyEnable);
        m_faceSetting->setIsEnabled(false);
    } else {
        m_faceSetting->setIsEnabled(true);
    }
    bool showFacePrivacy = !faceConfig->isNtPlatform || faceConfig->ntFaceMosaicSupport;
    ui->labelFacePrivacy->setVisible(showFacePrivacy);
    ui->widget_3->setVisible(showFacePrivacy);

    m_eventLoop.exit();
}

void TabFaceAdvanced::ON_RESPONSE_FLAG_SET_FACE_CONFIG(MessageReceive *message)
{
    Q_UNUSED(message)
}

void TabFaceAdvanced::hideMessage()
{
    ui->widgetMessage->hide();
}

void TabFaceAdvanced::showNotConnectedMessage()
{
    //closeWait();
    ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
}

void TabFaceAdvanced::showNotSupportMessage()
{
    //closeWait();
    ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
}

void TabFaceAdvanced::clearSetting()
{
    updateEnableState();
    ui->labelFacePrivacy->setVisible(true);
    ui->widget_3->setVisible(true);
}

void TabFaceAdvanced::saveData()
{
    //showWait();
    sendMessage(REQUEST_FLAG_SET_FACE_CONFIG, &m_faceConfig, sizeof(MsFaceConfig));
    // QEventLoop eventLoop;
    // QTimer::singleShot(2000, &eventLoop, SLOT(quit()));
    // eventLoop.exec();
    // m_hasApply = true;
    onChannelButtonClicked(currentChannel());
    //closeWait();
}

void TabFaceAdvanced::updateEnableState()
{
    ui->pushButtonEdit->setEnabled(m_isConnected && m_isSupported);
    ui->comboBoxFacePrivacy->setEnabled(m_isConnected && m_isSupported);
    ui->pushButtonApply->setEnabled(m_isConnected && m_isSupported);
    ui->toolButtonTip->setEnabled(m_isConnected && m_isSupported);
}

void TabFaceAdvanced::onLanguageChanged()
{
    ui->pushButtonEdit->setText(GET_TEXT("COMMON/1019", "Edit"));
    ui->labelAttroniteSettings->setText(GET_TEXT("FACE/141032", "Attribute Recognition Settings"));
    ui->labelFacePrivacy->setText(GET_TEXT("FACE/141033", "Face Privacy"));
    QString questionText = GET_TEXT("FACE/141056", "To enable Face Privacy Mode:")
                               .append("\n" + GET_TEXT("FACE/141057", "1. Video parameters will be adjusted to recommended settings automatically."))
                               .append("\n" + GET_TEXT("FACE/141058", "2. Face Capture and Attribute Recognition are not available."))
                               .append("\n" + GET_TEXT("FACE/141059", "3. Recognition results won't push to NVR."));
    ui->labelTip->setText(questionText);

    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButtonApply->setText(GET_TEXT("COMMON/1003", "Apply"));
}

void TabFaceAdvanced::onChannelButtonClicked(int index)
{
    if (!isVisible()) {
        return;
    }
    if (m_currentChannel != index || !m_hasApply) {
        m_toolButtonClicked = false;
    }
    m_hasApply = false;
    //
    m_faceSetting->clear();

    m_currentChannel = index;
    setCurrentChannel(m_currentChannel);
    ui->commonVideo->playVideo(m_currentChannel);

    m_isConnected = false;
    m_isSupported = false;

    ui->toolButtonTip->setChecked(m_toolButtonClicked);
    on_toolButtonTip_clicked(m_toolButtonClicked);

    //检查通道是否连接
    m_isConnected = LiveView::instance()->isChannelConnected(m_currentChannel);
    if (!m_isConnected) {
        showNotConnectedMessage();
        clearSetting();
        return;
    }
    //showWait();
    sendMessage(REQUEST_FLAG_GET_FACE_SUPPORT, &m_currentChannel, sizeof(int));
    //m_eventLoop.exec();
    if (!m_isSupported) {
        showNotSupportMessage();
        clearSetting();
        //closeWait();
        return;
    }
    hideMessage();
    updateEnableState();
    sendMessage(REQUEST_FLAG_GET_FACE_CONFIG, &m_currentChannel, sizeof(int));
    //m_eventLoop.exec();
    //closeWait();
}

void TabFaceAdvanced::on_pushButtonApply_clicked()
{
    if (ui->comboBoxFacePrivacy->currentIntData() == 1) {
        if (m_faceConfig.mosaicModeStatus != 0) {
            QString questionText = GET_TEXT("FACE/141056", "To enable Face Privacy Mode:")
                                       .append("\n" + GET_TEXT("FACE/141057", "1. Video parameters will be adjusted to recommended settings automatically."))
                                       .append("\n" + GET_TEXT("FACE/141058", "2. Face Capture and Attribute Recognition are not available."))
                                       .append("\n" + GET_TEXT("FACE/141059", "3. Recognition results won't push to NVR."))
                                       .append("\n\n" + GET_TEXT("FACE/141062", "Are you sure to continue?"));
            const int result = MessageBox::question(this, questionText);
            if (result == MessageBox::Yes) {
                m_faceConfig.mosaicModeStatus = 1;
                saveData();
            }
        } else {
            saveData();
        }

    } else {
        saveData();
    }
}

void TabFaceAdvanced::on_pushButtonBack_clicked()
{
    back();
}

void TabFaceAdvanced::on_pushButtonEdit_clicked()
{
    m_faceSetting->initializeData(m_faceConfig.attributeEnable, m_faceConfig.attributeType);
    int result = m_faceSetting->exec();
    if (result == QDialog::Accepted) {
        m_faceConfig.attributeEnable = m_faceSetting->getAttribute();
        m_faceConfig.attributeType = m_faceSetting->getAttributeType();
    }
    ui->pushButtonEdit->clearUnderMouse();
}

void TabFaceAdvanced::on_comboBoxFacePrivacy_currentIndexChanged(int index)
{
    int data = ui->comboBoxFacePrivacy->itemData(index).toInt();
    m_faceConfig.mosaicEnable = data;
}

void TabFaceAdvanced::on_toolButtonTip_clicked(bool checked)
{
    if (checked) {
        ui->toolButtonTip->setIcon(QIcon(":/common/common/note_hover.png"));
    } else {
        ui->toolButtonTip->setIcon(QIcon(":/common/common/note.png"));
    }
    ui->labelTip->setVisible(checked);
    m_toolButtonClicked = checked;
}
