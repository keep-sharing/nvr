#include "CameraAudio.h"
#include "ui_CameraAudio.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "channelcopydialog.h"
#include "LiveView.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"

CameraAudio::CameraAudio(QWidget *parent)
    : AbstractCameraPage(parent)
    , ui(new Ui::CameraAudio)
{
    ui->setupUi(this);

    ui->channelGroup->setCount(qMsNvr->maxChannel());
    connect(ui->channelGroup, SIGNAL(buttonClicked(int)), this, SLOT(onChannelGroupClicked(int)));

    ui->labelAudioEnable->setText(GET_TEXT("CAMERAAUDIO/107001", "Audio"));
    ui->comboBoxEnable->clear();
    ui->comboBoxEnable->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);
    ui->comboBoxEnable->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);

    ui->labelAudioMode->setText(GET_TEXT("CAMERAAUDIO/107002", "Audio Mode"));
    ui->comboBoxAudioMode->clear();
    ui->comboBoxAudioMode->addItem(GET_TEXT("CAMERAAUDIO/107003", "Audio Input"), 0);
    ui->comboBoxAudioMode->addItem(GET_TEXT("CAMERAAUDIO/107004", "Audio Output"), 1);
    ui->comboBoxAudioMode->addItem(GET_TEXT("CAMERAAUDIO/107005", "Both Audio Input & Output"), 2);

    ui->labelAudioInput->setText(GET_TEXT("CAMERAAUDIO/107003", "Audio Input"));

    ui->labelDenoise->setText(GET_TEXT("CAMERAAUDIO/107006", "Denoise"));
    ui->comboBoxDenoise->clear();
    ui->comboBoxDenoise->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);
    ui->comboBoxDenoise->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);

    ui->labelEncoding->setText(GET_TEXT("CAMERAAUDIO/107007", "Encoding"));
    ui->comboBoxEncoding->clear();
    ui->comboBoxEncoding->addItem("G.711-ULaw", ENCODE_G711_ULAW);
    ui->comboBoxEncoding->addItem("G.711-ALaw", ENCODE_G711_ALAW);
    ui->comboBoxEncoding->addItem("AAC LC", ENCODE_AAC_LC);
    ui->comboBoxEncoding->addItem("G.722", ENCODE_G722);
    ui->comboBoxEncoding->addItem("G.726", ENCODE_G726);

    ui->labelInputMode->setText(GET_TEXT("CAMERAAUDIO/107008", "Input Mode"));
    ui->comboBoxInputMode->clear();
    ui->comboBoxInputMode->addItem(GET_TEXT("CAMERAAUDIO/107009", "Mic Input"), 0);
    ui->comboBoxInputMode->addItem(GET_TEXT("CAMERAAUDIO/107010", "Line Input"), 1);

    ui->labelSampleRate->setText(GET_TEXT("CAMERAAUDIO/107011", "Sample Rate"));

    ui->labelBitRate->setText(GET_TEXT("CAMERAAUDIO/107015", "Audio Bit Rate"));

    ui->sliderInputGain->setRange(0, 100);

    ui->labelAudioOutput->setText(GET_TEXT("CAMERAAUDIO/107004", "Audio Output"));

    ui->labelInputGain->setText(GET_TEXT("CAMERAAUDIO/107012", "Input Gain"));

    ui->labelAutoGainControl->setText(GET_TEXT("CAMERAAUDIO/107013", "Auto Gain Control"));
    ui->comboBoxAutoGainControl->clear();
    ui->comboBoxAutoGainControl->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);
    ui->comboBoxAutoGainControl->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);

    ui->labelOutputVolume->setText(GET_TEXT("CAMERAAUDIO/107014", "Output Volume"));
    ui->sliderOutputVolume->setRange(0, 100);

    ui->pushButtonCopy->setText(GET_TEXT("COMMON/1005", "Copy"));
    ui->pushButtonApply->setText(GET_TEXT("COMMON/1003", "Apply"));
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
}

CameraAudio::~CameraAudio()
{
    delete ui;
}

void CameraAudio::initializeData()
{
    ui->channelGroup->setCurrentIndex(0);
}

void CameraAudio::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPC_AUDIO_INFO:
        ON_RESPONSE_FLAG_GET_IPC_AUDIO_INFO(message);
        break;
    case RESPONSE_FLAG_SET_IPC_AUDIO_INFO:
        ON_RESPONSE_FLAG_SET_IPC_AUDIO_INFO(message);
        break;
    }
}

void CameraAudio::ON_RESPONSE_FLAG_GET_IPC_AUDIO_INFO(MessageReceive *message)
{
    ipc_audio_info *audio_info = static_cast<ipc_audio_info *>(message->data);
    memset(&m_audio_info, 0, sizeof(ipc_audio_info));
    if (!audio_info) {
        qMsWarning() << "data is nullprt.";
    } else {
        memcpy(&m_audio_info, audio_info, sizeof(ipc_audio_info));
        qMsDebug() << QString("res: %1, support: %2, supportMode: %3, supportInputMode: %4").arg(audio_info->res).arg(audio_info->support).arg(audio_info->supportMode).arg(audio_info->supportInputMode)
                   << "\n--config--"
                   << "\n----chnid:" << audio_info->config.chnid
                   << "\n----audioEnable:" << audio_info->config.audioEnable
                   << "\n----audioMode:" << audio_info->config.audioMode
                   << "\n----denoise:" << audio_info->config.denoise
                   << "\n----encoding:" << audio_info->config.encoding
                   << "\n----sampleRate:" << audio_info->config.sampleRate
                   << "\n----inputGain:" << audio_info->config.inputGain
                   << "\n----inputMode:" << audio_info->config.inputMode
                   << "\n----audioGainControl:" << audio_info->config.audioGainControl
                   << "\n----outputVolume:" << audio_info->config.outputVolume
                   << "\n----audioBitrate:" << audio_info->config.audioBitrate
                   << "\n--option--"
                   << "\n----encodeNumber:" << audio_info->options.encodeNumber;
        if (audio_info->res == -1) {
            ShowMessageBox(GET_TEXT("COMMONBACKUP/151001", "Failed to get parameters."));
        } else {
            if (audio_info->support == 1) {
                //
                switch (audio_info->supportMode) {
                case 0:
                    setAudioOutputVisible(false);
                    break;
                case 1:
                    setAudioOutputVisible(true);
                    break;
                default:
                    break;
                }
                //
                switch (audio_info->supportInputMode) {
                case 0:
                    ui->labelInputMode->hide();
                    ui->comboBoxInputMode->hide();
                    break;
                case 1:
                    ui->labelInputMode->show();
                    ui->comboBoxInputMode->show();
                    break;
                default:
                    break;
                }

                //
                ui->pushButtonCopy->setEnabled(true);
                ui->pushButtonApply->setEnabled(true);

                ui->comboBoxEnable->setEnabled(true);
                ui->comboBoxEnable->setCurrentIndexFromData(audio_info->config.audioEnable);

                ui->comboBoxAudioMode->setCurrentIndexFromData(audio_info->config.audioMode);
                ui->comboBoxDenoise->setCurrentIndexFromData(audio_info->config.denoise);
                showEncodingComboBox();
                ui->comboBoxInputMode->setCurrentIndexFromData(audio_info->config.inputMode);
                ui->sliderInputGain->setValue(audio_info->config.inputGain);
                //
                ui->comboBoxAutoGainControl->setCurrentIndexFromData(audio_info->config.audioGainControl);
                ui->sliderOutputVolume->setValue(audio_info->config.outputVolume);
            } else {
                ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
            }
        }
    }
    //MsWaitting::closeGlobalWait();
}

void CameraAudio::ON_RESPONSE_FLAG_SET_IPC_AUDIO_INFO(MessageReceive *message)
{
    int result = *(int *)message->data;
    qMsDebug() << "result:" << result;
    m_eventLoop.exit();
}

void CameraAudio::setSettingEnable(bool enable)
{
    ui->comboBoxAudioMode->setEnabled(enable);
    ui->comboBoxDenoise->setEnabled(enable);
    ui->comboBoxEncoding->setEnabled(enable);
    ui->comboBoxInputMode->setEnabled(enable);
    ui->comboBoxSampleRate->setEnabled(enable);
    ui->comboBoxAudioBitRate->setEnabled(enable);
    ui->sliderInputGain->setEnabled(enable);
    ui->comboBoxAutoGainControl->setEnabled(enable);
    ui->sliderOutputVolume->setEnabled(enable);
}

void CameraAudio::clearSetting()
{
    ui->comboBoxEnable->setCurrentIndexFromData(0);
}

void CameraAudio::setAudioOutputVisible(bool visible)
{
    ui->labelAudioMode->setVisible(visible);
    ui->comboBoxAudioMode->setVisible(visible);
    ui->labelAudioOutput->setVisible(visible);
    ui->labelAutoGainControl->setVisible(visible);
    ui->comboBoxAutoGainControl->setVisible(visible);
    ui->labelOutputVolume->setVisible(visible);
    ui->sliderOutputVolume->setVisible(visible);
}

void CameraAudio::showEncodingComboBox()
{
    ui->comboBoxEncoding->beginEdit();
    ui->comboBoxEncoding->clear();
    QMap<int, QString> encodingMap;
    for (int i = 0; i < m_audio_info.options.encodeNumber; ++i) {
        const ipc_audio_encode &encode = m_audio_info.options.encodeInfo[i];
        switch (encode.encode) {
        case ENCODE_AAC_LC:
            encodingMap.insert(encode.encode, "AAC LC");
            break;
        case ENCODE_G711_ULAW:
            encodingMap.insert(encode.encode, "G.711-ULaw");
            break;
        case ENCODE_G711_ALAW:
            encodingMap.insert(encode.encode, "G.711-ALaw");
            break;
        case ENCODE_G722:
            encodingMap.insert(encode.encode, "G.722");
            break;
        case ENCODE_G726:
            encodingMap.insert(encode.encode, "G.726");
            break;
        default:
            qMsWarning() << QString("unknow encode: %1").arg(encode.encode);
            break;
        }
    }
    //为了排序
    if (encodingMap.contains(ENCODE_G711_ULAW)) {
        ui->comboBoxEncoding->addItem(encodingMap.value(ENCODE_G711_ULAW), ENCODE_G711_ULAW);
    }
    if (encodingMap.contains(ENCODE_G711_ALAW)) {
        ui->comboBoxEncoding->addItem(encodingMap.value(ENCODE_G711_ALAW), ENCODE_G711_ALAW);
    }
    if (encodingMap.contains(ENCODE_AAC_LC)) {
        ui->comboBoxEncoding->addItem(encodingMap.value(ENCODE_AAC_LC), ENCODE_AAC_LC);
    }
    if (encodingMap.contains(ENCODE_G722)) {
        ui->comboBoxEncoding->addItem(encodingMap.value(ENCODE_G722), ENCODE_G722);
    }
    if (encodingMap.contains(ENCODE_G726)) {
        ui->comboBoxEncoding->addItem(encodingMap.value(ENCODE_G726), ENCODE_G726);
    }
    ui->comboBoxEncoding->endEdit();
    //
    ui->comboBoxEncoding->setCurrentIndexFromData(m_audio_info.config.encoding);
}

void CameraAudio::showSampleRateComboBox()
{
    ui->comboBoxSampleRate->beginEdit();
    ui->comboBoxSampleRate->clear();
    int currentEncode = ui->comboBoxEncoding->currentData().toInt();
    switch (currentEncode) {
    case ENCODE_G722:
    case ENCODE_G726:
        ui->comboBoxSampleRate->setEnabled(false);
        break;
    default:
        if (ui->comboBoxEnable->currentData().toInt() == 1) {
            ui->comboBoxSampleRate->setEnabled(true);
        } else {
            ui->comboBoxSampleRate->setEnabled(false);
        }
        break;
    }
    for (int i = 0; i < m_audio_info.options.encodeNumber; ++i) {
        const ipc_audio_encode &encode = m_audio_info.options.encodeInfo[i];
        if (encode.encode == currentEncode) {
            for (int i = 0; i < encode.sampleRateNumber; ++i) {
                const ipc_audio_samplerate &sampleate = encode.sampleRate[i];
                switch (sampleate.sampleRate) {
                case SAMPLERATE_8KHZ:
                    ui->comboBoxSampleRate->addItem("8KHz", sampleate.sampleRate);
                    break;
                case SAMPLERATE_16KHZ:
                    ui->comboBoxSampleRate->addItem("16KHz", sampleate.sampleRate);
                    break;
                case SAMPLERATE_32KHZ:
                    ui->comboBoxSampleRate->addItem("32KHz", sampleate.sampleRate);
                    break;
                case SAMPLERATE_44KHZ:
                    ui->comboBoxSampleRate->addItem("44.1KHz", sampleate.sampleRate);
                    break;
                case SAMPLERATE_48KHZ:
                    ui->comboBoxSampleRate->addItem("48KHz", sampleate.sampleRate);
                    break;
                default:
                    qMsWarning() << QString("unknow sampleate: %1").arg(sampleate.sampleRate);
                    break;
                }
            }
            break;
        }
    }
    ui->comboBoxSampleRate->endEdit();
    //
    int index = ui->comboBoxSampleRate->findData(m_audio_info.config.sampleRate);
    if (index < 0) {
        ui->comboBoxSampleRate->setCurrentIndex(0);
    } else {
        ui->comboBoxSampleRate->setCurrentIndexFromData(m_audio_info.config.sampleRate);
    }
}

void CameraAudio::showAudioBitRateComboBox()
{
    int currentEncode = ui->comboBoxEncoding->currentData().toInt();
    if (currentEncode != ENCODE_AAC_LC) {
        ui->labelBitRate->hide();
        ui->comboBoxAudioBitRate->hide();
        return;
    }
    ui->comboBoxAudioBitRate->beginEdit();
    ui->comboBoxAudioBitRate->clear();
    int currentSampleRate = ui->comboBoxSampleRate->currentData().toInt();
    for (int i = 0; i < m_audio_info.options.encodeNumber; ++i) {
        const ipc_audio_encode &encode = m_audio_info.options.encodeInfo[i];
        if (encode.encode == currentEncode) {
            qMsDebug() << "encode:" << encode.encode << ", sampleRateNumber:" << encode.sampleRateNumber;
            for (int j = 0; j < encode.sampleRateNumber; ++j) {
                const ipc_audio_samplerate &sampleate = encode.sampleRate[j];
                qMsDebug() << sampleate.sampleRate << "," << currentSampleRate;
                if (sampleate.sampleRate == currentSampleRate) {
                    qMsDebug() << sampleate.bitrateNumber;
                    for (int k = 0; k < sampleate.bitrateNumber; ++k) {
                        int bitRate = sampleate.bitrateValue[k];
                        ui->comboBoxAudioBitRate->addItem(QString("%1kbps").arg(bitRate), bitRate);
                    }
                    if (sampleate.bitrateNumber > 0) {
                        ui->labelBitRate->show();
                        ui->comboBoxAudioBitRate->show();
                    } else {
                        ui->labelBitRate->hide();
                        ui->comboBoxAudioBitRate->hide();
                    }
                    break;
                }
            }
            break;
        }
    }
    ui->comboBoxAudioBitRate->endEdit();
    //
    int bitRateIndex = ui->comboBoxAudioBitRate->findData(m_audio_info.config.audioBitrate);
    if (bitRateIndex == -1) {
        switch (currentSampleRate) {
        case SAMPLERATE_8KHZ:
            ui->comboBoxAudioBitRate->setCurrentIndexFromData(24);
            break;
        case SAMPLERATE_16KHZ:
            ui->comboBoxAudioBitRate->setCurrentIndexFromData(48);
            break;
        case SAMPLERATE_32KHZ:
            ui->comboBoxAudioBitRate->setCurrentIndexFromData(96);
            break;
        case SAMPLERATE_44KHZ:
            ui->comboBoxAudioBitRate->setCurrentIndexFromData(128);
            break;
        case SAMPLERATE_48KHZ:
            ui->comboBoxAudioBitRate->setCurrentIndexFromData(144);
            break;
        }
    } else {
        ui->comboBoxAudioBitRate->setCurrentIndexFromData(m_audio_info.config.audioBitrate);
    }
}

void CameraAudio::saveAudioSettings(const QList<int> &channels)
{
    req_ipc_audio_config set_config;
    memset(&set_config, 0, sizeof(req_ipc_audio_config));
    for (int i = 0; i < channels.size(); ++i) {
        int channel = channels.at(i);
        set_config.channels |= ((quint64)1 << channel);
    }
    memcpy(&set_config.info, &m_audio_info.config, sizeof(ipc_audio_config));
    ipc_audio_config &audio_config = set_config.info;
    audio_config.chnid = m_currentChannel;
    audio_config.audioEnable = ui->comboBoxEnable->currentData().toInt();
    audio_config.audioMode = ui->comboBoxAudioMode->currentData().toInt();
    audio_config.denoise = ui->comboBoxDenoise->currentData().toInt();
    audio_config.encoding = ui->comboBoxEncoding->currentData().toInt();
    audio_config.sampleRate = ui->comboBoxSampleRate->currentData().toInt();
    audio_config.inputGain = ui->sliderInputGain->value();
    audio_config.inputMode = ui->comboBoxInputMode->currentData().toInt();
    audio_config.audioGainControl = ui->comboBoxAutoGainControl->currentData().toInt();
    audio_config.outputVolume = ui->sliderOutputVolume->value();
    audio_config.audioBitrate = ui->comboBoxAudioBitRate->currentData().toInt();
    //MsWaitting::showGlobalWait();
    sendMessage(REQUEST_FLAG_SET_IPC_AUDIO_INFO, &set_config, sizeof(req_ipc_audio_config));
    //m_eventLoop.exec();
    //MsWaitting::closeGlobalWait();
}

void CameraAudio::onChannelGroupClicked(int channel)
{
    memset(&m_audio_info, 0, sizeof(ipc_audio_info));

    m_currentChannel = channel;
    ui->video->playVideo(m_currentChannel);

    clearSetting();
    ui->comboBoxEnable->setEnabled(false);

    ui->pushButtonCopy->setEnabled(false);
    ui->pushButtonApply->setEnabled(false);

    ui->widgetMessage->hideMessage();
    if (!LiveView::instance()->isChannelConnected(m_currentChannel)) {
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20102", "This channel is not connected."));
        return;
    }

    if (!qMsNvr->isMsCamera(m_currentChannel)) {
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
        return;
    }

    //考虑至Audio这块IPC经过多个版本变更，较为繁杂，故此次NVR仅从IPC 75-r5版本开始兼容，即低于75-r5版本的IPC提示This channel does not support this function.
    char version[50];
    //get_channel_version_name(m_currentChannel, version, sizeof(version));
    MsCameraVersion cameraVersion(version);
    MsCameraVersion supportVersion(7, 75, 5);
    qMsDebug() << "CameraVersion:" << cameraVersion
               << ", SupportVersion:" << supportVersion;
    if (cameraVersion < supportVersion) {
        ui->widgetMessage->showMessage(GET_TEXT("LIVEVIEW/20107", "This channel does not support this function."));
        return;
    }

    sendMessage(REQUEST_FLAG_GET_IPC_AUDIO_INFO, &m_currentChannel, sizeof(int));
    //MsWaitting::showGlobalWait();
}

void CameraAudio::on_comboBoxEnable_indexSet(int index)
{
    qMsDebug() << index;
    setSettingEnable(index == 0);
    ui->comboBoxEncoding->reSetIndex();
}

void CameraAudio::on_comboBoxEncoding_indexSet(int index)
{
    qMsDebug() << index;
    int currentEncode = ui->comboBoxEncoding->currentData().toInt();
    if (currentEncode != ENCODE_AAC_LC) {
        ui->labelBitRate->hide();
        ui->comboBoxAudioBitRate->hide();
    } else {
        ui->labelBitRate->show();
        ui->comboBoxAudioBitRate->show();
    }
    showSampleRateComboBox();
}

void CameraAudio::on_comboBoxSampleRate_indexSet(int index)
{
    qMsDebug() << index;
    showAudioBitRateComboBox();
}

void CameraAudio::on_pushButtonCopy_clicked()
{
    ChannelCopyDialog copy(this);
    copy.setCurrentChannel(m_currentChannel);
    int result = copy.exec();
    if (result == QDialog::Accepted) {
        QList<int> copyList = copy.checkedList(false);
        saveAudioSettings(copyList);
    }
}

void CameraAudio::on_pushButtonApply_clicked()
{
    QList<int> channels;
    channels.append(m_currentChannel);
    saveAudioSettings(channels);
}

void CameraAudio::on_pushButtonBack_clicked()
{
    back();
}
