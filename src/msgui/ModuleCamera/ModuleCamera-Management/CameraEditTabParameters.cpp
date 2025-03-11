#include "CameraEditTabParameters.h"
#include "ui_CameraEditTabParameters.h"
#include "CameraEdit.h"
#include "LogWrite.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "centralmessage.h"
#include "qjson/include/parser.h"

extern "C" {
#include "msg.h"

}

const int MinBitRate = 16;
const int MaxBitRate = 16384;

CameraEditTabParameters::CameraEditTabParameters(QWidget *parent)
    : AbstractSettingTab(parent)
    , ui(new Ui::CameraEditTabParameters)
{
    ui->setupUi(this);

    m_edit = static_cast<CameraEdit *>(parent);

    ui->slider_level_main->setRange(1, 10);
    ui->label_level_main->hide();
    ui->slider_level_main->hide();

    ui->slider_level_sub->setRange(1, 10);
    ui->label_level_sub->hide();
    ui->slider_level_sub->hide();

    connect(ui->checkBox_eventStream, SIGNAL(checkStateSet(int)), this, SLOT(onCheckBoxEventStreamCheckStateSet(int)));
    connect(ui->comboBox_bitRate_main, SIGNAL(currentBitRateSet(int)), this, SLOT(onComboBoxMainBitRateSet(int)));
    connect(ui->comboBox_bitRate_main, SIGNAL(currentBitRateEditingFinished(int)), this, SLOT(onComboBoxMainBitRateEditingFinished(int)));
    connect(ui->comboBox_bitRate_sub, SIGNAL(currentBitRateEditingFinished(int)), this, SLOT(onComboBoxSubBitRateEditingFinished(int)));
    onLanguageChanged();
}

CameraEditTabParameters::~CameraEditTabParameters()
{
    delete ui;
}

void CameraEditTabParameters::initializeData()
{
    clearSetting();
    qMsDebug() << "show json parameters";
    m_cameraStreamJsonData = new CameraStreamJsonData(m_edit->m_jsonData);
    //
    m_channel_previous = m_edit->m_channel;
    //
    //MsWaitting::showGlobalWait();

    //
    struct camera camera_previous;
    qMsNvr->readDatabaseCamera(&camera_previous, m_channel_previous);
    //
    memset(&m_model_info, 0, sizeof(CAM_MODEL_INFO));
    sendMessage(REQUEST_FLAG_GET_SINGLE_IPCTYPE, &m_channel_previous, sizeof(int));
    //m_eventLoop.exec();
    sendMessage(REQUEST_FLAG_GET_IPCIMAGE_DISPLAY, &m_channel_previous, sizeof(int));
    //m_eventLoop.exec();
    sendMessage(REQUEST_FLAG_GET_IPCPARAM, &m_channel_previous, sizeof(int));
    // int result = m_eventLoop.exec();
    // if (result < 0) {
    //     //MsWaitting::closeGlobalWait();
    //     return;
    // }
    ui->comboBox_secondaryStream->setEnabled(true);
    ui->comboBox_recordStreamType->setEnabled(true);
    //判断是否支持人脸，若支持是否开启人脸隐私
    m_isOpenFacePrivacy = false;
    sendMessage(REQUEST_FLAG_GET_FACE_SUPPORT, &m_channel_previous, sizeof(int));
    //m_eventLoop.exec();
    showGeneralParameters();
    //
    m_showEventStream = true;
    //
    if (qMsNvr->isFisheye(m_channel_previous)) {
        sendMessage(REQUEST_FLAG_GET_FISHEYE_MODE, &m_channel_previous, sizeof(int));
        //m_eventLoop.exec();
        //
        qMsDebug() << "channel:" << m_channel_previous << FisheyeMinoridString(camera_previous.minorid);
        int channel_id = 0;
        if (m_fishmode_param.fishcorrect == 0) {
            if (channel_id != 1) {
                qMsDebug() << QString("hide event stream, channel: %1, channel id: %2").arg(m_channel_previous).arg(channel_id);
                m_showEventStream = false;
            }
        }
    }
    //
    MsCameraVersion cameraVersion(m_edit->m_ipcdev.fwversion);
    if (cameraVersion < MsCameraVersion(7, 75)) {
        qMsDebug() << QString("hide event stream, channel: %1, channel id: %2") << cameraVersion;
        m_showEventStream = false;
    }
    //
    if (m_showEventStream) {
        ui->comboBox_recordStreamType->clear();
        ui->comboBox_recordStreamType->addItem(GET_TEXT("CAMERAEDIT/31021", "General"));
        ui->comboBox_recordStreamType->addItem(GET_TEXT("CAMERAEDIT/31022", "Event"));
    } else {
        ui->comboBox_recordStreamType->clear();
        ui->comboBox_recordStreamType->addItem(GET_TEXT("CAMERAEDIT/31021", "General"));
    }
    ui->comboBox_recordStreamType->setCurrentIndex(0);
    ui->label_eventStream->hide();
    ui->checkBox_eventStream->hide();
    //
    if (m_showEventStream) {
        req_stream_info req;
        req.chnid = m_channel_previous;
        req.fisheyeType = qMsNvr->isFisheye(m_channel_previous);
        Q_UNUSED(req)
        //m_eventLoop.exec();
    }
    //
    //MsWaitting::closeGlobalWait();

    //
    on_comboBox_recordStreamType_activated(0);

    //
    /**************
     * 对于以Multi-channel模式添加的鱼眼，On-board Display Mode对应的各个通道的Parameters信息页面显示也不同，具体为：
     * 1O ---- 主次码流可编辑
     * 1P ---- 主次码流可编辑
     * 2P ---- 主次码流可编辑
     * 4R ---- 仅主码流可编辑， 次码流置空置灰不可编辑
     * 1O3R ---- 仅主码流可编辑， 次码流置空置灰不可编辑
     * 1P3R ---- 仅主码流可编辑， 次码流置空置灰不可编辑
     * 1O1P3R ---- 仅主码流可编辑， 次码流置空置灰不可编辑
    **************/
    if (qMsNvr->isFisheye(m_channel_previous)) {
        if (m_fishmode_param.fishcorrect == 0) {
            bool isSubEnable = true;
            switch (m_fishmode_param.fishdisplay) {
            case MsQt::FISHEYE_DISPLAY_1O:
            case MsQt::FISHEYE_DISPLAY_1P:
            case MsQt::FISHEYE_DISPLAY_2P:
                break;
            default:
                isSubEnable = false;
                break;
            }
            ui->comboBox_secondaryStream->setEnabled(isSubEnable);
            if (isSubEnable) {
                on_comboBox_secondaryStream_activated(m_ipc_param.sub_enable);
            } else {
                setSubStreamEnableForFisheye(isSubEnable);
            }
        }
    }
}

void CameraEditTabParameters::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_SINGLE_IPCTYPE:
        ON_RESPONSE_FLAG_GET_SINGLE_IPCTYPE(message);
        message->accept();
        break;
    case RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY:
        ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(message);
        message->accept();
        break;
    case RESPONSE_FLAG_GET_FISHEYE_MODE:
        ON_RESPONSE_FLAG_GET_FISHEYE_MODE(message);
        message->accept();
        break;
    case RESPONSE_FLAG_GET_IPCPARAM:
        ON_RESPONSE_FLAG_GET_IPCPARAM(message);
        message->accept();
        break;
    case RESPONSE_FLAG_SET_IPC_COMMON_PARAM:
        ON_RESPONSE_FLAG_SET_IPC_COMMON_PARAM(message);
        message->accept();
        break;
    case RESPONSE_FLAG_GET_IPC_COMMON_PARAM:
        ON_RESPONSE_FLAG_GET_IPC_COMMON_PARAM(message);
        message->accept();
        break;
    case RESPONSE_FLAG_GET_IPC_EVENT_STREAM_INFO:
        ON_RESPONSE_FLAG_GET_IPC_EVENT_STREAM_INFO(message);
        message->accept();
        break;
    case RESPONSE_FLAG_SET_IPC_EVENT_STREAM_INFO:
        ON_RESPONSE_FLAG_SET_IPC_EVENT_STREAM_INFO(message);
        message->accept();
        break;
    case RESPONSE_FLAG_SET_IPCPARAM:
        ON_RESPONSE_FLAG_SET_IPCPARAM_EX(message);
        message->accept();
        break;
    case RESPONSE_FLAG_GET_FACE_SUPPORT:
        ON_RESPONSE_FLAG_GET_FACE_SUPPORT(message);
        message->accept();
        break;
    case RESPONSE_FLAG_GET_FACE_CONFIG:
        ON_RESPONSE_FLAG_GET_FACE_CONFIG(message);
        message->accept();
        break;
    }
}

void CameraEditTabParameters::ON_RESPONSE_FLAG_GET_SINGLE_IPCTYPE(MessageReceive *message)
{
    CAM_MODEL_INFO *cam_model_info = (CAM_MODEL_INFO *)message->data;
    if (cam_model_info) {
        qDebug() << QString("CameraEdit::ON_RESPONSE_FLAG_GET_SINGLE_IPCTYPE, channel: %1, ip: %2, model: %3, fwversion: %4")
                        .arg(cam_model_info->chnid)
                        .arg(cam_model_info->ipaddr)
                        .arg(cam_model_info->model)
                        .arg(cam_model_info->fwversion);
        memcpy(&m_model_info, cam_model_info, sizeof(CAM_MODEL_INFO));

        m_cameraModel.setModel(QString(m_model_info.model));
    }
    m_eventLoop.exit();
}

void CameraEditTabParameters::ON_RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY(MessageReceive *message)
{
    message->accept();
    resp_image_display *image_display = (resp_image_display *)message->data;
    if (image_display) {
        m_cameraModel.setPowerFrequency(image_display->image.exposurectrl);
        m_cameraModel.setLensDistortCorrect(image_display->image.lencorrect);
    } else {
        qMsWarning() << "data is null";
    }
    m_eventLoop.exit();
}

void CameraEditTabParameters::ON_RESPONSE_FLAG_GET_FISHEYE_MODE(MessageReceive *message)
{
    memset(&m_fishmode_param, 0, sizeof(m_fishmode_param));
    memcpy(&m_fishmode_param, message->data, sizeof(m_fishmode_param));
    m_eventLoop.exit();
}

void CameraEditTabParameters::ON_RESPONSE_FLAG_GET_IPCPARAM(MessageReceive *message)
{
    resp_get_ipc_param *ipc_param = (resp_get_ipc_param *)message->data;
    if (!ipc_param) {
        qMsWarning() << message;
        ShowMessageBox(GET_TEXT("CAMERAEDIT/31009", "Faild to get parameters."));
        m_eventLoop.exit(-1);
        return;
    } else {
        qDebug() << "----RESPONSE_FLAG_GET_IPCPARAM----"
                 << "\n----ret:" << ipc_param->ret
                 << "\n----chnid:" << ipc_param->chnid
                 << "\n----sub_enable:" << ipc_param->sub_enable
                 << "\n----audio_enable:" << ipc_param->audio_enable
                 << "\n----videoType_enable:" << ipc_param->videoType_enable
                 << "\n----ipc_type:" << ipc_param->ipc_type
                 << "\n----sub_stream:" << ipc_param->sub_stream
                 << "\n----supportsmartstream:" << ipc_param->supportsmartstream
                 << "\n----smartStream:" << ipc_param->smartStream
                 << "\n----smartStream_level:" << ipc_param->smartStream_level
                 << "\n----subSupportSmartStream:" << ipc_param->subSupportSmartStream
                 << "\n----subSmartStream:" << ipc_param->subSmartStream
                 << "\n----subSmartStream_level:" << ipc_param->subSmartStream_level
                 << "\n----productmodel:" << ipc_param->productmodel
                 << "\n----main_range----"
                 << "\n------framerate_max:" << ipc_param->main_range.framerate_max
                 << "\n------framerate_min:" << ipc_param->main_range.framerate_min
                 << "\n------bitrate_max:" << ipc_param->main_range.bitrate_max
                 << "\n------bitrate_min:" << ipc_param->main_range.bitrate_min
                 << "\n------vcodec_type:" << ipc_param->main_range.vcodec_type
                 << "\n------framerate:" << ipc_param->main_range.framerate
                 << "\n------bitrate:" << ipc_param->main_range.bitrate
                 << "\n------iframeinterval:" << ipc_param->main_range.iframeinterval
                 << "\n------rateControl:" << ipc_param->main_range.rateControl
                 << "\n----sub_range----"
                 << "\n------framerate_max:" << ipc_param->sub_range.framerate_max
                 << "\n------framerate_min:" << ipc_param->sub_range.framerate_min
                 << "\n------bitrate_max:" << ipc_param->sub_range.bitrate_max
                 << "\n------bitrate_min:" << ipc_param->sub_range.bitrate_min
                 << "\n------vcodec_type:" << ipc_param->sub_range.vcodec_type
                 << "\n------framerate:" << ipc_param->sub_range.framerate
                 << "\n------bitrate:" << ipc_param->sub_range.bitrate
                 << "\n------iframeinterval:" << ipc_param->sub_range.iframeinterval
                 << "\n------rateControl:" << ipc_param->sub_range.rateControl
                 << "\n------mainStream:" << ipc_param->mainStream
                 << "\n------sub_stream:" << ipc_param->sub_stream;
    }
    //
    memcpy(&m_ipc_param, ipc_param, sizeof(struct resp_get_ipc_param));
    //
    m_mainStream = QString("stream_%1").arg(ipc_param->mainStream - 1);
    if (ipc_param->sub_stream > 0) {
        m_SubStream = QString("stream_%1").arg(ipc_param->sub_stream - 1);
    } else {
        m_SubStream = "";
    }
    if (m_ipc_param.ret != 0) {
        qMsWarning() << QString("channel: %1, result: %2").arg(m_ipc_param.chnid).arg(m_ipc_param.ret);
        ShowMessageBox(GET_TEXT("CAMERAEDIT/31009", "Faild to get parameters."));
        m_eventLoop.exit(-2);
    } else {
        m_lastSecondaryStreamEnable = ipc_param->sub_enable;
        m_eventLoop.exit();
    }
}

void CameraEditTabParameters::ON_RESPONSE_FLAG_GET_IPC_COMMON_PARAM(MessageReceive *message)
{
    resp_http_common_param *param = (resp_http_common_param *)message->data;
    if (!param) {
        qWarning() << "CameraEdit::ON_RESPONSE_FLAG_GET_IPC_COMMON_PARAM, data is null.";
        return;
    }
    int iValue = 0;
    qMsDebug() << "res:" << param->res
               << ", key:" << param->info.key
               << ", value:" << param->info.value;
    if (param->res == 0) {
        iValue = atoi(param->info.value);
        if (strcmp(param->info.key, "ratecontrol1") == 0) {
            ui->comboBox_bitRateControl_main->setCurrentIndex(iValue);
        } else if (strcmp(param->info.key, "ratecontrol2") == 0
                   || strcmp(param->info.key, "ratecontrol3") == 0) {
            ui->comboBox_bitRateControl_sub->setCurrentIndex((iValue));
        }
    }
    showBitrateCtl();
}

void CameraEditTabParameters::ON_RESPONSE_FLAG_GET_IPC_EVENT_STREAM_INFO(MessageReceive *message)
{
    event_stream_info *stream_info = (event_stream_info *)message->data;
    memset(&m_event_info, 0, sizeof(m_event_info));
    memcpy(&m_event_info, stream_info, sizeof(m_event_info));

    qDebug() << "====RESPONSE_FLAG_GET_IPC_EVENT_STREAM_INFO===="
             << ", chnid" << m_event_info.chnid
             << ", fisheyeType:" << m_event_info.fisheyeType
             << ", enable:" << m_event_info.enable
             << ", framerate:" << m_event_info.framerate
             << ", bitrate:" << m_event_info.bitrate
             << ", iframeinterval;" << m_event_info.iframeinterval;

    m_eventLoop.exit();
}

void CameraEditTabParameters::ON_RESPONSE_FLAG_SET_IPC_EVENT_STREAM_INFO(MessageReceive *message)
{
    int result = *(int *)(message->data);
    qMsDebug() << "result:" << result;

    m_eventLoop.exit();
}

void CameraEditTabParameters::ON_RESPONSE_FLAG_SET_IPC_COMMON_PARAM(MessageReceive *message)
{
    Q_UNUSED(message);

    //usleep(2000 * 1000);
    QTimer::singleShot(2000, this, SLOT(saveBitrateCtl()));
}

void CameraEditTabParameters::ON_RESPONSE_FLAG_SET_IPCPARAM_EX(MessageReceive *message)
{
    m_eventLoop.exit();

    m_ipcSetParamResult = -1;
    if (!message->data) {
        qMsWarning() << "data is null.";
        return;
    }
    m_ipcSetParamResult = *((int *)message->data);
    qMsDebug() << QString("result: 0x%1").arg(m_ipcSetParamResult, 0, 16);
}

void CameraEditTabParameters::ON_RESPONSE_FLAG_GET_FACE_SUPPORT(MessageReceive *message)
{
    if (!message->data) {
        qMsWarning() << "data is null.";
        return;
    }
    bool isSupportedFace = *(static_cast<bool *>(message->data));
    if (isSupportedFace) {
        sendMessage(REQUEST_FLAG_GET_FACE_CONFIG, &m_channel_previous, sizeof(int));
    } else {
        m_eventLoop.exit();
    }
}

void CameraEditTabParameters::ON_RESPONSE_FLAG_GET_FACE_CONFIG(MessageReceive *message)
{
    struct MsFaceConfig *faceConfig = static_cast<MsFaceConfig *>(message->data);
    if (!faceConfig) {
        qWarning() << "data is null.";
        return;
    }
    m_isOpenFacePrivacy = faceConfig->enable && faceConfig->mosaicEnable;
    m_eventLoop.exit();
}

void CameraEditTabParameters::showGeneralParameters()
{
    //
    ui->comboBox_recordStreamType->clear();
    ui->comboBox_recordStreamType->addItem(GET_TEXT("CAMERAEDIT/31021", "General"));
    ui->comboBox_recordStreamType->addItem(GET_TEXT("CAMERAEDIT/31022", "Event"));

    ui->comboBox_codec_main->clear();
    ui->comboBox_codec_main->addItem(QString("H.264"), CODECTYPE_H264);
    ui->comboBox_codec_main->addItem(QString("H.265"), CODECTYPE_H265);

    ui->comboBox_codec_sub->clear();
    ui->comboBox_codec_sub->addItem(QString("H.264"), CODECTYPE_H264);
    ui->comboBox_codec_sub->addItem(QString("H.265"), CODECTYPE_H265);

    ui->comboBox_secondaryStream->beginEdit();
    ui->comboBox_secondaryStream->clear();
    ui->comboBox_secondaryStream->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);
    ui->comboBox_secondaryStream->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBox_secondaryStream->endEdit();

    ui->comboBox_bitRateControl_main->clear();
    ui->comboBox_bitRateControl_main->addItem("CBR", 0);
    ui->comboBox_bitRateControl_main->addItem("VBR", 1);

    ui->comboBox_bitRateControl_sub->clear();
    ui->comboBox_bitRateControl_sub->addItem("CBR", 0);
    ui->comboBox_bitRateControl_sub->addItem("VBR", 1);

    ui->comboBox_smartStream_main->clear();
    ui->comboBox_smartStream_main->addItem("Off", 0);
    ui->comboBox_smartStream_main->addItem("On", 1);

    ui->comboBox_smartStream_sub->clear();
    ui->comboBox_smartStream_sub->addItem("Off", 0);
    ui->comboBox_smartStream_sub->addItem("On", 1);
    if (qMsNvr->isMsCamera(m_ipc_param.chnid)) {
        m_readygetbitctlMap.clear();
        m_readygetbitctlMap.insert(0, 1);
        m_readygetbitctlMap.insert(m_ipc_param.sub_stream, m_ipc_param.sub_enable);
        showBitrateCtl();
        MsWaitting::execGlobalWait();
    }

    //main video codec
    ui->comboBox_codec_main->setCurrentIndexFromData(m_ipc_param.main_range.vcodec_type);

    if (m_ipc_param.main_range.rateControl >= 0) {
        ui->comboBox_bitRateControl_main->setCurrentIndexFromData(m_ipc_param.main_range.rateControl);
    }
    on_comboBox_codec_main_activated(ui->comboBox_codec_main->currentIndex());

    //bit rate
    ui->comboBox_bitRate_main->clear();
    QList<int> bitRateList = qMsNvr->bitRateList();
    for (int i = 0; i < bitRateList.size(); ++i) {
        const int &bitrate = bitRateList.at(i);
        if (bitrate >= m_ipc_param.main_range.bitrate_min && bitrate <= m_ipc_param.main_range.bitrate_max) {
            ui->comboBox_bitRate_main->addItem(QString::number(bitrate), bitrate);
        }
    }
    ui->comboBox_bitRate_main->setCurrentBitRate(m_ipc_param.main_range.bitrate);

    //i-frame interval
    int iframerange = 0;
    if (m_ipc_param.mainStream > 3) {
        iframerange = m_ipc_param.streamGop[0];
    } else {
        iframerange = m_ipc_param.streamGop[m_ipc_param.mainStream - 1];
    }
    if (!iframerange) {
        iframerange = 120;
        ui->comboBox_frameInterval_main->clear();
        if (m_ipc_param.main_range.framerate_max > 60) {
            iframerange = 240;
        }
        if (MsCameraVersion(m_model_info.fwversion) >= MsCameraVersion(7, 68)) {
            MsCameraModel cameraModel(m_model_info.model);
            if (cameraModel.isHighFrameRate_90()) {
                iframerange = 180;
            } else if (cameraModel.isHighFrameRate_120()) {
                iframerange = 240;
            }
        }
    }
    if (ui->comboBox_frameInterval_main->count() == 0) {
        for (int i = 0; i < iframerange; ++i) {
            ui->comboBox_frameInterval_main->addItem(QString::number(i + 1), i + 1);
        }
    }
    ui->comboBox_frameInterval_main->setCurrentIndexFromData(m_ipc_param.main_range.iframeinterval);

    //smart stream
    if (m_ipc_param.supportsmartstream) {
        ui->label_smartStream_main->setVisible(true);
        ui->comboBox_smartStream_main->setVisible(true);
        ui->comboBox_smartStream_main->setCurrentIndexFromData(m_ipc_param.smartStream);
        ui->slider_level_main->setValue(m_ipc_param.smartStream_level);
    } else {
        ui->label_smartStream_main->setVisible(false);
        ui->comboBox_smartStream_main->setVisible(false);
        ui->comboBox_smartStream_main->setCurrentIndexFromData(0);
    }
    on_comboBox_smartStream_main_activated(ui->comboBox_smartStream_main->currentIndex());

    //sub enable
    ui->comboBox_secondaryStream->setCurrentIndexFromData(m_ipc_param.sub_enable);
    m_channel_substream = -1;
    if (m_ipc_param.sub_stream) {
        m_channel_substream = m_ipc_param.sub_stream;
    }

    if (m_ipc_param.sub_enable) {
        //sub video codec
        ui->comboBox_codec_sub->setCurrentIndexFromData(m_ipc_param.sub_range.vcodec_type);
        on_comboBox_codec_sub_activated(ui->comboBox_codec_sub->currentIndex());
        //sub bitrate control
        //ui->comboBox_bitRateControl_sub->setCurrentIndex(1);

        //bit rate
        ui->comboBox_bitRate_sub->clear();
        for (int i = 0; i < bitRateList.size(); ++i) {
            const int &bitrate = bitRateList.at(i);
            if (bitrate >= m_ipc_param.sub_range.bitrate_min && bitrate <= m_ipc_param.sub_range.bitrate_max) {
                if (!(bitrate == 3072)) {
                    ui->comboBox_bitRate_sub->addItem(QString::number(bitrate), bitrate);
                }
            }
        }
        ui->comboBox_bitRate_sub->setCurrentBitRate(m_ipc_param.sub_range.bitrate);
        //i-frame range
        int iframerange = m_ipc_param.streamGop[m_ipc_param.sub_stream - 1];
        if (!iframerange) {
            iframerange = 120;
            if (MsCameraVersion(m_model_info.fwversion) >= MsCameraVersion(7, 68)) {
                MsCameraModel cameraModel(m_model_info.model);
                if (cameraModel.isHighFrameRate_90()) {
                    iframerange = 180;
                } else if (cameraModel.isHighFrameRate_120()) {
                    iframerange = 240;
                }
            }
        }
        ui->comboBox_frameInterval_sub->clear();
        for (int i = 0; i < iframerange; ++i) {
            ui->comboBox_frameInterval_sub->addItem(QString::number(i + 1), i + 1);
        }
        ui->comboBox_frameInterval_sub->setCurrentIndexFromData(m_ipc_param.sub_range.iframeinterval);
        //bitRateControl rate
        if (m_ipc_param.sub_range.rateControl >= 0) {
            ui->comboBox_bitRateControl_sub->setCurrentIndex(m_ipc_param.sub_range.rateControl);
        }

        //smart stream
        if (m_ipc_param.subSupportSmartStream) {
            if (m_ipc_param.subSmartStream == 0 || m_ipc_param.subSmartStream == 1) {
                ui->comboBox_smartStream_sub->clear();
                ui->comboBox_smartStream_sub->addItem("Off", 0);
                ui->comboBox_smartStream_sub->addItem("On", 1);
                ui->comboBox_smartStream_sub->endEdit();
                ui->comboBox_smartStream_sub->setCurrentIndexFromData(m_ipc_param.subSmartStream);
            } else {
                ui->comboBox_smartStream_sub->clear();
            }
            ui->slider_level_sub->setValue(m_ipc_param.subSmartStream_level);
        } else {
            ui->comboBox_smartStream_sub->clear();
            ui->comboBox_smartStream_sub->setCurrentIndexFromData(0);
        }
        on_comboBox_smartStream_sub_activated(ui->comboBox_smartStream_sub->currentIndex());
    } else {
        //MSHN-9227	QT-Camera ：Camera Edit-Parameters，次码流Disable情况下，选项都需清空且置灰
        ui->comboBox_codec_sub->setCurrentIndex(-1);
        ui->comboBox_frameSize_sub->setCurrentIndex(-1);
        ui->comboBox_frameRate_sub->setCurrentIndex(-1);
        ui->comboBox_bitRate_sub->setCurrentIndex(-1);
        ui->comboBox_bitRateControl_sub->setCurrentIndex(-1);
        ui->comboBox_frameInterval_sub->setCurrentIndex(-1);
        ui->comboBox_smartStream_sub->setCurrentIndex(-1);
        ui->slider_level_sub->setValue(ui->slider_level_sub->minimumValue());
    }

    on_comboBox_secondaryStream_activated(m_ipc_param.sub_enable);
    //
    m_set_ipc_param.framerate = ui->comboBox_frameRate_main->currentData().toInt();
    m_set_ipc_param.bitrate = ui->comboBox_bitRate_main->currentBitRate();
    m_set_ipc_param.iframeinterval = ui->comboBox_frameInterval_main->currentData().toInt();

    //
    ui->comboBox_frameSize_main->reSetIndex();
}

void CameraEditTabParameters::showEventParameters()
{
}

void CameraEditTabParameters::setSubStreamEnableForFisheye(bool enable)
{
    if (enable) {

    } else {
        ui->comboBox_secondaryStream->setCurrentIndexFromData(0);
        ui->comboBox_codec_sub->setCurrentIndex(-1);
        ui->comboBox_frameSize_sub->setCurrentIndex(-1);
        ui->comboBox_frameRate_sub->setCurrentIndex(-1);
        ui->comboBox_bitRate_sub->setCurrentIndex(-1);
        ui->comboBox_bitRateControl_sub->setCurrentIndex(-1);
        ui->comboBox_frameInterval_sub->setCurrentIndex(-1);
        ui->comboBox_smartStream_sub->setCurrentIndex(-1);
        ui->slider_level_sub->setValue(ui->slider_level_sub->minimumValue());
    }
    ui->comboBox_secondaryStream->setEnabled(enable);
    ui->comboBox_codec_sub->setEnabled(enable && !m_isOpenFacePrivacy);
    ui->comboBox_frameSize_sub->setEnabled(enable);
    ui->comboBox_frameRate_sub->setEnabled(enable);
    ui->comboBox_bitRate_sub->setEnabled(enable);
    ui->comboBox_bitRateControl_sub->setEnabled(enable);
    ui->comboBox_frameInterval_sub->setEnabled(enable);
    ui->comboBox_smartStream_sub->setEnabled(enable);
    ui->slider_level_sub->setEnabled(enable);
}

void CameraEditTabParameters::showBitrateCtl()
{
    int mainsub = 0, value = 0;
    if (m_readygetbitctlMap.isEmpty()) {
        //MsWaitting::closeGlobalWait();
        return;
    }

    if (!m_readygetbitctlMap.isEmpty()) {
        for (auto iter = m_readygetbitctlMap.begin(); iter != m_readygetbitctlMap.end();) {
            mainsub = iter.key();
            value = iter.value();
            qDebug() << "iter chnid:" << m_channel_previous << " mainsub:" << mainsub << " value:" << value;
            iter = m_readygetbitctlMap.erase(iter);
            if (value) {
                break;
            }
        }
    }

    struct req_http_common_param req;
    memset(&req, 0, sizeof(struct req_http_common_param));

    req.chnid = m_channel_previous;
    if (mainsub == 0) {
        snprintf(req.info.httpname, sizeof(req.info.httpname), "%s", "vsetting.html");
        snprintf(req.info.pagename, sizeof(req.info.pagename), "%s", "video.0");
        snprintf(req.info.key, sizeof(req.info.key), "%s", "ratecontrol1");
    } else if (mainsub == 2) {
        snprintf(req.info.httpname, sizeof(req.info.httpname), "%s", "vsetting_sub.html");
        snprintf(req.info.pagename, sizeof(req.info.pagename), "%s", "video.1");
        snprintf(req.info.key, sizeof(req.info.key), "%s", "ratecontrol2");
    } else if (mainsub == 3) {
        snprintf(req.info.httpname, sizeof(req.info.httpname), "%s", "vsetting_third.html");
        snprintf(req.info.pagename, sizeof(req.info.pagename), "%s", "video.2");
        snprintf(req.info.key, sizeof(req.info.key), "%s", "ratecontrol3");
    }
    qDebug() << "showBitrateCtl chnid:" << m_channel_previous << " mainsub:" << mainsub;
    sendMessage(REQUEST_FLAG_GET_IPC_COMMON_PARAM, &req, sizeof(struct req_http_common_param));
}

void CameraEditTabParameters::saveBitrateCtl()
{
    int mainsub = 0, value = 0;

    if (m_readybitctlMap.isEmpty()) {
        ////m_waitting->//closeWait();
        return;
    } else {
        auto iter = m_readybitctlMap.begin();
        mainsub = iter.key();
        value = iter.value();
        m_readybitctlMap.erase(iter);
    }

    struct req_http_common_param req;
    memset(&req, 0, sizeof(struct req_http_common_param));

    req.chnid = m_channel_previous;
    if (mainsub == 0) {
        snprintf(req.info.httpname, sizeof(req.info.httpname), "%s", "vsetting.html");
        snprintf(req.info.pagename, sizeof(req.info.pagename), "%s", "video.0");
        snprintf(req.info.key, sizeof(req.info.key), "%s", "ratecontrol1");
    } else if (mainsub == 2) {
        snprintf(req.info.httpname, sizeof(req.info.httpname), "%s", "vsetting_sub.html");
        snprintf(req.info.pagename, sizeof(req.info.pagename), "%s", "video.1");
        snprintf(req.info.key, sizeof(req.info.key), "%s", "ratecontrol2");
    } else if (mainsub == 3) {
        snprintf(req.info.httpname, sizeof(req.info.httpname), "%s", "vsetting_third.html");
        snprintf(req.info.pagename, sizeof(req.info.pagename), "%s", "video.2");
        snprintf(req.info.key, sizeof(req.info.key), "%s", "ratecontrol3");
    }

    snprintf(req.info.value, sizeof(req.info.value), "%d", value);
    qDebug() << "saveBitrateCtl chnid:" << m_channel_previous << " mainsub:" << mainsub << " value:" << value;
    sendMessage(REQUEST_FLAG_SET_IPC_COMMON_PARAM, (void *)&req, sizeof(struct req_http_common_param));

    return;
}

int CameraEditTabParameters::saveParameters()
{
    m_readybitctlMap.clear();
    //MsWaitting::showGlobalWait();

    camera cam;
    memset(&cam, 0, sizeof(camera));
    read_camera(SQLITE_FILE_NAME, &cam, m_channel_previous);
    m_ipcSetParamResult = -1;
    if (cam.camera_protocol == IPC_PROTOCOL_RTSP) {
        //MsWaitting::closeGlobalWait();
        ExecMessageBox(GET_TEXT("CAMERAEDIT/31013", "Save Unsuccessfully."));
        return m_ipcSetParamResult;
    }
    //    m_readybitctlMap.insert(0, ui->comboBox_bitRateControl_main->currentData().toInt());
    //    if (m_channel_substream != -1)
    //    {
    //       m_readybitctlMap.insert(m_channel_substream, ui->comboBox_bitRateControl_sub->currentData().toInt());
    //    }
    //    saveBitrateCtl();

    //
    m_set_ipc_param.chnid = m_channel_previous;
    QStringList resolution = ui->comboBox_frameSize_main->currentText().split("*");
    if (resolution.size() == 2) {
        m_set_ipc_param.width = resolution.at(0).toInt();
        m_set_ipc_param.height = resolution.at(1).toInt();
    }
    m_set_ipc_param.codec = ui->comboBox_codec_main->currentData().toInt();
    m_set_ipc_param.framerate = ui->comboBox_frameRate_main->currentIntData();
    m_set_ipc_param.substream_enable = ui->comboBox_secondaryStream->isEnabled() ? ui->comboBox_secondaryStream->currentIntData() : -1;
    m_set_ipc_param.sub_framerate = ui->comboBox_frameRate_sub->currentData().toInt();
    m_set_ipc_param.sub_bitrate = ui->comboBox_bitRate_sub->currentBitRate();
    resolution = ui->comboBox_frameSize_sub->currentText().split("*");
    if (resolution.size() == 2) {
        m_set_ipc_param.sub_width = resolution.at(0).toInt();
        m_set_ipc_param.sub_height = resolution.at(1).toInt();
    }
    m_set_ipc_param.sub_codec = ui->comboBox_codec_sub->currentData().toInt();
    m_set_ipc_param.audio_enable = m_ipc_param.audio_enable;
    m_set_ipc_param.videoType_enable = 1;
    m_set_ipc_param.subiframeinterval = ui->comboBox_frameInterval_sub->currentData().toInt();

    m_set_ipc_param.supportsmartstream = 1;
    m_set_ipc_param.smartstream = ui->comboBox_smartStream_main->currentData().toInt();
    m_set_ipc_param.smartstreamlevel = ui->slider_level_main->value();
    m_set_ipc_param.subSmartstream = ui->comboBox_smartStream_sub->currentData().toInt();
    m_set_ipc_param.subSmartstreamlevel = ui->slider_level_sub->value();

    //sendMessage(REQUEST_FLAG_SET_IPCPARAM, (void*)&m_set_ipc_param, sizeof(m_set_ipc_param));
    //m_eventLoop.exec();

    //采用新接口设置，m_set_ipc_param后续可能会移除
    req_set_ipc_paramex set_ipc_paramex;
    memset(&set_ipc_paramex, 0, sizeof(set_ipc_paramex));
    //
    set_ipc_paramex.chnid[m_channel_previous] = '1';
    set_ipc_paramex.trans_protocol = -1;
    set_ipc_paramex.sync_time = -1;
    set_ipc_paramex.audio_enable = -1;
    set_ipc_paramex.videoType_enable = -1;
    set_ipc_paramex.subEnable = m_set_ipc_param.substream_enable;
    set_ipc_paramex.fisheyeType = qMsNvr->isFisheye(m_channel_previous);
    //
    req_set_ipc_subparam &param_main = set_ipc_paramex.stream[STREAM_TYPE_MAINSTREAM];
    param_main.recordtype = -1;
    param_main.codec = m_set_ipc_param.codec;
    param_main.width = m_set_ipc_param.width;
    param_main.height = m_set_ipc_param.height;
    param_main.framerate = m_set_ipc_param.framerate;
    param_main.bitrate = m_set_ipc_param.bitrate;
    param_main.iframeinterval = m_set_ipc_param.iframeinterval;
    param_main.ratecontrol = ui->comboBox_bitRateControl_main->currentData().toInt();
    param_main.smartstream = m_set_ipc_param.smartstream;
    param_main.smartstreamlevel = m_set_ipc_param.smartstreamlevel;
    //
    req_set_ipc_subparam &param_sub = set_ipc_paramex.stream[STREAM_TYPE_SUBSTREAM];
    param_main.recordtype = -1;
    param_sub.codec = m_set_ipc_param.sub_codec;
    param_sub.width = m_set_ipc_param.sub_width;
    param_sub.height = m_set_ipc_param.sub_height;
    param_sub.framerate = m_set_ipc_param.sub_framerate;
    param_sub.bitrate = m_set_ipc_param.sub_bitrate;
    param_sub.iframeinterval = m_set_ipc_param.subiframeinterval;
    param_sub.ratecontrol = ui->comboBox_bitRateControl_sub->currentData().toInt();
    param_sub.smartstream = m_set_ipc_param.subSmartstream;
    param_sub.smartstreamlevel = m_set_ipc_param.subSmartstreamlevel;

    //
    qMsDebug() << "save event stream:" << m_showEventStream;
    if (m_showEventStream) {
        m_event_info.chnid = m_channel_previous;
        m_event_info.fisheyeType = qMsNvr->isFisheye(m_channel_previous);
        qDebug() << ", chnid:" << m_event_info.chnid
                 << ", fisheyeType:" << m_event_info.fisheyeType
                 << ", enable:" << m_event_info.enable
                 << ", framerate:" << m_event_info.framerate
                 << ", bitrate:" << m_event_info.bitrate
                 << ", iframeinterval:" << m_event_info.iframeinterval;
        //sendMessage(REQUEST_FLAG_SET_IPC_EVENT_STREAM_INFO, (void*)&m_event_info, sizeof(m_event_info));
        //
        param_main.recordtype = m_event_info.enable;
        param_main.etframerate = m_event_info.framerate;
        param_main.etbitrate = m_event_info.bitrate;
        param_main.etIframeinterval = m_event_info.iframeinterval;
    }
    //
    if (!m_lastSecondaryStreamEnable && set_ipc_paramex.subEnable) {
        param_sub.width = 0;
        param_sub.height = 0;
    }
    //
    qMsDebug() << "\n==REQUEST_FLAG_SET_IPCPARAM=="
               << "\n--chnid:" << m_channel_previous
               << "\n--fisheyeType:" << set_ipc_paramex.fisheyeType
               << "\n--subEnable:" << set_ipc_paramex.subEnable
               << "\n--fisheyeType:" << set_ipc_paramex.fisheyeType
               << "\n====main===="
               << "\n----codec:" << param_main.codec
               << "\n----width:" << param_main.width
               << "\n----height:" << param_main.height
               << "\n----framerate:" << param_main.framerate
               << "\n----bitrate:" << param_main.bitrate
               << "\n----iframeinterval:" << param_main.iframeinterval
               << "\n----ratecontrol:" << param_main.ratecontrol
               << "\n----smartstream:" << param_main.smartstream
               << "\n----smartstreamlevel:" << param_main.smartstreamlevel
               << "\n====sub===="
               << "\n----codec:" << param_sub.codec
               << "\n----width:" << param_sub.width
               << "\n----height:" << param_sub.height
               << "\n----framerate:" << param_sub.framerate
               << "\n----bitrate:" << param_sub.bitrate
               << "\n----iframeinterval:" << param_sub.iframeinterval
               << "\n----ratecontrol:" << param_sub.ratecontrol
               << "\n----smartstream:" << param_sub.smartstream
               << "\n----smartstreamlevel:" << param_sub.smartstreamlevel
               << "\n====event===="
               << "\n----recordtype:" << param_main.recordtype
               << "\n----etframerate:" << param_main.etframerate
               << "\n----etbitrate:" << param_main.etbitrate
               << "\n----etIframeinterval:" << param_main.etIframeinterval;
    //
    sendMessage(REQUEST_FLAG_SET_IPCPARAM, (void *)&set_ipc_paramex, sizeof(req_set_ipc_paramex));
    //m_eventLoop.exec();
    //MsWaitting::closeGlobalWait();

    //
    int result = (m_ipcSetParamResult >> m_channel_previous & 0x01);
    qMsDebug() << "result:" << result;
    if (result == 0) {
        m_lastSecondaryStreamEnable = set_ipc_paramex.subEnable;
        ExecMessageBox(GET_TEXT("CAMERAEDIT/31010", "Save Successfully."));
        return 0;
    } else {
        ExecMessageBox(GET_TEXT("CAMERAEDIT/31013", "Save Unsuccessfully."));
        return -1;
    }
}

void CameraEditTabParameters::clearSetting()
{
    ui->comboBox_recordStreamType->setEnabled(false);
    ui->comboBox_codec_main->setEnabled(false);
    ui->comboBox_frameSize_main->setEnabled(false);
    ui->comboBox_bitRateControl_main->setEnabled(false);
    ui->comboBox_frameInterval_main->setEnabled(false);
    ui->comboBox_smartStream_main->setEnabled(false);

    ui->comboBox_recordStreamType->clear();
    ui->comboBox_codec_main->clear();
    ui->comboBox_codec_sub->clear();
    ui->comboBox_bitRateControl_main->clear();
    ui->comboBox_bitRateControl_sub->clear();
    ui->comboBox_smartStream_main->clear();
    ui->comboBox_smartStream_sub->clear();

    ui->comboBox_bitRate_main->clear();
    ui->comboBox_bitRate_sub->clear();
    ui->comboBox_frameRate_main->clear();
    ui->comboBox_frameRate_sub->clear();
    ui->comboBox_frameSize_main->clear();
    ui->comboBox_frameSize_sub->clear();
    ui->comboBox_frameInterval_main->clear();
    ui->comboBox_frameInterval_sub->clear();

    ui->comboBox_secondaryStream->clear();
    ui->comboBox_secondaryStream->setEnabled(false);
    on_comboBox_secondaryStream_activated(0);
    ui->checkBox_eventStream->setChecked(false);
    ui->label_eventStream->hide();
    ui->checkBox_eventStream->hide();
}

void CameraEditTabParameters::onLanguageChanged()
{
    //
    ui->label_primaryStream->setText(GET_TEXT("IMAGE/37333", "Primary Stream"));
    ui->label_secondaryStream->setText(GET_TEXT("CHANNELMANAGE/30057", "Secondary Stream"));
    //
    ui->label_recordStreamType->setText(GET_TEXT("CAMERAEDIT/31020", "Record Stream Type"));
    ui->label_eventStream->setText(GET_TEXT("CAMERAEDIT/31023", "Event Stream"));
    ui->checkBox_eventStream->setText(GET_TEXT("COMMON/1009", "Enable"));
    ui->label_codec_main->setText(GET_TEXT("CHANNELMANAGE/30056", "Video Codec"));
    ui->label_frame_main->setText(GET_TEXT("CHANNELMANAGE/30058", "Frame Size"));
    ui->label_frameRate_main->setText(GET_TEXT("CHANNELMANAGE/30059", "Max Frame Rate"));
    ui->label_bitRate_main->setText(GET_TEXT("CHANNELMANAGE/30060", "Bit Rate"));
    ui->label_bitRateControl_main->setText(GET_TEXT("CAMERAEDIT/31016", "Bitrate Control"));
    ui->label_frameInterval_main->setText(GET_TEXT("CAMERAEDIT/31017", "I-frame Interval"));
    ui->label_smartStream_main->setText(GET_TEXT("CAMERAEDIT/31018", "Smart Stream"));
    ui->label_level_main->setText(GET_TEXT("CAMERAEDIT/31019", "Level"));
    //
    ui->label_subStream->setText(GET_TEXT("CHANNELMANAGE/30057", "Secondary Stream"));
    ui->label_codec_sub->setText(GET_TEXT("CHANNELMANAGE/30056", "Video Codec"));
    ui->label_frame_sub->setText(GET_TEXT("CHANNELMANAGE/30058", "Frame Size"));
    ui->label_frameRate_sub->setText(GET_TEXT("CHANNELMANAGE/30059", "Max Frame Rate"));
    ui->label_bitRate_sub->setText(GET_TEXT("CHANNELMANAGE/30060", "Bit Rate"));
    ui->label_bitRateControl_sub->setText(GET_TEXT("CAMERAEDIT/31016", "Bitrate Control"));
    ui->label_frameInterval_sub->setText(GET_TEXT("CAMERAEDIT/31017", "I-frame Interval"));
    ui->label_smartStream_sub->setText(GET_TEXT("CAMERAEDIT/31018", "Smart Stream"));
    ui->label_level_sub->setText(GET_TEXT("CAMERAEDIT/31019", "Level"));
    //
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
    ui->pushButton_applly->setText(GET_TEXT("COMMON/1003", "Apply"));
}

void CameraEditTabParameters::on_comboBox_recordStreamType_activated(int index)
{
    if (index == 0) {
        ui->label_eventStream->hide();
        ui->checkBox_eventStream->hide();

        //showGeneralParameters();

        ui->comboBox_codec_main->setEnabled(true && !m_isOpenFacePrivacy);
        ui->comboBox_frameSize_main->setEnabled(true && !m_isOpenFacePrivacy);
        if (ui->comboBox_smartStream_main->currentData().toInt() == 0) {
            ui->comboBox_bitRateControl_main->setEnabled(true);
        }
        ui->comboBox_frameInterval_main->setEnabled(true);
        ui->comboBox_smartStream_main->setEnabled(true);
        ui->comboBox_frameRate_main->setEnabled(true);
        ui->comboBox_bitRate_main->setEnabled(true);
        ui->slider_level_main->setEnabled(true);

        ui->comboBox_frameRate_main->setCurrentIndexFromData(m_set_ipc_param.framerate);
        ui->comboBox_bitRate_main->setCurrentBitRate(m_set_ipc_param.bitrate);
        ui->comboBox_frameInterval_main->setCurrentIndexFromData(m_set_ipc_param.iframeinterval);
    } else {
        ui->label_eventStream->show();
        ui->checkBox_eventStream->show();

        showEventParameters();

        ui->comboBox_codec_main->setEnabled(false);
        ui->comboBox_frameSize_main->setEnabled(false);
        ui->comboBox_bitRateControl_main->setEnabled(false);
        ui->comboBox_frameInterval_main->setEnabled(false);
        ui->comboBox_smartStream_main->setEnabled(false);
        ui->slider_level_main->setEnabled(false);

        ui->checkBox_eventStream->setChecked(m_event_info.enable);
        ui->comboBox_frameRate_main->setCurrentIndexFromData(m_event_info.framerate);
        ui->comboBox_bitRate_main->setCurrentBitRate(m_event_info.bitrate);
        ui->comboBox_frameInterval_main->setCurrentIndexFromData(m_event_info.iframeinterval);
    }
}

void CameraEditTabParameters::on_comboBox_secondaryStream_activated(int index)
{
    Q_UNUSED(index);
    int enable = ui->comboBox_secondaryStream->currentIntData();
    if (m_channel_substream == -1) {
        enable = false;
    }
    ui->comboBox_codec_sub->setEnabled(enable && !m_isOpenFacePrivacy);
    ui->comboBox_frameSize_sub->setEnabled(enable);
    ui->comboBox_frameRate_sub->setEnabled(enable);
    ui->comboBox_bitRate_sub->setEnabled(enable);
    if (ui->slider_level_sub->isVisible()) {
        ui->comboBox_bitRateControl_sub->setEnabled(false);
    } else {
        ui->comboBox_bitRateControl_sub->setEnabled(enable);
    }
    ui->comboBox_frameInterval_sub->setEnabled(enable);
    ui->comboBox_smartStream_sub->setEnabled(enable && ui->comboBox_smartStream_sub->count());
    ui->slider_level_sub->setEnabled(enable);
}

void CameraEditTabParameters::on_comboBox_frameRate_main_activated(int index)
{
    ui->comboBox_frameInterval_main->setCurrentIndexFromData((index + 1) * 2);

    int type = ui->comboBox_recordStreamType->currentIndex();
    switch (type) {
    case 0:
        m_set_ipc_param.framerate = ui->comboBox_frameRate_main->currentData().toInt();
        break;
    case 1:
        m_event_info.framerate = ui->comboBox_frameRate_main->currentData().toInt();
        break;
    }
}

void CameraEditTabParameters::on_comboBox_frameRate_sub_activated(int index)
{
    ui->comboBox_frameInterval_sub->setCurrentIndexFromData((index + 1) * 2);
}

void CameraEditTabParameters::onCheckBoxEventStreamCheckStateSet(int state)
{
    switch (state) {
    case Qt::Checked:
        m_event_info.enable = 1;
        ui->comboBox_frameRate_main->setEnabled(true);
        ui->comboBox_bitRate_main->setEnabled(true);
        break;
    case Qt::Unchecked:
        m_event_info.enable = 0;
        ui->comboBox_frameRate_main->setEnabled(false);
        ui->comboBox_bitRate_main->setEnabled(false);
        break;
    default:
        break;
    }
}

void CameraEditTabParameters::onComboBoxMainBitRateSet(int bitRate)
{
    int type = ui->comboBox_recordStreamType->currentIndex();
    qDebug() << "type:" << type << ", bitRate:" << bitRate;
    switch (type) {
    case 0:
        m_set_ipc_param.bitrate = bitRate;
        break;
    case 1:
        m_event_info.bitrate = bitRate;
        break;
    }
}

void CameraEditTabParameters::onComboBoxMainBitRateEditingFinished(int bitRate)
{
    if (bitRate < MinBitRate) {
        ui->comboBox_bitRate_main->setCurrentBitRate(MinBitRate);
    } else if (bitRate > MaxBitRate) {
        ui->comboBox_bitRate_main->setCurrentBitRate(MaxBitRate);
    }
}

void CameraEditTabParameters::onComboBoxSubBitRateEditingFinished(int bitRate)
{
    if (bitRate < MinBitRate) {
        ui->comboBox_bitRate_sub->setCurrentBitRate(MinBitRate);
    } else if (bitRate > MaxBitRate) {
        ui->comboBox_bitRate_sub->setCurrentBitRate(MaxBitRate);
    }
}

void CameraEditTabParameters::on_comboBox_frameInterval_main_currentIndexChanged(int index)
{
    Q_UNUSED(index)

    int type = ui->comboBox_recordStreamType->currentIndex();
    switch (type) {
    case 0:
        m_set_ipc_param.iframeinterval = ui->comboBox_frameInterval_main->currentData().toInt();
        break;
    case 1:
        m_event_info.iframeinterval = ui->comboBox_frameInterval_main->currentData().toInt();
        break;
    }
}

void CameraEditTabParameters::on_comboBox_frameSize_main_indexSet(int index)
{
    QString mainFrameSize = ui->comboBox_frameSize_main->itemText(index);
    const CameraStreamJsonData::StreamInfo &stream = m_cameraStreamJsonData->steamMapValue(m_mainStream);
    int codec = ui->comboBox_codec_main->currentData().toInt();
    int mainMaxFrameRate = 0;
    switch (codec) {
    case CODECTYPE_H264: {
        const CameraStreamJsonData::VideoInfo &video = stream.videosMap.value("h264");
        if (video.framesMap.contains(mainFrameSize)) {
            mainMaxFrameRate = video.framesMap.value(mainFrameSize);
        }
        break;
    }
    case CODECTYPE_H265: {
        const CameraStreamJsonData::VideoInfo &video = stream.videosMap.value("h265");
        if (video.framesMap.contains(mainFrameSize)) {
            mainMaxFrameRate = video.framesMap.value(mainFrameSize);
        }
        break;
    }
    default:
        break;
    }
    if (mainMaxFrameRate == 0) {
        mainMaxFrameRate = m_ipc_param.main_range.framerate_max;
    }
    ui->comboBox_frameRate_main->clear();
    for (int i = 1; i <= mainMaxFrameRate; ++i) {
        ui->comboBox_frameRate_main->addItem(QString("%1").arg(i), i);
    }
    if (ui->comboBox_frameRate_main->findData(m_ipc_param.main_range.framerate) < 0) {
        ui->comboBox_frameRate_main->setCurrentIndexFromData(mainMaxFrameRate);
    } else {
        ui->comboBox_frameRate_main->setCurrentIndexFromData(m_ipc_param.main_range.framerate);
    }
    //sub
    int mainSubFrameRate = 0;
    if (m_cameraStreamJsonData->mainEffectsMapIsContains(mainFrameSize)) {
        mainSubFrameRate = m_cameraStreamJsonData->mainEffectsMapValue(mainFrameSize);
        //
        ui->comboBox_frameRate_sub->clear();
        for (int i = 1; i <= mainSubFrameRate; ++i) {
            ui->comboBox_frameRate_sub->addItem(QString("%1").arg(i), i);
        }
        if (ui->comboBox_frameRate_sub->findData(m_ipc_param.sub_range.framerate) < 0) {
            ui->comboBox_frameRate_sub->setCurrentIndexFromData(mainSubFrameRate);
        } else {
            ui->comboBox_frameRate_sub->setCurrentIndexFromData(m_ipc_param.sub_range.framerate);
        }
    }
}

void CameraEditTabParameters::on_comboBox_frameSize_main_activated(int index)
{
    Q_UNUSED(index)

    ui->comboBox_frameRate_main->setCurrentIndex(ui->comboBox_frameRate_main->count() - 1);
}

void CameraEditTabParameters::on_comboBox_frameSize_sub_indexSet(int index)
{
    QString mainFrameSize = ui->comboBox_frameSize_main->currentText();
    if (m_cameraStreamJsonData->mainEffectsMapIsContains(mainFrameSize)) {

    } else {
        QString subFrameSize = ui->comboBox_frameSize_sub->itemText(index);
        const CameraStreamJsonData::StreamInfo &stream = m_cameraStreamJsonData->steamMapValue(m_SubStream);
        int codec = ui->comboBox_codec_sub->currentData().toInt();
        int subMaxFrameRate = 0;
        switch (codec) {
        case CODECTYPE_H264: {
            const CameraStreamJsonData::VideoInfo &video = stream.videosMap.value("h264");
            if (video.framesMap.contains(subFrameSize)) {
                subMaxFrameRate = video.framesMap.value(subFrameSize);
            }
            break;
        }
        case CODECTYPE_H265: {
            const CameraStreamJsonData::VideoInfo &video = stream.videosMap.value("h265");
            if (video.framesMap.contains(subFrameSize)) {
                subMaxFrameRate = video.framesMap.value(subFrameSize);
            }
            break;
        }
        default:
            break;
        }
        if (subMaxFrameRate == 0) {
            subMaxFrameRate = m_ipc_param.sub_range.framerate_max;
        }
        ui->comboBox_frameRate_sub->clear();
        for (int i = 1; i <= subMaxFrameRate; ++i) {
            ui->comboBox_frameRate_sub->addItem(QString("%1").arg(i), i);
        }
        if (ui->comboBox_frameRate_sub->findData(m_ipc_param.sub_range.framerate) < 0) {
            ui->comboBox_frameRate_sub->setCurrentIndexFromData(subMaxFrameRate);
        } else {
            ui->comboBox_frameRate_sub->setCurrentIndexFromData(m_ipc_param.sub_range.framerate);
        }
    }
}

void CameraEditTabParameters::on_comboBox_frameSize_sub_activated(int index)
{
    Q_UNUSED(index)

    ui->comboBox_frameRate_sub->setCurrentIndex(ui->comboBox_frameRate_sub->count() - 1);
}

void CameraEditTabParameters::on_comboBox_smartStream_main_activated(int index)
{
    int data = ui->comboBox_smartStream_main->itemData(index).toInt();
    if (data == 1) {
        ui->comboBox_bitRateControl_main->setEnabled(false);
        ui->label_level_main->show();
        ui->slider_level_main->show();
    } else {
        ui->comboBox_bitRateControl_main->setEnabled(true);
        ui->label_level_main->hide();
        ui->slider_level_main->hide();
    }
}

void CameraEditTabParameters::on_comboBox_smartStream_sub_activated(int index)
{
    int data = ui->comboBox_smartStream_sub->itemData(index).toInt();
    if (data == 1) {
        ui->comboBox_bitRateControl_sub->setEnabled(false);
        ui->label_level_sub->show();
        ui->slider_level_sub->show();
    } else {
        ui->comboBox_bitRateControl_sub->setEnabled(true);
        ui->label_level_sub->hide();
        ui->slider_level_sub->hide();
    }
}

void CameraEditTabParameters::on_comboBox_codec_sub_activated(int index)
{
    int codec = ui->comboBox_codec_sub->itemData(index).toInt();
    QString codecStr;
    if (codec == CODECTYPE_H264) {
        codecStr = "h264";
    } else {
        codecStr = "h265";
    }
    const CameraStreamJsonData::StreamInfo &stream = m_cameraStreamJsonData->steamMapValue(m_SubStream);
    ui->comboBox_frameSize_sub->beginEdit();
    const CameraStreamJsonData::VideoInfo &video = stream.videosMap.value(codecStr);
    ui->comboBox_frameSize_sub->clear();
    for (auto item = video.framesSizeList.begin(); item != video.framesSizeList.end(); ++item) {
        ui->comboBox_frameSize_sub->addItem(*item);
    }
    int currentSubFramesizeIndex = ui->comboBox_frameSize_sub->findText(QString("%1*%2").arg(m_ipc_param.sub_range.cur_res.width).arg(m_ipc_param.sub_range.cur_res.height));
    if (currentSubFramesizeIndex < 0) {
        currentSubFramesizeIndex = 0;
    }
    ui->comboBox_frameSize_sub->setCurrentIndex(currentSubFramesizeIndex);
    ui->comboBox_frameSize_sub->endEdit();
    ui->comboBox_frameSize_sub->reSetIndex();
}

void CameraEditTabParameters::on_comboBox_codec_main_activated(int index)
{
    int codec = ui->comboBox_codec_main->itemData(index).toInt();
    QString codecStr;
    if (codec == CODECTYPE_H264) {
        codecStr = "h264";
    } else {
        codecStr = "h265";
    }
    //main frame size
    const CameraStreamJsonData::StreamInfo &stream = m_cameraStreamJsonData->steamMapValue(m_mainStream);
    ui->comboBox_frameSize_main->beginEdit();
    ui->comboBox_frameSize_main->clear();
    const CameraStreamJsonData::VideoInfo &video = stream.videosMap.value(codecStr);
    for (auto item = video.framesSizeList.begin(); item != video.framesSizeList.end(); ++item) {
        ui->comboBox_frameSize_main->addItem(*item);
    }

    int currentMainFramesizeIndex = ui->comboBox_frameSize_main->findText(QString("%1*%2").arg(m_ipc_param.main_range.cur_res.width).arg(m_ipc_param.main_range.cur_res.height));
    if (currentMainFramesizeIndex < 0) {
        qWarning() << QString("Not fount main frame size: %1x%2").arg(m_ipc_param.main_range.cur_res.width).arg(m_ipc_param.main_range.cur_res.height);
    }
    ui->comboBox_frameSize_main->setCurrentIndex(currentMainFramesizeIndex);

    ui->comboBox_frameSize_main->endEdit();
    ui->comboBox_frameSize_main->reSetIndex();
}

void CameraEditTabParameters::on_pushButton_ok_clicked()
{
    int ret = saveParameters();
    if (ret == 0) {
        //log
        struct camera cam;
        qMsNvr->readDatabaseCamera(&cam, m_channel_previous);
        qMsNvr->writeLog(cam, SUB_OP_CONFIG_LOCAL, SUB_PARAM_EDIT_IPC);
        //
        m_edit->accept();
    }
    m_edit->reject();
}

void CameraEditTabParameters::on_pushButton_cancel_clicked()
{
#if 0
    ui->comboBox_frameSize_main->clear();
    ui->comboBox_frameRate_main->clear();
    ui->comboBox_bitRate_main->clear();
    ui->comboBox_frameInterval_main->clear();
    ui->comboBox_frameSize_sub->clear();
    ui->comboBox_frameRate_sub->clear();
    ui->comboBox_bitRate_sub->clear();
    ui->comboBox_frameInterval_sub->clear();
#endif
    m_edit->reject();
}

void CameraEditTabParameters::on_pushButton_applly_clicked()
{
    int ret = saveParameters();
    if (ret == 0) {
        //log
        struct camera cam;
        qMsNvr->readDatabaseCamera(&cam, m_channel_previous);
        qMsNvr->writeLog(cam, SUB_OP_CONFIG_LOCAL, SUB_PARAM_EDIT_IPC);
    }
}
