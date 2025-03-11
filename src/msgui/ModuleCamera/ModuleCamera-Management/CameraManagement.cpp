#include "CameraManagement.h"
#include "ui_CameraManagement.h"
#include "CameraEdit.h"
#include "CameraStatusWidget.h"
#include "EventLoop.h"
#include "LiveView.h"
#include "LogWrite.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "MyDebug.h"
#include "camerachanneladd.h"
#include "centralmessage.h"
#include "msuser.h"
#include "myqt.h"
#include <QTimer>
extern "C" {
#include "msg.h"

}

CameraManagement::CameraManagement(QWidget *parent)
    : AbstractCameraPage(parent)
    , ui(new Ui::CameraManagement)
{
    ui->setupUi(this);

    ui->widget_tab->addTab(GET_TEXT("CHANNELMANAGE/30001", "Camera Management"));
    ui->widget_tab->addTab(GET_TEXT("CHANNELMANAGE/30055", "Batch Settings"));
    connect(ui->widget_tab, SIGNAL(tabClicked(int)), this, SLOT(onTabClicked(int)));

    //
    ui->comboBox_protocol->clear();
    QMap<int, ipc_protocol> protocolMap = qMsNvr->protocolMap();
    for (auto iter = protocolMap.constBegin(); iter != protocolMap.constEnd(); ++iter) {
        const ipc_protocol &protocol = iter.value();
        ui->comboBox_protocol->addItem(protocol.pro_name, protocol.pro_id);
    }
    if (!MsDevice::instance()->isMilesight()) {
        ui->comboBox_protocol->removeItemFromData(IPC_PROTOCOL_MSDOMAIN);
    }
    ui->comboBox_protocol->setCurrentIndex(0);

    ui->comboBox_transportProtocol->clear();
    ui->comboBox_transportProtocol->addItem("Auto", TRANSPROTOCOL_AUTO);
    ui->comboBox_transportProtocol->addItem("UDP", TRANSPROTOCOL_UDP);
    ui->comboBox_transportProtocol->addItem("TCP", TRANSPROTOCOL_TCP);

#if 0
    ui->comboBox_videoCodec_main->clear();
    ui->comboBox_videoCodec_main->addItem(QString("H.264"), CODECTYPE_H264);
    ui->comboBox_videoCodec_main->addItem(QString("H.265"), CODECTYPE_H265);

    ui->comboBox_videoCodec_sub->clear();
    ui->comboBox_videoCodec_sub->addItem(QString("H.264"), CODECTYPE_H264);
    ui->comboBox_videoCodec_sub->addItem(QString("H.265"), CODECTYPE_H265);

    ui->comboBox_bitrateControl_main->clear();
    ui->comboBox_bitrateControl_main->addItem("CBR", 0);
    ui->comboBox_bitrateControl_main->addItem("VBR", 1);

    ui->comboBox_bitrateControl_sub->clear();
    ui->comboBox_bitrateControl_sub->addItem("CBR", 0);
    ui->comboBox_bitrateControl_sub->addItem("VBR", 1);
#endif
    ui->pushButton_authentication->setVisible(qMsNvr->isPoe());

    //表头
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("CHANNELMANAGE/30008", "Channel");
    headerList << GET_TEXT("CHANNELMANAGE/30009", "Channel Name");
    headerList << GET_TEXT("COMMON/1019", "Edit");
    headerList << GET_TEXT("CHANNELMANAGE/30023", "Delete");
    headerList << GET_TEXT("MENU/10006", "Status");
    headerList << GET_TEXT("COMMON/1033", "IP Address");
    headerList << GET_TEXT("FISHEYE/12016", "Channel ID");
    headerList << GET_TEXT("CHANNELMANAGE/30011", "Port");
    headerList << GET_TEXT("CHANNELMANAGE/30014", "Protocol");
    headerList << GET_TEXT("CHANNELMANAGE/30027", "MAC");
    headerList << GET_TEXT("CHANNELMANAGE/30054", "Firmware Version");
    headerList << GET_TEXT("CHANNELMANAGE/30029", "Model");
    headerList << QString("SN");
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setColumnCount(headerList.size());
    //相关信号和槽
    connect(ui->tableView, SIGNAL(itemClicked(int, int)), this, SLOT(onTableItemClicked(int, int)));
    connect(ui->tableView, SIGNAL(headerChecked(bool)), this, SLOT(onTableHeaderChecked(bool)));
    connect(ui->tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onTableDoubleClicked(QModelIndex)));
    //用代理显示图片
    ui->tableView->setItemDelegateForColumn(ColumnEdit, new ItemButtonDelegate(QPixmap(":/common/common/edit.png"), this));
    ui->tableView->setItemDelegateForColumn(ColumnDelete, new ItemButtonDelegate(QPixmap(":/common/common/delete.png"), this));
    //排序
    ui->tableView->setSortableForColumn(ColumnCheck, false);
    ui->tableView->setSortableForColumn(ColumnEdit, false);
    ui->tableView->setSortableForColumn(ColumnDelete, false);
    ui->tableView->setSortType(ColumnChannel, SortFilterProxyModel::SortInt);
    ui->tableView->setSortType(ColumnIP, SortFilterProxyModel::SortIP);
    ui->tableView->setSortType(ColumnPort, SortFilterProxyModel::SortPort);
    ui->tableView->setSortType(ColumnStatus, SortFilterProxyModel::SortInt);
    //列宽
    ui->tableView->setColumnWidth(ColumnCheck, 50);
    ui->tableView->setColumnWidth(ColumnChannel, 80);
    ui->tableView->setColumnWidth(ColumnName, 150);
    ui->tableView->setColumnWidth(ColumnEdit, 70);
    ui->tableView->setColumnWidth(ColumnDelete, 70);
    ui->tableView->setColumnWidth(ColumnStatus, 120);
    ui->tableView->setColumnWidth(ColumnIP, 150);
    ui->tableView->setColumnWidth(ColumnChannelId, 100);
    ui->tableView->setColumnWidth(ColumnPort, 80);
    ui->tableView->setColumnWidth(ColumnMAC, 140);
    ui->tableView->setColumnWidth(ColumnFirmware, 180);
    ui->tableView->setColumnWidth(ColumnModel, 200);
    ui->tableView->setColumnWidth(ColumnSN, 150);

    //
    m_cameraEdit = new CameraEdit(this);

    //

    //valid check
    ui->lineEdit_channelName->setCheckMode(MyLineEdit::EmptyCheck);
    ui->lineEdit_ip->setCheckMode(MyLineEdit::ServerCheck);
    ui->lineEdit_port->setCheckMode(MyLineEdit::RangeCheck, 1, 65535);
    ui->lineEditHTTPSPort->setCheckMode(MyLineEdit::RangeCheck, 1, 65535);
    ui->lineEdit_userName->setCheckMode(MyLineEdit::UserNameCheck);
    ui->lineEdit_password->setCheckMode(MyLineEdit::EmptyCheck);

    ui->horizontalSliderMain->setRange(1, 10);
    ui->horizontalSliderSub->setRange(1, 10);

    m_cameraStreamJsonData = new CameraStreamJsonData();

    onLanguageChanged();
}

CameraManagement::~CameraManagement()
{
    delete m_cameraStreamJsonData;
    delete ui;
}

void CameraManagement::initializeData()
{
    show();
    ui->widget_tab->setCurrentTab(0);
    m_currentTab = TabCameraManagement;
}

void CameraManagement::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPCLIST:
        ON_RESPONSE_FLAG_GET_IPCLIST(message);
        break;
    case RESPONSE_FLAG_GET_NETWORK_BANDWIDTH:
        ON_RESPONSE_FLAG_GET_NETWORK_BANDWIDTH(message);
        break;
    case RESPONSE_FLAG_TEST_IPCCONNECT:
    case RESPONSE_FLAG_TRY_TEST_IPCCONNECT:
        ON_RESPONSE_FLAG_TEST_IPCCONNECT(message);
        break;
    case RESPONSE_FLAG_GET_IPCPARAM:
        ON_RESPONSE_FLAG_GET_IPCPARAM(message);
        break;
    case RESPONSE_FLAG_SET_IPCPARAM_BATCH:
        ON_RESPONSE_FLAG_SET_IPCPARAM_BATCH(message);
        break;
    case RESPONSE_FLAG_CHECK_IPC_FISHEYE_INFO:
        ON_RESPONSE_FLAG_CHECK_IPC_FISHEYE_INFO(message);
        break;
    case RESPONSE_FLAG_GET_IPC_COMMON_PARAM:
        ON_RESPONSE_FLAG_GET_IPC_COMMON_PARAM(message);
        break;
    case RESPONSE_FLAG_SET_IPCPARAM:
        ON_RESPONSE_FLAG_SET_IPCPARAM_EX(message);
        break;
    case RESPONSE_FLAG_GET_IPC_FRAME_RESOLUTION:
        ON_RESPONSE_FLAG_GET_IPC_FRAME_RESOLUTION(message);
        break;
    case RESPONSE_FLAG_REMOVE_IPC:
        ON_RESPONSE_FLAG_REMOVE_IPC(message);
        break;
    default:
        break;
    }
}

void CameraManagement::updateBatchSettings(const BatchInfo &info)
{
    clearBatchSettings();
    m_ipc_param = info.param;
    m_mainStream = QString("stream_%1").arg(m_ipc_param.mainStream - 1);
    if (m_ipc_param.sub_stream > 0) {
        m_SubStream = QString("stream_%1").arg(m_ipc_param.sub_stream - 1);
    } else {
        m_SubStream = "";
    }
    //transport protocol
    camera db_camera;
    memset(&db_camera, 0, sizeof(camera));
    read_camera(SQLITE_FILE_NAME, &db_camera, m_ipc_param.chnid);
    ui->comboBox_protocol_2->clear();
    ui->comboBox_protocol_2->addItem("Auto", TRANSPROTOCOL_AUTO);
    ui->comboBox_protocol_2->addItem("UDP", TRANSPROTOCOL_UDP);
    ui->comboBox_protocol_2->addItem("TCP", TRANSPROTOCOL_TCP);
    if (db_camera.transmit_protocol > TRANSPROTOCOL_AUTO) {
        db_camera.transmit_protocol = TRANSPROTOCOL_AUTO;
    }
    ui->comboBox_protocol_2->setCurrentIndexFromData(db_camera.transmit_protocol);

    //main video codec
    ui->comboBox_videoCodec_main->clear();
    ui->comboBox_videoCodec_main->addItem(QString("H.264"), CODECTYPE_H264);
    ui->comboBox_videoCodec_main->addItem(QString("H.265"), CODECTYPE_H265);
    ui->comboBox_videoCodec_main->setCurrentIndexFromData(m_ipc_param.main_range.vcodec_type);
    on_comboBox_videoCodec_main_activated(ui->comboBox_videoCodec_main->currentIndex());

    //main bitrate control
    if (m_ipc_param.main_range.rateControl == 0 || m_ipc_param.main_range.rateControl == 1) {
        ui->comboBox_bitrateControl_main->clear();
        ui->comboBox_bitrateControl_main->addItem("CBR", 0);
        ui->comboBox_bitrateControl_main->addItem("VBR", 1);
        ui->comboBox_bitrateControl_main->setEnabled(true);
    } else {
        ui->comboBox_bitrateControl_main->setEnabled(false);
        ui->comboBox_bitrateControl_main->clear();
    }

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

    //i-frame range
    int iframerange = 0;
    if (m_ipc_param.mainStream > 3) {
        iframerange = m_ipc_param.streamGop[0];
    } else {
        iframerange = m_ipc_param.streamGop[m_ipc_param.mainStream - 1];
    }
    if (!iframerange) {
        iframerange = 120;
        if (m_ipc_param.main_range.framerate_max > 60) {
            iframerange = 240;
        }
        MsCameraVersion version = qMsNvr->cameraVersion(m_ipc_param.chnid);
        if (version >= MsCameraVersion(7, 68)) {
            MsCameraModel cameraModel(qMsNvr->cameraRealModel(m_ipc_param.chnid));
            if (cameraModel.isHighFrameRate_90()) {
                iframerange = 180;
            } else if (cameraModel.isHighFrameRate_120()) {
                iframerange = 240;
            }
        }
        if (!qMsNvr->isMsCamera(m_ipc_param.chnid) && m_ipc_param.main_range.iframeIntervalMax > 0) {
            iframerange = m_ipc_param.main_range.iframeIntervalMax;
        }
    }
    if (ui->comboBox_iFrameInterval_main->count() == 0) {
        for (int i = 0; i < iframerange; ++i) {
            ui->comboBox_iFrameInterval_main->addItem(QString::number(i + 1), i + 1);
        }
    }
    ui->comboBox_iFrameInterval_main->setCurrentIndexFromData(m_ipc_param.main_range.iframeinterval);

    //smart stream
    if (m_ipc_param.supportsmartstream) {
        ui->comboBoxSmartStreamMain->setEnabled(true);

        ui->comboBoxSmartStreamMain->clear();
        ui->comboBoxSmartStreamMain->addItem("Off", 0);
        ui->comboBoxSmartStreamMain->addItem("On", 1);
        ui->comboBoxSmartStreamMain->setCurrentIndexFromData(m_ipc_param.smartStream);
        ui->horizontalSliderMain->setValue(m_ipc_param.smartStream_level);

        ui->horizontalSliderSub->setValue(m_ipc_param.subSmartStream_level);
    } else {
        ui->comboBoxSmartStreamMain->clear();

        ui->comboBoxSmartStreamMain->setEnabled(false);
    }

    //sub enable
    ui->comboBox_secondaryStream->beginEdit();
    ui->comboBox_secondaryStream->clear();
    ui->comboBox_secondaryStream->addItem(GET_TEXT("COMMON/1009", "Enable"), 1);
    ui->comboBox_secondaryStream->addItem(GET_TEXT("COMMON/1018", "Disable"), 0);
    ui->comboBox_secondaryStream->endEdit();
    ui->comboBox_secondaryStream->setCurrentIndexFromData(m_ipc_param.sub_enable);
    on_comboBox_secondaryStream_activated(ui->comboBox_secondaryStream->currentIndex());

    on_comboBoxSmartStreamMain_activated(ui->comboBoxSmartStreamMain->currentIndex());

    //
    if (m_ipc_param.sub_enable) {
        //sub video codec
        ui->comboBox_videoCodec_sub->clear();
        ui->comboBox_videoCodec_sub->addItem(QString("H.264"), CODECTYPE_H264);
        ui->comboBox_videoCodec_sub->addItem(QString("H.265"), CODECTYPE_H265);
        ui->comboBox_videoCodec_sub->setCurrentIndexFromData(m_ipc_param.sub_range.vcodec_type);
        on_comboBox_videoCodec_sub_activated(ui->comboBox_videoCodec_sub->currentIndex());

        //sub bitrate control
        if (m_ipc_param.sub_range.rateControl == 0 || m_ipc_param.sub_range.rateControl == 1) {
            ui->comboBox_bitrateControl_sub->clear();
            ui->comboBox_bitrateControl_sub->addItem("CBR", 0);
            ui->comboBox_bitrateControl_sub->addItem("VBR", 1);
            ui->comboBox_bitrateControl_sub->setEnabled(true);
        } else {
            ui->comboBox_bitrateControl_sub->setEnabled(false);
            ui->comboBox_bitrateControl_sub->clear();
        }

        //bit rate
        ui->comboBox_bitRate_sub->clear();
        for (int i = 0; i < bitRateList.size(); ++i) {
            const int &bitrate = bitRateList.at(i);
            if (bitrate >= m_ipc_param.sub_range.bitrate_min && bitrate <= m_ipc_param.sub_range.bitrate_max) {
                ui->comboBox_bitRate_sub->addItem(QString::number(bitrate), bitrate);
            }
        }
        ui->comboBox_bitRate_sub->setCurrentBitRate(m_ipc_param.sub_range.bitrate);

        //i-frame range
        iframerange = m_ipc_param.streamGop[m_ipc_param.sub_stream - 1];
        if (!iframerange) {
            iframerange = 120;
            MsCameraVersion version = qMsNvr->cameraVersion(m_ipc_param.chnid);
            if (version >= MsCameraVersion(7, 68)) {
                MsCameraModel cameraModel(qMsNvr->cameraRealModel(m_ipc_param.chnid));
                if (cameraModel.isHighFrameRate_90()) {
                    iframerange = 180;
                } else if (cameraModel.isHighFrameRate_120()) {
                    iframerange = 240;
                }
            }
            if (!qMsNvr->isMsCamera(m_ipc_param.chnid) && m_ipc_param.sub_range.iframeIntervalMax > 0) {
                iframerange = m_ipc_param.sub_range.iframeIntervalMax;
            }
        }
        if (ui->comboBox_iFrameInterval_sub->count() == 0) {
            for (int i = 0; i < iframerange; ++i) {
                ui->comboBox_iFrameInterval_sub->addItem(QString::number(i + 1), i + 1);
            }
        }
        ui->comboBox_iFrameInterval_sub->setCurrentIndexFromData(m_ipc_param.sub_range.iframeinterval);

        //frame rate
        //frame rate，5M机型，次码流的最高帧率和主码流同步
        if (m_jsonData.isEmpty()) {
            MsCameraModel cameraModel(qMsNvr->cameraRealModel(m_ipc_param.chnid));
            if (cameraModel.is5MpModel()) {
                ui->comboBox_maxFrameRate_sub->clear();
                for (int i = 0; i < ui->comboBox_maxFrameRate_main->count(); ++i) {
                    ui->comboBox_maxFrameRate_sub->addItem(QString::number(i + 1), i + 1);
                }
            } else {
                ui->comboBox_maxFrameRate_sub->clear();
                for (int i = 0; i < m_ipc_param.sub_range.framerate_max; ++i) {
                    ui->comboBox_maxFrameRate_sub->addItem(QString::number(i + 1), i + 1);
                }
            }
        }
        if (!m_ipc_param.supportsmartstream) {
            ui->comboBoxSmartStreamSub->clear();
            ui->comboBoxSmartStreamSub->setEnabled(false);
        } else {
            ui->comboBoxSmartStreamSub->setEnabled(true);
            ui->comboBoxSmartStreamSub->clear();
            ui->comboBoxSmartStreamSub->addItem("Off", 0);
            ui->comboBoxSmartStreamSub->addItem("On", 1);
            ui->comboBoxSmartStreamSub->setCurrentIndexFromData(m_ipc_param.subSmartStream);
        }

        ui->comboBox_maxFrameRate_sub->setCurrentIndexFromData(m_ipc_param.sub_range.framerate);
    }
    on_comboBoxSmartStreamSub_activated(ui->comboBoxSmartStreamSub->currentIndex());
    //
    ui->comboBox_bitrateControl_main->setCurrentIndexFromData(QString(info.main_common.info.value).toInt());
    ui->comboBox_bitrateControl_sub->setCurrentIndexFromData((QString(info.sub_common.info.value).toInt()));
}

void CameraManagement::ON_RESPONSE_FLAG_GET_IPCLIST(MessageReceive *message)
{
    Q_UNUSED(message)
}

void CameraManagement::ON_RESPONSE_FLAG_GET_NETWORK_BANDWIDTH(MessageReceive *message)
{
    //MsWaitting::closeGlobalWait();
    struct resp_network_bandwidth *info = (struct resp_network_bandwidth *)message->data;
    if (!info) {
        qWarning() << "CameraManagement::ON_RESPONSE_FLAG_GET_NETWORK_BANDWIDTH, data is null.";
        return;
    }
    qMsDebug() << "total:" << info->total
               << ", free:" << info->free
               << ", used:" << info->used;
    ui->label_band->setText(GET_TEXT("CHANNELMANAGE/30037", "Free Receiving Bandwidth: %1Mbps").arg(info->free / 1024.0, 0, 'f', 2));

    //
    refreshUnusedChannelList();
}

void CameraManagement::ON_RESPONSE_FLAG_TEST_IPCCONNECT(MessageReceive *message)
{
    //MsWaitting::closeGlobalWait();
    if (message->data) {
        QString strMessage;
        m_ipcTestResult = static_cast<IPC_CONN_RES>(*((int *)message->data));
        switch (m_ipcTestResult) {
        case IPC_OK:
            break;
        case IPC_NETWORK_ERR:
            strMessage = GET_TEXT("CHANNELMANAGE/30040", "Network Error.");
            break;
        case IPC_INVALID_USER:
            strMessage = GET_TEXT("CHANNELMANAGE/30041", "User Name or Password Error.");
            break;
        case IPC_UNKNOWN_ERR:
            strMessage = GET_TEXT("CHANNELMANAGE/30042", "Unknown Error.");
            break;
        case IPC_PROTO_NOT_SUPPORT:
            strMessage = GET_TEXT("CHANNELMANAGE/30043", "Unsupported Protocol.");
            break;
        case IPC_OUT_LIMIT_BANDWIDTH:
            strMessage = "Bandwidth reaches the limit.";
            break;
        default:
            strMessage = GET_TEXT("CHANNELMANAGE/30042", "Unknown Error.");
            break;
        }
        if (m_ipcTestResult != IPC_OK) {
            ShowMessageBox(strMessage);
        }
    }
}

void CameraManagement::ON_RESPONSE_FLAG_GET_IPCPARAM(MessageReceive *message)
{
    struct resp_get_ipc_param *ipc_param = (struct resp_get_ipc_param *)message->data;
    if (!ipc_param) {
        qMsWarning() << message;
        gEventLoopExit(-1);
        return;
    }
    if (ipc_param->ret != 0) {
        qMsWarning() << QString("GET_IPCPARAM failed, channel = %1").arg(ipc_param->chnid);
        gEventLoopExit(ipc_param->ret);
        return;
    }

    m_temp_ipc_param = *ipc_param;
    gEventLoopExit(0);
}

void CameraManagement::ON_RESPONSE_FLAG_SET_IPCPARAM_BATCH(MessageReceive *message)
{
    Q_UNUSED(message);

    //MsWaitting::closeGlobalWait();

    ShowMessageBox(GET_TEXT("CHANNELMANAGE/154003", "Operate successfully."));

    m_channelCheckedList.clear();
    on_pushButton_refresh_clicked();
}

void CameraManagement::ON_RESPONSE_FLAG_CHECK_IPC_FISHEYE_INFO(MessageReceive *message)
{
    memset(&m_fisheye_info, 0, sizeof(m_fisheye_info));
    memcpy(&m_fisheye_info, message->data, sizeof(m_fisheye_info));
    m_eventLoop.exit();
}

void CameraManagement::ON_RESPONSE_FLAG_GET_IPC_COMMON_PARAM(MessageReceive *message)
{
    resp_http_common_param *param = (resp_http_common_param *)message->data;
    if (!param) {
        qMsWarning() << message;
        gEventLoopExit(-1);
        return;
    }
    if (param->res != 0) {
        gEventLoopExit(param->res);
        return;
    }

    m_temp_common_param = *param;

    gEventLoopExit(0);
}

void CameraManagement::ON_RESPONSE_FLAG_SET_IPCPARAM_EX(MessageReceive *message)
{
    Q_UNUSED(message)

    //MsWaitting::closeGlobalWait();

    ShowMessageBox(GET_TEXT("CHANNELMANAGE/154003", "Operate successfully."));
    m_channelCheckedList.clear();
    on_pushButton_refresh_clicked();
}

void CameraManagement::ON_RESPONSE_FLAG_GET_IPC_FRAME_RESOLUTION(MessageReceive *message)
{
    m_jsonData.clear();
    if (message->data && message->header.size > 0) {
        m_jsonData = QByteArray(static_cast<char *>(message->data), message->header.size);
        m_cameraStreamJsonData->parseJson(m_jsonData);
    }
    m_eventLoop.exit();
}

void CameraManagement::ON_RESPONSE_FLAG_REMOVE_IPC(MessageReceive *message)
{
    Q_UNUSED(message)
    m_eventLoop.exit();
}

void CameraManagement::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
}

void CameraManagement::deleteCamera(int channel)
{
    qDebug() << QString("Begin delete camera, channel = %1").arg(channel);

    //
    qMsNvr->closeLiveviewAudio();
    qMsNvr->closeTalkback(channel);

    //mask
    privacy_mask mask;
    read_privacy_mask(SQLITE_FILE_NAME, &mask, channel);
    memset(mask.area, 0, sizeof(mask.area));
    for (int i = 0; i < MAX_MASK_AREA_NUM; i++) {
        mask.area[i].width = 480;
        mask.area[i].height = 270;
    }
    write_privacy_mask(SQLITE_FILE_NAME, &mask);

    //osd
    qMsNvr->setOSDName(channel, QString("CAM%1").arg(channel + 1));

    //camera
    camera cameraInfo;
    memset(&cameraInfo, 0, sizeof(cameraInfo));
    read_camera(SQLITE_FILE_NAME, &cameraInfo, channel);

    //log
    struct op_lr_a_d_e_ip_channel olic;
    memset(&olic, 0, sizeof(struct op_lr_a_d_e_ip_channel));
    snprintf(olic.ip, sizeof(olic.ip), "%s", cameraInfo.ip_addr);
    olic.action = SUB_PARAM_DEL_IPC;
    olic.port = cameraInfo.manage_port;
    olic.channel = channel + 1;
    olic.timeset = cameraInfo.sync_time;
    switch (cameraInfo.camera_protocol) {
    case IPC_PROTOCOL_ONVIF:
        snprintf(olic.protocal, sizeof(olic.protocal), "%s", "ONVIF");
        break;
    case IPC_PROTOCOL_RTSP:
        snprintf(olic.protocal, sizeof(olic.protocal), "%s", "RTSP");
        break;
    case IPC_PROTOCOL_MILESIGHT:
        snprintf(olic.protocal, sizeof(olic.protocal), "%s", "MSSF");
        break;
    default:
        break;
    }
    switch (cameraInfo.transmit_protocol) {
    case TRANSPROTOCOL_UDP:
        snprintf(olic.tran_protocal, sizeof(olic.tran_protocal), "%s", "UDP");
        break;
    case TRANSPROTOCOL_TCP:
        snprintf(olic.tran_protocal, sizeof(olic.tran_protocal), "%s", "TCP");
        break;
    case TRANSPROTOCOL_AUTO:
        snprintf(olic.tran_protocal, sizeof(olic.tran_protocal), "%s", "Auto");
        break;
    default:
        break;
    }
    //
    struct log_data log_data;
    memset(&log_data, 0, sizeof(struct log_data));
    log_data.log_data_info.subType = SUP_OP_DELETE_IP_CHANNEL_LOCK;
    log_data.log_data_info.parameter_type = SUB_PARAM_DEL_IPC;
    snprintf(log_data.log_data_info.user, sizeof(log_data.log_data_info.user), "%s", gMsUser.userName().toStdString().c_str());
    log_data.log_data_info.chan_no = channel + 1;
    msfs_log_pack_detail(&log_data, OP_A_D_E_IP_CHANNEL, &olic, sizeof(olic));
    MsWriteLog(log_data);

    //
    int brightness = cameraInfo.brightness;
    int contrast = cameraInfo.contrast;
    int saturation = cameraInfo.saturation;
    int record_stream = cameraInfo.record_stream;
    memset(&cameraInfo, 0, sizeof(cameraInfo));
    cameraInfo.id = channel;
    cameraInfo.enable = 0;
    cameraInfo.type = CAMERA_TYPE_IPNC;
    cameraInfo.brightness = brightness;
    cameraInfo.contrast = contrast;
    cameraInfo.saturation = saturation;
    cameraInfo.record_stream = record_stream;
    qMsNvr->writeDatabaseCamera(&cameraInfo);

    //
    sendMessage(REQUEST_FLAG_REMOVE_IPC, (void *)&channel, sizeof(int));
    //m_eventLoop.exec();

    //
    struct other_poe_camera other_ipc;
    other_ipc.other_poe_channel = 0;
    other_ipc.id = channel;
    Q_UNUSED(other_ipc)

    qDebug() << QString("End delete camera, channel = %1").arg(channel);
}

bool CameraManagement::isInputValid()
{
    bool valid = true;
    valid &= ui->lineEdit_channelName->checkValid();

    switch (ui->comboBox_protocol->currentIntData()) {
    case IPC_PROTOCOL_RTSP:
        valid &= ui->lineEdit_primary->checkValid();
        if (!ui->lineEdit_secondary->isEmpty()) {
            valid &= ui->lineEdit_secondary->checkValid();
        }
        break;
    case IPC_PROTOCOL_MSDOMAIN: {
        QRegExp rx("(^ddns\\.milesight\\.com\\/)([0-9a-fA-F]{12})");
        if (rx.indexIn(ui->lineEdit_ip->text()) == -1) {
            ui->lineEdit_ip->setCustomValid(false, GET_TEXT("MYLINETIP/112001", "Invalid."));
            valid = false;
        }
        break;
    }
    default:
        valid &= ui->lineEdit_ip->checkValid();
        valid &= ui->lineEdit_port->checkValid();
        break;
    }
    if (ui->comboBox_transportProtocol->currentData() == TRANSPROTOCOL_ROH) {
        valid &= ui->lineEditHTTPSPort->checkValid();
    }
    valid &= ui->lineEdit_userName->checkValid();
    if (!valid) {
        return false;
    }
    return true;
}

void CameraManagement::testIpcConnect()
{
    struct req_test_ipcconnect test_ipcconnect;
    memset(&test_ipcconnect, 0, sizeof(test_ipcconnect));
    snprintf(test_ipcconnect.ip, sizeof(test_ipcconnect.ip), "%s", ui->lineEdit_ip->text().toStdString().c_str());
    test_ipcconnect.port = ui->lineEdit_port->text().toInt();
    snprintf(test_ipcconnect.user, sizeof(test_ipcconnect.user), "%s", ui->lineEdit_userName->text().toStdString().c_str());
    snprintf(test_ipcconnect.password, sizeof(test_ipcconnect.password), "%s", ui->lineEdit_password->text().toStdString().c_str());
    test_ipcconnect.protocol = ui->comboBox_protocol->currentData().toInt();
    test_ipcconnect.transportProtocol = ui->comboBox_transportProtocol->currentIntData();
    if (ui->comboBox_protocol->currentIntData() == IPC_PROTOCOL_MSDOMAIN) {
        snprintf(test_ipcconnect.ddns, sizeof(test_ipcconnect.ddns), "%s", ui->lineEdit_ip->text().toStdString().c_str());
    }
    //test_ipcconnect.model = ui->comboBox_model->itemData(ui->comboBox_model->currentIndex()).value<int>();
    sendMessage(REQUEST_FLAG_TEST_IPCCONNECT, (void *)&test_ipcconnect, sizeof(test_ipcconnect));
}

void CameraManagement::refreshUnusedChannelList()
{
    ui->comboBox_channel->clear();

    QList<int> channelList = qMsNvr->disabledCameraList();
    for (int i = 0; i < channelList.size(); ++i) {
        int channel = channelList.at(i);
        ui->comboBox_channel->addItem(QString::number(channel + 1), channel);
    }
    ui->comboBox_channel->setCurrentIndex(0);
    on_comboBox_channel_activated(0);
}

void CameraManagement::clearBatchSettings()
{
    ui->comboBox_protocol_2->clear();
    ui->comboBoxSmartStreamMain->clear();
    ui->comboBoxSmartStreamMain->setEnabled(true);
    on_comboBoxSmartStreamMain_activated(0);

    ui->comboBoxSmartStreamSub->clear();
    on_comboBoxSmartStreamSub_activated(0);

    ui->comboBox_secondaryStream->clear();
    on_comboBox_secondaryStream_activated(0);
    ui->comboBox_bitrateControl_sub->setEnabled(true);
    ui->comboBoxSmartStreamSub->setEnabled(true);

    ui->comboBox_videoCodec_main->clear();
    ui->comboBox_frameSize_main->clear();
    ui->comboBox_maxFrameRate_main->clear();
    ui->comboBox_bitRate_main->clear();
    ui->comboBox_bitrateControl_main->clear();
    ui->comboBox_iFrameInterval_main->clear();

    ui->comboBox_videoCodec_sub->clear();
    ui->comboBox_frameSize_sub->clear();
    ui->comboBox_maxFrameRate_sub->clear();
    ui->comboBox_bitRate_sub->clear();
    ui->comboBox_bitrateControl_sub->clear();
    ui->comboBox_iFrameInterval_sub->clear();
    ui->checkBox_timeSetting_2->setChecked(false);
}

void CameraManagement::clearCameraManagement()
{
    int defaultTransport = get_param_int(SQLITE_FILE_NAME, PARAM_TRANSPORT_PROTOCOL, 0);
    ui->comboBox_transportProtocol->setCurrentIndexFromData(defaultTransport);

    ui->lineEdit_ip->clear();
    ui->lineEdit_port->setText(QString("%1").arg(80));
    ui->lineEditHTTPSPort->setText(QString("%1").arg(443));
    ui->lineEdit_userName->setText("admin");
    ui->lineEdit_password->clear();
    ui->checkBox_timeSetting->setChecked(false);

    //protocol
    m_msddns = "ddns.milesight.com/";
    m_primary = "rtsp://";
    m_secondary = "rtsp://";
    m_ipAddress = "";
    m_perProtocol = IPC_PROTOCOL_ONVIF;
    ui->lineEdit_primary->setText(m_primary);
    ui->lineEdit_secondary->setText(m_secondary);
    ui->comboBox_protocol->setCurrentIndexFromData(0);
    on_comboBox_protocol_activated(0);
    on_comboBox_transportProtocol_activated(0);
}

CameraManagement::BatchInfo CameraManagement::getBatchInfo(int channel)
{
    qMsDebug() << "channel:" << channel;
    //MsWaitting::showGlobalWait();
    sendMessage(REQUEST_FLAG_GET_IPC_FRAME_RESOLUTION, &channel, sizeof(int));
    //m_eventLoop.exec();
    BatchInfo info;
    do {
        sendMessage(REQUEST_FLAG_GET_IPCPARAM, &channel, sizeof(int));
        if (gEventLoopExec() != 0) {
            break;
        }
        info.param = m_temp_ipc_param;
        //
        struct req_http_common_param req;
        memset(&req, 0, sizeof(struct req_http_common_param));
        req.chnid = channel;
        snprintf(req.info.httpname, sizeof(req.info.httpname), "%s", "vsetting.html");
        snprintf(req.info.pagename, sizeof(req.info.pagename), "%s", "video.0");
        snprintf(req.info.key, sizeof(req.info.key), "%s", "ratecontrol1");
        sendMessage(REQUEST_FLAG_GET_IPC_COMMON_PARAM, &req, sizeof(struct req_http_common_param));
        if (gEventLoopExec() != 0) {
            break;
        }
        info.main_common = m_temp_common_param;

        if (ui->comboBox_secondaryStream->currentIntData()) {
            memset(&req, 0, sizeof(struct req_http_common_param));
            req.chnid = channel;
            snprintf(req.info.httpname, sizeof(req.info.httpname), "%s", "vsetting_sub.html");
            snprintf(req.info.pagename, sizeof(req.info.pagename), "%s", "video.1");
            snprintf(req.info.key, sizeof(req.info.key), "%s", "ratecontrol2");
            sendMessage(REQUEST_FLAG_GET_IPC_COMMON_PARAM, &req, sizeof(struct req_http_common_param));
            if (gEventLoopExec() != 0) {
                break;
            }
            info.sub_common = m_temp_common_param;
        }

        info.channel = channel;
        //MsWaitting::closeGlobalWait();
        return info;
    } while (0);
    //MsWaitting::closeGlobalWait();
    return info;
}

void CameraManagement::lineEditIPShow(bool enable)
{
    if (enable) {
        ui->label_primary->hide();
        ui->label_secondary->hide();
        ui->lineEdit_primary->hide();
        ui->lineEdit_secondary->hide();

        if (ui->comboBox_protocol->currentIntData() != IPC_PROTOCOL_MSDOMAIN) {
            ui->label_port->show();
            ui->lineEdit_port->show();
        }
        ui->label_ip->show();
        ui->lineEdit_ip->show();

    } else {
        ui->label_ip->hide();
        ui->label_port->hide();
        ui->lineEdit_ip->hide();
        ui->lineEdit_port->hide();

        ui->label_primary->show();
        ui->label_secondary->show();
        ui->lineEdit_primary->show();
        ui->lineEdit_secondary->show();
    }
}

void CameraManagement::updateFrameRateMain(int maxFrameRate, int currentFrameRate, MsCameraModel cameraModel)
{
    //qWarning() << "CameraEdit::updateFrameRateMain, maxFrameRate:" << maxFrameRate << ", currentFrameRate:" << currentFrameRate;
    ui->comboBox_maxFrameRate_main->clear();
    for (int i = 1; i <= maxFrameRate; ++i) {
        ui->comboBox_maxFrameRate_main->addItem(QString("%1").arg(i), i);
    }
    //5M机型，次码流的最高帧率和主码流同步
    if (cameraModel.is5MpModel()) {
        qDebug() << QString("5Mp Model, sub MaxFrameRate is same with main MaxFrameRate: %1").arg(maxFrameRate);
        ui->comboBox_maxFrameRate_sub->clear();
        for (int i = 1; i <= maxFrameRate; ++i) {
            ui->comboBox_maxFrameRate_sub->addItem(QString("%1").arg(i), i);
        }
        ui->comboBox_maxFrameRate_sub->setCurrentIndexFromData(m_ipc_param.sub_range.framerate);
    }

    //
    int index = ui->comboBox_maxFrameRate_main->findData(currentFrameRate);
    if (index < 0) {
        ui->comboBox_maxFrameRate_main->setCurrentIndexFromData(maxFrameRate);
    } else {
        ui->comboBox_maxFrameRate_main->setCurrentIndexFromData(currentFrameRate);
    }
}

void CameraManagement::onLanguageChanged()
{
    ui->label_channel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));
    ui->label_channelName->setText(GET_TEXT("CHANNELMANAGE/30009", "Channel Name"));
    ui->label_protocol->setText(GET_TEXT("CHANNELMANAGE/30014", "Protocol"));
    ui->label_ip->setText(GET_TEXT("COMMON/1033", "IP Address"));
    ui->label_primary->setText(GET_TEXT("CHANNELMANAGE/30024", "Primary"));
    ui->label_secondary->setText(GET_TEXT("CHANNELMANAGE/30017", "Secondary"));
    ui->label_transportProtocol->setText(GET_TEXT("CHANNELMANAGE/30015", "Transport Protocol"));
    ui->label_userName->setText(GET_TEXT("COMMON/1007", "User Name"));
    ui->label_password->setText(GET_TEXT("COMMON/1008", "Password"));
    ui->label_timeSetting->setText(GET_TEXT("CHANNELMANAGE/30018", "Time Setting"));
    ui->checkBox_timeSetting->setText(GET_TEXT("CHANNELMANAGE/30019", "Sync Time With NVR"));
    ui->pushButton_test->setText(GET_TEXT("CHANNELMANAGE/30020", "Test"));
    ui->pushButton_add->setText(GET_TEXT("COMMON/1000", "Add"));

    ui->label_protocol_2->setText(GET_TEXT("CHANNELMANAGE/30015", "Transport Protocol"));
    ui->label_secondaryStream->setText(GET_TEXT("CHANNELMANAGE/30057", "Secondary Stream"));
    ui->label_videoCodec_main->setText(GET_TEXT("CHANNELMANAGE/30056", "Video Codec"));
    ui->label_videoCodec_sub->setText(GET_TEXT("CHANNELMANAGE/30056", "Video Codec"));
    ui->label_frameSize_main->setText(GET_TEXT("CHANNELMANAGE/30058", "Frame Size"));
    ui->label_frameSize_sub->setText(GET_TEXT("CHANNELMANAGE/30058", "Frame Size"));
    ui->label_maxFrameRate_main->setText(GET_TEXT("CHANNELMANAGE/30059", "Max Frame Rate"));
    ui->label_maxFrameRate_sub->setText(GET_TEXT("CHANNELMANAGE/30059", "Max Frame Rate"));
    ui->label_bitRate_main->setText(GET_TEXT("CHANNELMANAGE/30060", "Bit Rate"));
    ui->label_bitRate_sub->setText(GET_TEXT("CHANNELMANAGE/30060", "Bit Rate"));
    ui->label_bitrateControl_main->setText(GET_TEXT("CAMERAEDIT/31016", "Bitrate Control"));
    ui->label_bitrateControl_sub->setText(GET_TEXT("CAMERAEDIT/31016", "Bitrate Control"));
    ui->label_iFrameInterval_main->setText(GET_TEXT("CAMERAEDIT/31017", "I-frame Interval"));
    ui->label_iFrameInterval_sub->setText(GET_TEXT("CAMERAEDIT/31017", "I-frame Interval"));
    ui->label_timeSetting_2->setText(GET_TEXT("CHANNELMANAGE/30018", "Time Setting"));
    ui->checkBox_timeSetting_2->setText(GET_TEXT("CHANNELMANAGE/30019", "Sync Time With NVR"));
    ui->pushButton_apply->setText(GET_TEXT("COMMON/1003", "Apply"));

    ui->pushButton_authentication->setText(GET_TEXT("CHANNELMANAGE/30067", "Edit Authentication"));
    ui->pushButton_delete->setText(GET_TEXT("CHANNELMANAGE/30023", "Delete"));
    ui->pushButton_refresh->setText(GET_TEXT("COMMON/1035", "Refresh"));
    ui->pushButton_back->setText(GET_TEXT("COMMON/1002", "Back"));

    ui->labelSmartStreamMain->setText(GET_TEXT("CAMERAEDIT/31018", "Smart Stream"));
    ui->labelSmartStreamSub->setText(GET_TEXT("CAMERAEDIT/31018", "Smart Stream"));
    ui->labelLevelMain->setText(GET_TEXT("CAMERAEDIT/31019", "Level"));
    ui->labelLevelMain->setText(GET_TEXT("CAMERAEDIT/31019", "Level"));

    ui->labelHTTPSPort->setText(GET_TEXT("SYSTEMNETWORK/71059", "HTTPS Port"));
}

void CameraManagement::onTabClicked(int index)
{
    m_currentTab = index;
    m_channelCheckedList.clear();
    ui->stackedWidget->setCurrentIndex(index);
    ui->tableView->clearSort();

    on_pushButton_refresh_clicked();
    if (m_currentTab == TabBatchSetting) {
        ui->pushButton_apply->show();
    } else if (m_currentTab == TabCameraManagement) {
        clearCameraManagement();
        ui->pushButton_apply->hide();
    }
}

void CameraManagement::onTableItemClicked(int row, int column)
{
    int channel = ui->tableView->itemText(row, ColumnChannel).toInt() - 1;
    if (channel < 0) {
        qWarning() << QString("CameraManagement::onTableItemClicked, row: %1, column: %2, channel: %3").arg(row).arg(column).arg(channel);
        return;
    }

    if (m_currentRow != row) {
        m_currentRow = row;
    }

    bool checked = ui->tableView->isItemChecked(row);
    ui->widget_video->playVideo(channel);

    switch (column) {
    case ColumnCheck: {
        QString protocol = ui->tableView->itemText(row, ColumnProtocol);

        if (checked) {
            m_readyDeleteMap.insert(row, channel);
            m_channelCheckedList.append(channel);
            if (m_currentTab == TabBatchSetting && protocol != "RTSP") {
                BatchInfo info = getBatchInfo(channel);
                if (info.channel >= 0) {
                    m_batchInfoMap.insert(channel, info);
                    //MsWaitting::showGlobalWait();
                    updateBatchSettings(info);
                    //MsWaitting::closeGlobalWait();
                }
            }
        } else {
            if (!m_readyDeleteMap.isEmpty()) {
                for (auto iter = m_readyDeleteMap.begin(); iter != m_readyDeleteMap.end(); ++iter) {
                    if (iter.key() == row && iter.value() == channel) {
                        m_readyDeleteMap.erase(iter);
                        break;
                    }
                }
            }
            m_channelCheckedList.removeAll(channel);
            m_batchInfoMap.remove(channel);
            if (protocol == "RTSP") {
                break;
            }
            if (m_currentTab == TabBatchSetting) {
                QMetaObject::invokeMethod(this, "getBatchParameters", Qt::QueuedConnection);
            }
        }
        break;
    }
    case ColumnEdit: {
        const resq_get_ipcdev &ipcdev = m_ipcdevMap.value(channel);
        m_cameraEdit->showEdit(channel, ipcdev);
        const int result = m_cameraEdit->exec();
        if (result == CameraEdit::Accepted) {
            //MsWaitting::showGlobalWait();
            QTimer::singleShot(2000, this, SLOT(refreshData()));
        }
        break;
    }
    case ColumnDelete: {
        //MsWaitting::showGlobalWait();
        if (m_ipcdevMap.contains(channel)) {
            auto ipc = m_ipcdevMap.value(channel);
            if (ipc.poe_channel) {
                //MsWaitting::closeGlobalWait();
                ShowMessageBox(GET_TEXT("CHANNELMANAGE/30063", "The PoE channel could not be deleted."));
                return;
            }
        }
        const MessageBox::Result &result = MessageBox::question(this, GET_TEXT("CHANNELMANAGE/30033", "Do you want to delete channel %1 ?").arg(channel + 1));
        if (result == MessageBox::Yes) {
            if (!m_readyDeleteMap.isEmpty()) {
                for (auto iter = m_readyDeleteMap.begin(); iter != m_readyDeleteMap.end(); ++iter) {
                    if (iter.key() == row && iter.value() == channel) {
                        m_readyDeleteMap.erase(iter);
                        break;
                    }
                }
            }

            deleteCamera(channel);
            ui->tableView->removeRow(row);
            m_readyDeleteMap.remove(row);
            m_channelCheckedList.removeAll(channel);
            refreshData();
            //sendMessage(REQUEST_FLAG_GET_NETWORK_BANDWIDTH, (void *)NULL, 0);
            //
            LiveView::instance()->resetSubLayoutMode();
        } else {
            //MsWaitting::closeGlobalWait();
        }
        break;
    }
    default:
        break;
    }
}

void CameraManagement::onTableHeaderChecked(bool checked)
{
    qDebug() << "Table Checked:" << checked << " Tab:" << m_currentTab;

    m_readyDeleteMap.clear();
    m_channelCheckedList.clear();
    m_batchInfoMap.clear();

    if (checked) {
        for (int i = 0; i < ui->tableView->rowCount(); i++) {
            int channel = ui->tableView->itemText(i, ColumnChannel).toInt() - 1;
            m_readyDeleteMap.insert(i, channel);
            m_channelCheckedList.append(channel);
        }
    } else {
    }

    if (m_currentTab == TabBatchSetting) {
        QMetaObject::invokeMethod(this, "getBatchParameters", Qt::QueuedConnection);
    }
}

void CameraManagement::onTableDoubleClicked(QModelIndex index)
{
    if (index.isValid()) {
        int channel = ui->tableView->itemText(index.row(), ColumnChannel).toInt() - 1;
        qDebug() << "[david debug] onTableDoubleClicked row:" << index.row() << " channel:" << channel;
        m_cameraEdit->showEdit(channel);
        const int result = m_cameraEdit->exec();
        if (result == CameraEdit::Accepted) {
            on_pushButton_refresh_clicked();
        }
    }

    return;
}

void CameraManagement::getBatchParameters()
{
    clearBatchSettings();
    if (m_channelCheckedList.isEmpty()) {
        return;
    }

    //MsWaitting::showGlobalWait();

    //取最早通道
    for (int i = 0; i < m_channelCheckedList.size(); ++i) {
        int channel = m_channelCheckedList.at(i);
        const resq_get_ipcdev &ipcdev = m_ipcdevMap.value(channel);
        if (ipcdev.state != RTSP_CLIENT_CONNECT && ipcdev.sub_state != RTSP_CLIENT_CONNECT) {
            continue;
        }

        //
        BatchInfo info;
        if (m_batchInfoMap.contains(channel)) {
            info = m_batchInfoMap.value(channel);
        } else {
            info = getBatchInfo(channel);
            if (info.channel < 0) {
                continue;
            }
            m_batchInfoMap.insert(channel, info);
        }

        //
        updateBatchSettings(info);
        break;
    }

    //MsWaitting::closeGlobalWait();
}

void CameraManagement::refreshData()
{
    sendMessage(REQUEST_FLAG_GET_IPCLIST, (void *)NULL, 0);
    sendMessage(REQUEST_FLAG_GET_NETWORK_BANDWIDTH, (void *)NULL, 0);
}

void CameraManagement::on_comboBox_channel_activated(int index)
{
    Q_UNUSED(index)

    int channel = ui->comboBox_channel->currentText().toInt() - 1;
    ui->lineEdit_channelName->setText(qMsNvr->channelName(channel));
}

void CameraManagement::on_comboBox_protocol_activated(int index)
{
    const IPC_PROTOCOL &protocol = static_cast<IPC_PROTOCOL>(ui->comboBox_protocol->itemData(index).toInt());
    if (m_perProtocol == IPC_PROTOCOL_ONVIF || m_perProtocol == IPC_PROTOCOL_MILESIGHT) {
        m_ipAddress = ui->lineEdit_ip->text();
    }
    int transportProtocolIndex = ui->comboBox_transportProtocol->currentIndex();
    ui->widget_5->hide();
    switch (protocol) {
    case IPC_PROTOCOL_ONVIF:
        ui->label_ip->setText(GET_TEXT("COMMON/1033", "IP Address"));
        ui->lineEdit_ip->setText(m_ipAddress);
        lineEditIPShow(true);
        ui->comboBox_transportProtocol->removeItemFromData(TRANSPROTOCOL_ROH);
        break;
    case IPC_PROTOCOL_MILESIGHT:
        ui->label_ip->setText(GET_TEXT("COMMON/1033", "IP Address"));
        ui->lineEdit_ip->setText(m_ipAddress);
        lineEditIPShow(true);
        ui->comboBox_transportProtocol->addItem(GET_TEXT("CHANNELMANAGE/174000", "Encryption"), TRANSPROTOCOL_ROH);
        break;
    case IPC_PROTOCOL_RTSP:
        lineEditIPShow(false);
        ui->comboBox_transportProtocol->removeItemFromData(TRANSPROTOCOL_ROH);
        break;
    case IPC_PROTOCOL_MSDOMAIN:
        ui->lineEdit_ip->setText(m_msddns);
        ui->label_ip->setText(GET_TEXT("CAMERA/143000", "Domain Address"));
        ui->label_port->hide();
        ui->lineEdit_port->hide();
        lineEditIPShow(true);
        ui->widget_5->show();
        ui->comboBox_transportProtocol->removeItemFromData(TRANSPROTOCOL_ROH);
        break;
    default:
        break;
    }
    if (transportProtocolIndex == TRANSPROTOCOL_ROH && ui->comboBox_transportProtocol->count() < TRANSPROTOCOL_MAX) {
        ui->comboBox_transportProtocol->setCurrentIndexFromData(TRANSPROTOCOL_AUTO);
    }
    on_comboBox_transportProtocol_activated(ui->comboBox_transportProtocol->currentIntData());
    m_perProtocol = protocol;
}

void CameraManagement::on_pushButton_test_clicked()
{
    if (!isInputValid()) {
        return;
    }

    testIpcConnect();
    MsWaitting::execGlobalWait();
    if (m_ipcTestResult == IPC_OK) {
        ShowMessageBox(GET_TEXT("CHANNELMANAGE/30053", "Connect Successfully."));
    }
}

void CameraManagement::on_pushButton_add_clicked()
{
    
}

void CameraManagement::on_pushButton_refresh_clicked()
{
    //MsWaitting::showGlobalWait();
    //ui->tableView->hide();
    refreshData();
}

void CameraManagement::on_pushButton_authentication_clicked()
{
    if (m_channelCheckedList.size() == 0) {
        ShowMessageBox(GET_TEXT("CHANNELMANAGE/30034", "No camera is selected."));
        return;
    }
    if (!m_authentication) {
        m_authentication = new Authentication(this);
        connect(m_authentication, SIGNAL(changed()), this, SLOT(onAuthChanged()));
    }
    m_authentication->initializeData();
    for (int i = 0; i < m_channelCheckedList.size(); i++) {
        m_authentication->append_edit_authentication(m_channelCheckedList.at(i));
    }
    m_authentication->show();
}

void CameraManagement::on_pushButton_delete_clicked()
{
    int is_poe_channel = 0;
    if (m_readyDeleteMap.isEmpty()) {
        ShowMessageBox(GET_TEXT("CHANNELMANAGE/30034", "No camera is selected."));
        return;
    } else {
        int result = MessageBox::question(this, GET_TEXT("CHANNELMANAGE/30035", "Do you want to delete these channels ?"));
        if (result == MessageBox::Cancel) {
            return;
        }
        //MsWaitting::showGlobalWait();
        for (auto iter = m_readyDeleteMap.constBegin(); iter != m_readyDeleteMap.constEnd(); ++iter) {
            //int row = iter.key();
            int channel = iter.value();
            if (m_ipcdevMap.contains(channel)) {
                auto ipc = m_ipcdevMap.value(channel);
                if (ipc.poe_channel) {
                    is_poe_channel = 1;
                    continue;
                }
            }
            qApp->processEvents();
            deleteCamera(channel);
        }
        if (is_poe_channel) {
            ShowMessageBox(GET_TEXT("CHANNELMANAGE/30063", "The PoE channel could not be deleted."));
        }
        m_readyDeleteMap.clear();
        m_channelCheckedList.clear();
        refreshData();
    }

    //
    LiveView::instance()->resetSubLayoutMode();
}

void CameraManagement::on_pushButton_back_clicked()
{
    emit sig_back();
}

void CameraManagement::on_comboBox_secondaryStream_activated(int index)
{
    bool enable = !index;
    bool bitrateControlSupport = enable && (ui->comboBox_bitrateControl_sub->count() > 0 || !(ui->comboBox_frameSize_sub->count() > 0));
    bool SmartStreamSubSupoort = enable && (ui->comboBoxSmartStreamSub->count() > 0 || !(ui->comboBox_frameSize_sub->count() > 0));
    ui->comboBox_videoCodec_sub->setEnabled(enable);
    ui->comboBox_frameSize_sub->setEnabled(enable);
    ui->comboBox_maxFrameRate_sub->setEnabled(enable);
    ui->comboBox_bitRate_sub->setEnabled(enable);
    if (ui->horizontalSliderSub->isVisible()) {
        ui->comboBox_bitrateControl_sub->setEnabled(false);
    } else {
        ui->comboBox_bitrateControl_sub->setEnabled(bitrateControlSupport);
    }
    ui->comboBox_iFrameInterval_sub->setEnabled(enable);
    ui->comboBoxSmartStreamSub->setEnabled(SmartStreamSubSupoort);
}

void CameraManagement::on_pushButton_apply_clicked()
{
    if (m_channelCheckedList.isEmpty() || ui->comboBox_protocol_2->count() == 0) {
        return;
    }

    struct req_set_ipc_param_batch param;
    memset(&param, 0, sizeof(param));
    param.framerate = ui->comboBox_maxFrameRate_main->currentData().toInt();
    param.bitrate = ui->comboBox_bitRate_main->currentBitRate();
    QString resolution = ui->comboBox_frameSize_main->currentText();
    QStringList resolutionList = resolution.split("*");
    if (resolutionList.size() == 2) {
        param.width = resolutionList.at(0).toInt();
        param.height = resolutionList.at(1).toInt();
    }
    param.codec = ui->comboBox_videoCodec_main->currentData().toInt();
    param.ratecontrol = ui->comboBox_bitrateControl_main->currentData().toInt();
    param.iframeinterval = ui->comboBox_iFrameInterval_main->currentData().toInt();

    param.substream_enable = ui->comboBox_secondaryStream->currentIntData();
    param.sub_ratecontrol = -1;
    if (param.substream_enable) {
        param.sub_framerate = ui->comboBox_maxFrameRate_sub->currentData().toInt();
        param.sub_bitrate = ui->comboBox_bitRate_sub->currentBitRate();
        QStringList resolutionList = ui->comboBox_frameSize_sub->currentText().split("*");
        if (resolutionList.size() == 2) {
            param.sub_width = resolutionList.at(0).toInt();
            param.sub_height = resolutionList.at(1).toInt();
        }
        param.sub_codec = ui->comboBox_videoCodec_sub->currentData().toInt();
        param.sub_ratecontrol = ui->comboBox_bitrateControl_sub->currentData().toInt();
        param.subiframeinterval = ui->comboBox_iFrameInterval_sub->currentData().toInt();
    }
    param.transmit_protocol = ui->comboBox_protocol_2->currentData().toInt();
    param.sync_time = ui->checkBox_timeSetting_2->isChecked();
    for (int i = 0; i < m_channelCheckedList.size(); ++i) {
        const int &channel = m_channelCheckedList.at(i);
        param.chnid[channel] = 1;
        param.videoType_enable[channel] = 1;
    }

    //采用新接口设置，req_set_ipc_param_batch后续可能会移除
    req_set_ipc_paramex set_ipc_paramex;
    memset(&set_ipc_paramex, 0, sizeof(set_ipc_paramex));
    //
    for (int i = 0; i < qMsNvr->maxChannel(); ++i) {
        camera db_camera;
        memset(&db_camera, 0, sizeof(camera));
        read_camera(SQLITE_FILE_NAME, &db_camera, i);
        if (m_channelCheckedList.contains(i) && !qMsNvr->isFisheye(i) && db_camera.transmit_protocol != TRANSPROTOCOL_ROH && db_camera.camera_protocol != IPC_PROTOCOL_RTSP) {
            set_ipc_paramex.chnid[i] = '1';
        } else {
            set_ipc_paramex.chnid[i] = '0';
        }
    }
    qMsDebug() << "req_set_ipc_paramex.chnid:" << set_ipc_paramex.chnid;
    set_ipc_paramex.trans_protocol = param.transmit_protocol;
    set_ipc_paramex.sync_time = param.sync_time;
    set_ipc_paramex.audio_enable = param.audio_enable;
    set_ipc_paramex.videoType_enable = 1;
    set_ipc_paramex.subEnable = param.substream_enable;
    //
    req_set_ipc_subparam &param_main = set_ipc_paramex.stream[STREAM_TYPE_MAINSTREAM];
    param_main.recordtype = -1;
    param_main.codec = param.codec;
    param_main.width = param.width;
    param_main.height = param.height;
    param_main.framerate = param.framerate;
    param_main.bitrate = param.bitrate;
    param_main.iframeinterval = param.iframeinterval;
    param_main.ratecontrol = param.ratecontrol;
    param_main.smartstream = ui->comboBoxSmartStreamMain->currentIntData();
    param_main.smartstreamlevel = static_cast<int>(ui->horizontalSliderMain->value());
    //
    req_set_ipc_subparam &param_sub = set_ipc_paramex.stream[STREAM_TYPE_SUBSTREAM];
    param_sub.recordtype = -1;
    param_sub.codec = param.sub_codec;
    param_sub.width = param.sub_width;
    param_sub.height = param.sub_height;
    param_sub.framerate = param.sub_framerate;
    param_sub.bitrate = param.sub_bitrate;
    param_sub.iframeinterval = param.subiframeinterval;
    param_sub.ratecontrol = param.sub_ratecontrol;
    param_sub.smartstream = ui->comboBoxSmartStreamSub->currentIntData();
    param_sub.smartstreamlevel = static_cast<int>(ui->horizontalSliderSub->value());
    //
    STRUCT(REQ_IPC_SUBPARAMS, &set_ipc_paramex.stream[STREAM_TYPE_MAINSTREAM],
           FIELD(int, profile); //no need set
           FIELD(int, codec);
           FIELD(int, width);
           FIELD(int, height);
           FIELD(int, framerate);
           FIELD(int, bitrate);
           FIELD(int, iframeinterval);
           FIELD(int, ratecontrol);

           FIELD(int, smartstream);
           FIELD(int, smartstreamlevel);

           //for event record
           FIELD(int, recordtype); //-1. nothing  | 0: general 1:event
           FIELD(int, etframerate);
           FIELD(int, etIframeinterval);
           FIELD(int, etbitrate););
    sendMessage(REQUEST_FLAG_SET_IPCPARAM, (void *)&set_ipc_paramex, sizeof(req_set_ipc_paramex));

    //MsWaitting::showGlobalWait();
}

void CameraManagement::onAuthChanged()
{
    on_pushButton_refresh_clicked();
    return;
}

void CameraManagement::onEditChanged()
{
    return;
}

void CameraManagement::on_comboBox_maxFrameRate_main_activated(int index)
{
    ui->comboBox_iFrameInterval_main->setCurrentIndexFromData((index + 1) * 2);
}

void CameraManagement::on_comboBox_maxFrameRate_sub_activated(int index)
{
    ui->comboBox_iFrameInterval_sub->setCurrentIndexFromData((index + 1) * 2);
}

void CameraManagement::on_lineEdit_ip_textEdited(const QString &arg1)
{
    if (ui->comboBox_protocol->currentIntData() == IPC_PROTOCOL_MSDOMAIN) {
        QRegExp rx("(^ddns\\.milesight\\.com\\/)(.{0,12})");
        if (rx.indexIn(arg1) == -1) {
            ui->lineEdit_ip->setText(m_msddns);
            return;
        }
        QString strIP = rx.cap(2).toUpper();
        m_msddns = rx.cap(1) + strIP;
        ui->lineEdit_ip->setText(m_msddns);
    }
}

void CameraManagement::on_lineEdit_primary_textEdited(const QString &arg1)
{
    if (ui->comboBox_protocol->currentIntData() == IPC_PROTOCOL_RTSP) {
        QRegExp rx("(^rtsp:\\/\\/)");
        if (rx.indexIn(arg1) == -1) {
            ui->lineEdit_primary->setText(m_primary);
            return;
        }
        m_primary = arg1;
    }
}

void CameraManagement::on_lineEdit_secondary_textEdited(const QString &arg1)
{
    if (ui->comboBox_protocol->currentIntData() == IPC_PROTOCOL_RTSP) {
        QRegExp rx("(^rtsp:\\/\\/)");
        if (rx.indexIn(arg1) == -1) {
            ui->lineEdit_secondary->setText(m_secondary);
            return;
        }
        m_secondary = arg1;
    }
}

void CameraManagement::on_comboBoxSmartStreamMain_activated(int index)
{
    if (index == 1) {
        ui->comboBox_bitrateControl_main->setEnabled(false);
        ui->labelLevelMain->show();
        ui->horizontalSliderMain->show();
    } else {
        if (ui->comboBox_protocol_2->count() > 0 && ui->comboBox_bitrateControl_main->count() == 0) {
            ui->comboBox_bitrateControl_main->setEnabled(false);
        } else {
            ui->comboBox_bitrateControl_main->setEnabled(true);
        }
        ui->labelLevelMain->hide();
        ui->horizontalSliderMain->hide();
    }
}

void CameraManagement::on_comboBoxSmartStreamSub_activated(int index)
{
    if (index == 1) {
        ui->comboBox_bitrateControl_sub->setEnabled(false);
        ui->labelLevelSub->show();
        ui->horizontalSliderSub->show();
    } else {
        if (ui->comboBox_protocol_2->count() > 0 && ui->comboBox_bitrateControl_sub->count() == 0) {
            ui->comboBox_bitrateControl_sub->setEnabled(false);
        } else {
            ui->comboBox_bitrateControl_sub->setEnabled(true && ui->comboBox_secondaryStream->currentIntData());
        }
        ui->labelLevelSub->hide();
        ui->horizontalSliderSub->hide();
    }
}

void CameraManagement::on_comboBox_frameSize_main_indexSet(int index)
{
    QString mainFrameSize = ui->comboBox_frameSize_main->itemText(index);
    int mainMaxFrameRate = 0;
    //m_jsonData为空为第三方机型，直接从码流和机型判断Max Frame Rate选项。非空解析json数据
    if (!m_jsonData.isEmpty()) {
        const CameraStreamJsonData::StreamInfo &stream = m_cameraStreamJsonData->steamMapValue(m_mainStream);
        int codec = ui->comboBox_videoCodec_main->currentData().toInt();
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
        ui->comboBox_maxFrameRate_main->clear();
        for (int i = 1; i <= mainMaxFrameRate; ++i) {
            ui->comboBox_maxFrameRate_main->addItem(QString("%1").arg(i), i);
        }
        if (ui->comboBox_maxFrameRate_main->findData(m_ipc_param.main_range.framerate) < 0) {
            ui->comboBox_maxFrameRate_main->setCurrentIndexFromData(mainMaxFrameRate);
        } else {
            ui->comboBox_maxFrameRate_main->setCurrentIndexFromData(m_ipc_param.main_range.framerate);
        }
        //sub
        int mainSubFrameRate = 0;
        if (m_cameraStreamJsonData->mainEffectsMapIsContains(mainFrameSize)) {
            mainSubFrameRate = m_cameraStreamJsonData->mainEffectsMapValue(mainFrameSize);
            //
            ui->comboBox_maxFrameRate_sub->clear();
            for (int i = 1; i <= mainSubFrameRate; ++i) {
                ui->comboBox_maxFrameRate_sub->addItem(QString("%1").arg(i), i);
            }
            if (ui->comboBox_maxFrameRate_sub->findData(m_ipc_param.sub_range.framerate) < 0) {
                ui->comboBox_maxFrameRate_sub->setCurrentIndexFromData(mainSubFrameRate);
            } else {
                ui->comboBox_maxFrameRate_sub->setCurrentIndexFromData(m_ipc_param.sub_range.framerate);
            }
        }
    } else {
        QStringList list = mainFrameSize.split("*");
        if (list.size() == 2) {
            MsCameraModel cameraModel(qMsNvr->cameraRealModel(m_ipc_param.chnid));
            mainMaxFrameRate = cameraModel.maxFrameRate(mainFrameSize);
            if (mainMaxFrameRate == 0) {
                mainMaxFrameRate = m_ipc_param.main_range.framerate_max;
            }
            updateFrameRateMain(mainMaxFrameRate, m_ipc_param.main_range.framerate, cameraModel);

            //
            int width = list.at(0).toInt();
            int height = list.at(1).toInt();
            int size = qRound(width * height / 1024.0 / 1024.0);
            //45平台5MP@20fps
            if (cameraModel.model().contains(QRegExp("MS-C53..-.*S.*PC"))) {
                int maxFrameRate = 0;
                if (cameraModel.powerFrequency() == MS_60Hz) {
                    maxFrameRate = 30;
                } else {
                    maxFrameRate = 25;
                }

                ui->comboBox_maxFrameRate_sub->clear();
                for (int i = 1; i <= maxFrameRate; ++i) {
                    ui->comboBox_maxFrameRate_sub->addItem(QString("%1").arg(i), i);
                }
                ui->comboBox_maxFrameRate_sub->setCurrentIndexFromData(m_ipc_param.sub_range.framerate);
            }
            //45平台主码流5百万4百万时次码流30帧，其他60帧
            else if (cameraModel.model().contains(QRegExp("MS-C53..-.*PC"))) {
                int maxFrameRate = 0;
                if (cameraModel.powerFrequency() == MS_60Hz) {
                    if (size >= 4) {
                        maxFrameRate = 30;
                    } else {
                        maxFrameRate = 60;
                    }
                } else if (cameraModel.powerFrequency() == MS_50Hz) {
                    if (size >= 4) {
                        maxFrameRate = 25;
                    } else {
                        maxFrameRate = 50;
                    }
                } else {
                    qMsWarning() << "powerFrequency:" << cameraModel.powerFrequency();
                }

                ui->comboBox_maxFrameRate_sub->clear();
                for (int i = 1; i <= maxFrameRate; ++i) {
                    ui->comboBox_maxFrameRate_sub->addItem(QString("%1").arg(i), i);
                }
                ui->comboBox_maxFrameRate_sub->setCurrentIndexFromData(m_ipc_param.sub_range.framerate);
            }
        }
    }
}

void CameraManagement::on_comboBox_frameSize_main_activated(int index)
{
    Q_UNUSED(index)

    ui->comboBox_maxFrameRate_main->setCurrentIndex(ui->comboBox_maxFrameRate_main->count() - 1);
}

void CameraManagement::on_comboBox_frameSize_sub_indexSet(int index)
{
    QString mainFrameSize = ui->comboBox_frameSize_main->currentText();
    if (m_cameraStreamJsonData->mainEffectsMapIsContains(mainFrameSize)) {

    } else {
        QString subFrameSize = ui->comboBox_frameSize_sub->itemText(index);
        const CameraStreamJsonData::StreamInfo &stream = m_cameraStreamJsonData->steamMapValue(m_SubStream);
        int codec = ui->comboBox_videoCodec_sub->currentData().toInt();
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
        ui->comboBox_maxFrameRate_sub->clear();
        for (int i = 1; i <= subMaxFrameRate; ++i) {
            ui->comboBox_maxFrameRate_sub->addItem(QString("%1").arg(i), i);
        }
        if (ui->comboBox_maxFrameRate_sub->findData(m_ipc_param.sub_range.framerate) < 0) {
            ui->comboBox_maxFrameRate_sub->setCurrentIndexFromData(subMaxFrameRate);
        } else {
            ui->comboBox_maxFrameRate_sub->setCurrentIndexFromData(m_ipc_param.sub_range.framerate);
        }
    }
}

void CameraManagement::on_comboBox_frameSize_sub_activated(int index)
{
    Q_UNUSED(index)

    ui->comboBox_maxFrameRate_sub->setCurrentIndex(ui->comboBox_maxFrameRate_sub->count() - 1);
}

void CameraManagement::on_comboBox_videoCodec_main_activated(int index)
{
    if (!m_jsonData.isEmpty()) {
        int codec = ui->comboBox_videoCodec_main->itemData(index).toInt();
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

    } else {
        ui->comboBox_frameSize_main->clear();
        for (int i = 0; i < 24; ++i) {
            const ipc_resolution &resolution = m_ipc_param.main_range.res[i];
            if (resolution.width > 0 && resolution.height > 0) {
                ui->comboBox_frameSize_main->addItem(QString("%1*%2").arg(resolution.width).arg(resolution.height));
            }
        }
    }
    int currentMainFramesizeIndex = ui->comboBox_frameSize_main->findText(QString("%1*%2").arg(m_ipc_param.main_range.cur_res.width).arg(m_ipc_param.main_range.cur_res.height));
    if (currentMainFramesizeIndex < 0) {
        currentMainFramesizeIndex = 0;
    }
    ui->comboBox_frameSize_main->setCurrentIndex(currentMainFramesizeIndex);

    ui->comboBox_frameSize_main->endEdit();
    ui->comboBox_frameSize_main->reSetIndex();
}

void CameraManagement::on_comboBox_videoCodec_sub_activated(int index)
{
    int codec = ui->comboBox_videoCodec_sub->itemData(index).toInt();
    if (!m_jsonData.isEmpty()) {
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
    } else {
        //43平台机型屏蔽264编码下 320*240 320*192 320*180分辨率
        MsCameraVersion cameraVersion = MsCameraVersion::fromChannel(m_ipc_param.chnid);
        if (codec == CODECTYPE_H264 && cameraVersion.chipType() == 4 && cameraVersion.chipVersion() == 3) {
            ui->comboBox_frameSize_sub->clear();
            for (int i = 0; i < 24; ++i) {
                const ipc_resolution &resolution = m_ipc_param.sub_range.res[i];
                if (resolution.width > 0 && resolution.height > 0) {
                    QString text = QString("%1*%2").arg(resolution.width).arg(resolution.height);
                    if (text == "320*240" || text == "320*192" || text == "320*180") {
                        continue;
                    }
                    ui->comboBox_frameSize_sub->addItem(text);
                }
            }
        }
        //43平台除鱼眼机型外265编码下,需要加上320*240 320*192 320*180分辨率
        else if (codec == CODECTYPE_H265 && cameraVersion.chipType() == 4 && cameraVersion.chipVersion() == 3) {
            ui->comboBox_frameSize_sub->clear();
            for (int i = 0; i < 24; ++i) {
                const ipc_resolution &resolution = m_ipc_param.sub_range.res[i];
                if (resolution.width > 0 && resolution.height > 0) {
                    QString text = QString("%1*%2").arg(resolution.width).arg(resolution.height);
                    ui->comboBox_frameSize_sub->addItem(text);
                }
            }
            if (!qMsNvr->isFisheye(m_ipc_param.chnid)) {
                if (ui->comboBox_frameSize_sub->findText("320*240") == -1) {
                    ui->comboBox_frameSize_sub->addItem("320*240");
                }
                if (ui->comboBox_frameSize_sub->findText("320*192") == -1) {
                    ui->comboBox_frameSize_sub->addItem("320*192");
                }
                if (ui->comboBox_frameSize_sub->findText("320*180") == -1) {
                    ui->comboBox_frameSize_sub->addItem("320*180");
                }
            }
        } else {
            ui->comboBox_frameSize_sub->clear();
            for (int i = 0; i < 24; ++i) {
                const ipc_resolution &resolution = m_ipc_param.sub_range.res[i];
                if (resolution.width > 0 && resolution.height > 0) {
                    ui->comboBox_frameSize_sub->addItem(QString("%1*%2").arg(resolution.width).arg(resolution.height));
                }
            }
        }
    }
    int currentSubFramesizeIndex = ui->comboBox_frameSize_sub->findText(QString("%1*%2").arg(m_ipc_param.sub_range.cur_res.width).arg(m_ipc_param.sub_range.cur_res.height));
    if (currentSubFramesizeIndex < 0) {
        currentSubFramesizeIndex = 0;
    }
    ui->comboBox_frameSize_sub->setCurrentIndex(currentSubFramesizeIndex);
    ui->comboBox_frameSize_sub->endEdit();
    ui->comboBox_frameSize_sub->reSetIndex();
}

void CameraManagement::on_comboBox_transportProtocol_activated(int index)
{
    Q_UNUSED(index)
    if (ui->comboBox_transportProtocol->currentIntData() == TRANSPROTOCOL_ROH) {
        ui->lineEditHTTPSPort->show();
        ui->labelHTTPSPort->show();
        ui->label_port->setText(GET_TEXT("SYSTEMNETWORK/71057", "HTTP Port"));
    } else {
        ui->lineEditHTTPSPort->hide();
        ui->labelHTTPSPort->hide();
        ui->label_port->setText(GET_TEXT("CHANNELMANAGE/30011", "Port"));
    }       
}